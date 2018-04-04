#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>//BRK(); SBRK();
#include <fcntl.h>
#include <sys/stat.h>

#include "pocketspc.h"
#include "apumisc.h"
#include "dsp.h"
#include "apu.h"

// Envelope timing table.  Number of counts that should be subtracted from the counter
// The counter starts at 30720 (0x7800).
const sint16 ENVCNT_START = 0x7800;
const sint16 ENVCNT[0x20] = {
	0x0000,0x000F,0x0014,0x0018,0x001E,0x0028,0x0030,0x003C,
	0x0050,0x0060,0x0078,0x00A0,0x00C0,0x00F0,0x0140,0x0180,
	0x01E0,0x0280,0x0300,0x03C0,0x0500,0x0600,0x0780,0x0A00,
	0x0C00,0x0F00,0x1400,0x1800,0x1E00,0x2800,0x3C00,0x7800
};

DspChannel channels[8];
uint8 DSP_MEM[0x100];
sint32 mixBuffer[DSPMIXBUFSIZE * 2];
//sint32 echoBuffer[DSPMIXBUFSIZE * 2];
sint16 brrTab[16 * 16];
//uint32 firTable[8 * 2 * 2];
uint8 *echoBase;
uint16 dspPreamp = 0x140;
uint16 echoDelay;
uint16 echoCursor;
//uint8 firOffset ALIGNED;

uint32 DecodeSampleBlock(DspChannel *channel) {
    uint8 *cur = (uint8*)&(APU_MEM[channel->blockPos]);
    sint16 *sample = channel->decoded;

    if (channel->blockPos > 0x10000 - 9) {
        // Set end of block, with no loop
        DspSetEndOfSample(channelNum);
        return 1;
    }

    // Is this the last block?
    if (channel->brrHeader & 1) {
        // channelNum will be set from asm
        DSP_MEM[DSP_ENDX] |= (1 << channelNum);

        if (channel->brrHeader & 2) {
            // Looping
    	    uint8 *sampleBase = APU_MEM + (DSP_MEM[DSP_DIR] << 8);
	        uint16 *lookupPointer = (uint16*)(sampleBase + (DSP_MEM[(channelNum << 4) + DSP_SRC] << 2));
            channel->blockPos = lookupPointer[1];
            cur = (uint8*)&(APU_MEM[channel->blockPos]);
        } else {
            DspSetEndOfSample(channelNum);
            return 1;
        }
    }

    channel->brrHeader = *cur;

    DecodeSampleBlockAsm(cur, sample, channel);

    channel->blockPos += 9;

    return 0;
}

void DspReset() {
    // Delay for 1 sample
    echoDelay = 4;
    echoCursor = 0;
    echoBase = APU_MEM;
	int i=0,c=0;

/*    firOffset = 0;
    for (i = 0; i < 8*2*2; i++) {
        firTable[i] = 0;
    }*/
 
    memset(DSP_MEM, 0, 0x100);
	
    // Disable echo emulation
    DSP_MEM[DSP_FLAG] = 0x20;
	
	for (i = 0; i < 2; i++) {
		memset(&channels[i], 0, sizeof(DspChannel));
        
		//removed by author
		/*channels[i].samplePos = 0;
        channels[i].envCount = 0;
        channels[i].active = false;
        channels[i].echoEnabled = false;*/
	}

    // Build a lookup table for the range values (thanks to trac)
    for (i = 0; i < 13; i++) {
		for (c = 0; c < 16; c++)
			brrTab[(i << 4) + c] = (sint16)((((c ^ 8) - 8) << i) >> 1);
	}
	// range 13-15
    for (i = 13; i < 16; i++) {
		for (c = 0; c < 8; c++)
			brrTab[(i << 4) + c] = 0;
		for(c = 8; c < 16; c++)
			brrTab[(i << 4) + c] = 0xF800;
	}
}

