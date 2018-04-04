#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "pocketspc.h"
#include "apu.h"
#include "apumisc.h"
#include "specific_shared.h"
#include "apu_shared.h"

////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////

//struct s_apu2 *APU2 = (struct s_apu2 *)(&SpecificIPC->APU2);

uint8 iplRom[64] ALIGNED =
{
	0xCD,0xEF,0xBD,0xE8,0x00,0xC6,0x1D,0xD0,0xFC,0x8F,0xAA,0xF4,0x8F,0xBB,0xF5,0x78,
	0xCC,0xF4,0xD0,0xFB,0x2F,0x19,0xEB,0xF4,0xD0,0xFC,0x7E,0xF4,0xD0,0x0B,0xE4,0xF5,
	0xCB,0xF4,0xD7,0x00,0xFC,0xD0,0xF3,0xAB,0x01,0x10,0xEF,0x7E,0xF4,0x10,0xEB,0xBA,
	0xF6,0xDA,0x00,0xBA,0xF4,0xC4,0xF4,0xDD,0x5D,0xD0,0xDB,0x1F,0x00,0x00,0xC0,0xFF
};

// Asm uses these defines, so don't change them around
uint8 *APU_MEM;
uint8 *APU_MEM_ZEROPAGEREAD;
uint8 *APU_MEM_ZEROPAGEWRITE;
uint8 APU_EXTRA_MEM[64] ALIGNED;
uint8 apuSleeping ALIGNED;

uint32 APU_STATE[16];

uint8 MakeRawPSWFromState(uint32 state[16]) {
	uint8 psw = 0;

    psw |= APU_STATE[8] & 0x80; // N flag
	psw |= ((APU_STATE[6] >> 1) & 1) << 6; // V
    psw |= ((APU_STATE[6] >> 2) & 1) << 3; // H
    psw |= (APU_STATE[8] == 0 ? 1 : 0) << 1; // Z
	psw |= APU_STATE[6] & 1; // C

    // DP
	psw |= ((APU_STATE[4] >> 8) & 1) << 5;

	return psw;
}

void SetStateFromRawPSW(uint32 state[16], uint8 psw) {
	APU_STATE[8] = 0;
    APU_STATE[6] &= ~0x7;

	if ((psw >> 7) & 1) APU_STATE[8] |= 0x80;
	if ((psw >> 6) & 1) APU_STATE[6] |= 1 << 1;
    if ((psw >> 3) & 1) APU_STATE[6] |= 1 << 2;
	if (!((psw >> 1) & 1)) APU_STATE[8] |= 1;
	if (psw & 1) APU_STATE[6] |= 1;

	APU_STATE[4] = ((psw >> 5) & 1) << 8;
}

void  ApuReset() {
    apuSleeping = 0;

    APU_MEM = (uint8*)APU_RAM_ADDRESS;
	
	APU_MEM_ZEROPAGEREAD = (uint8*)&MemZeroPageReadTable;
    APU_MEM_ZEROPAGEWRITE = (uint8*)&MemZeroPageWriteTable;
	
	int i=0;
    for (i = 0; i < 65472; i += 0x40) { 
        memset(APU_MEM+i, 0, 0x20);
        memset(APU_MEM+i+0x20, 0xFF, 0x20);
    }
	
	memset(APU_MEM + 0xF0, 0, 0x10);
	
    ApuSetShowRom();
	
	for (i=0; i<=0x3F; i++) {
        APU_MEM[0xFFC0 + i] = iplRom[i];
        APU_EXTRA_MEM[i] = iplRom[i];
    }
	
	for (i=0; i<=0x3F; i++) {
        APU_MEM[0xFFC0 + i] = iplRom[i];
        APU_EXTRA_MEM[i] = iplRom[i];
    }
	
	for (i = 0; i < 0x100; i++) {
        ((uint32*)APU_MEM_ZEROPAGEREAD)[i] = (uint32)(&MemReadDoNothing);	//byte
        ((uint32*)APU_MEM_ZEROPAGEWRITE)[i + 0x40] = (uint32)(&MemWriteDoNothing); //byte
    }
	
    // Set up special read/write zones
    ((uint32*)APU_MEM_ZEROPAGEREAD)[0xf3] = (uint32)(&MemReadDspData);
    ((uint32*)APU_MEM_ZEROPAGEREAD)[0xfd] = (uint32)(&MemReadCounter);
    ((uint32*)APU_MEM_ZEROPAGEREAD)[0xfe] = (uint32)(&MemReadCounter);
    ((uint32*)APU_MEM_ZEROPAGEREAD)[0xff] = (uint32)(&MemReadCounter);    

    ((uint32*)APU_MEM_ZEROPAGEWRITE)[0xf1 + 0x40] = (uint32)(&MemWriteApuControl);
    ((uint32*)APU_MEM_ZEROPAGEWRITE)[0xf3 + 0x40] = (uint32)(&MemWriteDspData);
    ((uint32*)APU_MEM_ZEROPAGEWRITE)[0xfa + 0x40] = (uint32)(&MemWriteCounter);
    ((uint32*)APU_MEM_ZEROPAGEWRITE)[0xfb + 0x40] = (uint32)(&MemWriteCounter);
    ((uint32*)APU_MEM_ZEROPAGEWRITE)[0xfc + 0x40] = (uint32)(&MemWriteCounter);
	
    for (i = 0; i < 4; i++) {
        ((uint32*)APU_MEM_ZEROPAGEREAD)[0xF4 + i] = (uint32)(&MemReadApuPort);
        ((uint32*)APU_MEM_ZEROPAGEWRITE)[0xF4 + i + 0x40]= (uint32)(&MemWriteApuPort);
        PORT_SNES_TO_SPC[i] = 0;
        PORT_SPC_TO_SNES[i] = 0;
    }
	
    for (i = 0; i < 0x40; i++) {
        ((uint32*)APU_MEM_ZEROPAGEWRITE)[i] = (uint32)(&MemWriteUpperByte);
    }
	
// 0 - A, 1 - X, 2 - Y, 3 - RAMBASE, 4 - DP, 5 - PC (Adjusted into rambase)
// 6 - Cycles (bit 0 - C, bit 1 - v, bit 2 - h, bits 3+ cycles left)
// 7 - Optable
// 8 - NZ

	// Set up the initial APU state
	APU_STATE[0] = APU_STATE[1] = APU_STATE[2] = 0;
    APU_STATE[3] = ((uint32)&(APU_MEM[0]));
	APU_STATE[4] = 0; // DP
	APU_STATE[5] = 0xFFC0 + APU_STATE[3];
	APU_STATE[6] = 0;
	APU_STATE[7] = (uint32)CpuJumpTable;
    APU_STATE[8] = 0;
    APU_SP = 0x1FF;

	APU2->TIM0 = 0;
	APU2->TIM1 = 0;
	APU2->TIM2 = 0;
}
