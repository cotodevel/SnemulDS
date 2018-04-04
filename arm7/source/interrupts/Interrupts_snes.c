#include "specific_shared.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "wifi_arm7.h"
#include "pocketspc.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "mixrate.h"
#include "apu_shared.h"

//User Handler Definitions
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer0handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer1handlerUser(){
	#if PROFILING_ON
		long long begin = TIMER2_DATA + ((long long)TIMER3_DATA << 19);
	#endif
    soundCursor = MIXBUFSIZE - soundCursor;

    // Left channel
    int channel = soundCursor == 0 ? 0 : 1;
    SCHANNEL_TIMER(channel) = SOUND_FREQ(MIXRATE);
    SCHANNEL_SOURCE(channel) = (uint32)&(playBuffer[MIXBUFSIZE - soundCursor]);
    SCHANNEL_LENGTH(channel) = (MIXBUFSIZE * 2) >> 2;
    SCHANNEL_REPEAT_POINT(channel) = 0;
    SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0) | SOUND_FORMAT_16BIT;

    // Right channel
    channel = soundCursor == 0 ? 2 : 3;
    SCHANNEL_TIMER(channel) = SOUND_FREQ(MIXRATE);
    SCHANNEL_SOURCE(channel) = (uint32)&(playBuffer[(MIXBUFSIZE - soundCursor) + (MIXBUFSIZE * 2)]);
    SCHANNEL_LENGTH(channel) = (MIXBUFSIZE * 2) >> 2;
    SCHANNEL_REPEAT_POINT(channel) = 0;
    SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0x7F) | SOUND_FORMAT_16BIT;

	#if PROFILING_ON
		long long end = TIMER2_DATA + ((long long)TIMER3_DATA << 19);
		SPC_IPC->cpuTime += end - begin;
	//    SPC_IPC->dspTime += (TIMER2_DATA + ((long long)TIMER3_DATA << 19)) - end;
	#endif
}


#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer2handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer3handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HblankUser(){
	// Block execution until the hblank processing on ARM9
	if (!SPC_disable)
	{
		int VCount = REG_VCOUNT;        
		scanlineCount++;
		uint32 T0 = APU_MEM[APU_TIMER0]?APU_MEM[APU_TIMER0]:0x100;
		uint32 T1 = APU_MEM[APU_TIMER1]?APU_MEM[APU_TIMER1]:0x100;
		uint32 T2 = APU_MEM[APU_TIMER2]?APU_MEM[APU_TIMER2]:0x100;
		
		if ((VCount & 1) == 1) {        		      	
			if (++APU2->TIM0 >= T0) {
				APU2->TIM0 -= T0;
				APU_MEM[APU_COUNTER0]++;
				APU_MEM[APU_COUNTER0] &= 0xf;
			}
			if (++APU2->TIM1 >= T1) {
				APU2->TIM1 -= T1;
				APU_MEM[APU_COUNTER1]++;
				APU_MEM[APU_COUNTER1] &= 0xf;
			}
		}
		APU2->TIM2 += 4;
		if (APU2->TIM2 >= T2) {
			APU2->TIM2 -= T2;
			APU_MEM[APU_COUNTER2]++;
			APU_MEM[APU_COUNTER2] &= 0xf;
		}
		
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VblankUser(){
	#if PROFILING_ON
		// Debug time data
		SPC_IPC->curTime += TIMER2_DATA | ((long long)TIMER3_DATA << 19);
		TIMER2_CR = 0;
		TIMER3_CR = 0;
		TIMER2_DATA = 0;
		TIMER2_CR = TIMER_DIV_64 | TIMER_ENABLE;
		TIMER3_DATA = 0;
		TIMER3_CR = TIMER_CASCADE | TIMER_ENABLE;
	#endif
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VcounterUser(){
}