void DspSetChannelVolume(uint32 channel) {
    channels[channel].leftVolume = DSP_MEM[(channel << 4) + DSP_VOL_L];
    channels[channel].rightVolume = DSP_MEM[(channel << 4) + DSP_VOL_R];

    channels[channel].leftCalcVolume = (channels[channel].leftVolume * channels[channel].envx) >> 7;
    channels[channel].rightCalcVolume = (channels[channel].rightVolume * channels[channel].envx) >> 7;
}

void DspSetChannelPitch(uint32 channel) {
	uint16 rawFreq = ((DSP_MEM[(channel << 4) + DSP_PITCH_H] << 8) + DSP_MEM[(channel << 4) + DSP_PITCH_L]) & 0x3fff;

    // Clear low bit of sample speed so we can do a little optimization in dsp mixing
//	channels[channel].sampleSpeed = (((rawFreq << 3) << 12) / MIXRATE) & (~1);
	channels[channel].sampleSpeed = (((rawFreq << 3) << 12) / MIXRATE);
}

void DspSetChannelSource(uint32 channel) {
	uint8 *sampleBase = APU_MEM + (DSP_MEM[DSP_DIR] << 8);
	uint16 *lookupPointer = (uint16*)(sampleBase + (DSP_MEM[(channel << 4) + DSP_SRC] << 2));

    channels[channel].blockPos = lookupPointer[0];
}

void DspSetEndOfSample(uint32 channel) {
    channels[channel].active = false;
    channels[channel].envState = ENVSTATE_NONE;

    channels[channel].envx = 0;
    DSP_MEM[(channel << 4) | DSP_ENVX] = 0;
    DSP_MEM[(channel << 4) | DSP_OUTX] = 0;
}

void DspSetChannelEnvelopeHeight(uint32 channel, uint8 height) {
    channels[channel].envx = height << 8;

    channels[channel].leftCalcVolume = (channels[channel].leftVolume * channels[channel].envx) >> 7;
    channels[channel].rightCalcVolume = (channels[channel].rightVolume * channels[channel].envx) >> 7;
}

void DspStartChannelEnvelope(uint32 channel) {
	uint8 adsr1 = DSP_MEM[(channel << 4) + DSP_ADSR1];

    // ADSR mode, set envelope up
    // Attack rate goes into envelopeSpeed initially
  	uint8 adsr2 = DSP_MEM[(channel << 4) + DSP_ADSR2];
    uint8 decay = (adsr1 >> 4) & 0x7;
    uint8 sustainLevel = adsr2 >> 5;
    uint8 sustainRate = adsr2 & 0x1f;

    channels[channel].decaySpeed = ENVCNT[(decay << 1) + 0x10];
    channels[channel].sustainLevel = 0x10 * (sustainLevel + 1);
    channels[channel].sustainSpeed = ENVCNT[sustainRate];

    // Don't set envelope parameters when we are releasing the note
    if (adsr1 & 0x80) {
        uint8 attack = adsr1 & 0xf;

        if (attack == 0xf) {
            // 0ms attack, go straight to full volume, and set decay
            DspSetChannelEnvelopeHeight(channel, 0x7f);
            channels[channel].envSpeed = channels[channel].decaySpeed;
            channels[channel].envState = ENVSTATE_DECAY;
        } else {
            DspSetChannelEnvelopeHeight(channel, 0);
            channels[channel].envSpeed = ENVCNT[(attack << 1) + 1];
            channels[channel].envState = ENVSTATE_ATTACK;
        }
    } else {
        // Gain mode
    	uint8 gain = DSP_MEM[(channel << 4) + DSP_GAIN];

        if ((gain & 0x80) == 0) {
            // Direct designation
            DspSetChannelEnvelopeHeight(channel, gain & 0x7f);
            channels[channel].envState = ENVSTATE_DIRECT;
        } else {
            DspSetChannelEnvelopeHeight(channel, 0);
            channels[channel].envSpeed = ENVCNT[gain & 0x1f];

            switch ((gain >> 5) & 0x3) {
            case 0:
                // Linear decrease
                channels[channel].envState = ENVSTATE_DECREASE;
                break;
            case 1:
                // Exponential decrease
                channels[channel].envState = ENVSTATE_DECEXP;
                break;
            case 2:
                // Linear increase
                channels[channel].envState = ENVSTATE_INCREASE;
                break;
            case 3:
                // Bent line increase
                channels[channel].envState = ENVSTATE_BENTLINE;
                break;
            }
        }
    }
}

void DspChangeChannelEnvelopeGain(uint32 channel) {
    // Don't set envelope parameters when we are releasing the note
    if (!channels[channel].active) return;
    if (channels[channel].envState == ENVSTATE_RELEASE) return;

    // If in ADSR mode, write to GAIN register has no effect
    if (DSP_MEM[(channel << 4) + DSP_ADSR1] & 0x80) return;

    // Otherwise treat it as GAIN change
	uint8 gain = DSP_MEM[(channel << 4) + DSP_GAIN];

    if ((gain & 0x80) == 0) {
        // Direct designation
        DspSetChannelEnvelopeHeight(channel, gain & 0x7f);
        channels[channel].envState = ENVSTATE_DIRECT;
        channels[channel].envSpeed = 0;
    } else {
        channels[channel].envSpeed = ENVCNT[gain & 0x1f];

        switch ((gain >> 5) & 0x3) {
        case 0:
            // Linear decrease
            channels[channel].envState = ENVSTATE_DECREASE;
            break;
        case 1:
            // Exponential decrease
            channels[channel].envState = ENVSTATE_DECEXP;
            break;
        case 2:
            // Linear increase
            channels[channel].envState = ENVSTATE_INCREASE;
            break;
        case 3:
            // Bent line increase
            channels[channel].envState = ENVSTATE_BENTLINE;
            break;
        }
    }
}

void DspChangeChannelEnvelopeAdsr1(uint32 channel, uint8 orig) {
    // Don't set envelope parameters when we are releasing the note
    if (!channels[channel].active) return;
    if (channels[channel].envState == ENVSTATE_RELEASE) return;

	uint8 adsr1 = DSP_MEM[(channel << 4) + DSP_ADSR1];

    uint8 decay = (adsr1 >> 4) & 0x7;
    channels[channel].decaySpeed = ENVCNT[(decay << 1) + 0x10];

    if (channels[channel].envState == ENVSTATE_ATTACK) {
        uint8 attack = adsr1 & 0xf;
        channels[channel].envSpeed = ENVCNT[(attack << 1) + 1];
    } else if (channels[channel].envState == ENVSTATE_DECAY) {
        channels[channel].envSpeed = channels[channel].decaySpeed;
    }

    if (adsr1 & 0x80) {
        if (!(orig & 0x80)) {
            // Switch to ADSR
            uint8 attack = adsr1 & 0xf;
            channels[channel].envState = ENVSTATE_ATTACK;
            channels[channel].envSpeed = ENVCNT[(attack << 1) + 1];
        }
    } else {
        // Switch to gain mode
        DspChangeChannelEnvelopeGain(channel);
    }
}

void DspChangeChannelEnvelopeAdsr2(uint32 channel) {
    // Don't set envelope parameters when we are releasing the note
    if (!channels[channel].active) return;
    if (channels[channel].envState == ENVSTATE_RELEASE) return;

	uint8 adsr2 = DSP_MEM[(channel << 4) + DSP_ADSR2];
    uint8 sustainRate = adsr2 & 0x1f;
    channels[channel].sustainSpeed = ENVCNT[sustainRate];

    if (channels[channel].envState == ENVSTATE_SUSTAIN) {
        channels[channel].envSpeed = channels[channel].sustainSpeed;
    }
}

void DspKeyOnChannel(uint32 i) {
    channels[i].envState = ENVSTATE_NONE;

    DspSetChannelEnvelopeHeight(i, 0);
    DSP_MEM[(i << 4) | DSP_ENVX] = 0;
    DSP_MEM[(i << 4) | DSP_OUTX] = 0;

    DspSetChannelVolume(i);
    DspSetChannelPitch(i);
    DspSetChannelSource(i);
    DspStartChannelEnvelope(i);

    channels[i].samplePos = 16 << 12;

    channels[i].brrHeader = 0;
    channels[i].prevSamp1 = 0;
    channels[i].prevSamp2 = 0;

    channels[i].envCount = ENVCNT_START;
    channels[i].active = true;

    if ((DSP_MEM[DSP_NOV]>>i)&1) {
        channels[i].active = false;
        // Noise sample
    }

    DSP_MEM[DSP_ENDX] &= ~(1 << i);
}

void DspPrepareStateAfterReload() {
    // Set up echo delay
    DspWriteByte(DSP_MEM[DSP_EDL], DSP_EDL);

    echoBase = APU_MEM + (DSP_MEM[DSP_ESA] << 8);
    
	memset(echoBase, 0, echoDelay);
	
	uint32 i=0;
	for (i = 0; i < 8; i++) {
		
        channels[i].echoEnabled = (DSP_MEM[DSP_EON] >> i) & 1;
		
        if (DSP_MEM[DSP_KON] & (1 << i)) {
            DspKeyOnChannel(i);
        }
	}
}

void DspWriteByte(uint8 val, uint8 address) {
    uint8 orig = DSP_MEM[address];
    DSP_MEM[address] = val;

    if (address > 0x7f) return;

    switch (address & 0xf) {
        case DSP_VOL_L:
			DspSetChannelVolume(address >> 4);
            break;
		case DSP_VOL_R:
			DspSetChannelVolume(address >> 4);
			break;
		case DSP_PITCH_L:
			DspSetChannelPitch(address >> 4);
			break;
		case DSP_PITCH_H:
			DspSetChannelPitch(address >> 4);
			break;
		case DSP_ADSR1:
    		DspChangeChannelEnvelopeAdsr1(address >> 4, orig);
			break;
		case DSP_ADSR2:
			DspChangeChannelEnvelopeAdsr2(address >> 4);
			break;
		case DSP_GAIN:
			DspChangeChannelEnvelopeGain(address >> 4);
			break;

        case 0xC:
            switch (address >> 4) {
				case (DSP_KON >> 4):
					// val &= ~DSP_MEM[DSP_KOF];
					//DSP_MEM[DSP_KON] = val & DSP_MEM[DSP_KOF];

					if (val) {
						DSP_MEM[DSP_KON] = val & DSP_MEM[DSP_KOF];
						val &= ~DSP_MEM[DSP_KOF];
						uint32 i=0;
						for (; i<8; i++)
							if ((val>>i)&1) {
								DspKeyOnChannel(i);
							}
					}
				break;

				case (DSP_KOF >> 4):{
					int i=0;
					for (; i<8; i++)
						if (((val>>i)&1) && channels[i].active && channels[i].envState != ENVSTATE_RELEASE) {
							// Set current state to release (ENDX will be set when release hits 0)
							channels[i].envState = ENVSTATE_RELEASE;
							channels[i].envCount = ENVCNT_START;
							channels[i].envSpeed = ENVCNT[0x1C];
						}
				}
    			break;

				case (DSP_ENDX >> 4):
					DSP_MEM[DSP_ENDX] = 0;
		    	break;
            }
            break;

        case 0xD:
            switch (address >> 4) {
				case (DSP_EDL >> 4):
					val &= 0xf;
					if (val == 0) {
						echoDelay = 4;
					} 
					else {
						echoDelay = ((uint32)(val << 4) * (MIXRATE / 1000)) << 2;
					}
                break;

				case (DSP_NOV >> 4):{
					int i=0;
					for (; i<8; i++)
						if ((val>>i)&1) {
							// TODO: Need to implement noise channels
							channels[i].active = false;
						}
				}
                break;

				case (DSP_ESA >> 4):
					echoBase = APU_MEM + (DSP_MEM[DSP_ESA] << 8);
                break;

				case (DSP_EON >> 4):{
					int i=0;
					for (; i < 8; i++) {
						channels[i].echoEnabled = (val >> i) & 1;
					}
				}
				break;
            }
		}

}