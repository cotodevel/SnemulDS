#ifndef snemuldsv6_apuarm7
#define snemuldsv6_apuarm7

#include "typedefsTGDS.h"
////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////

#define APU_MEM_IN_VRAM
#define APU_CONTROL_REG		0xF1

#define APU_TIMER0			0xFA
#define APU_TIMER1			0xFB
#define APU_TIMER2			0xFC

#define APU_COUNTER0		0xFD
#define APU_COUNTER1		0xFE
#define APU_COUNTER2		0xFF

// Cycles per second
//#define spcCyclesPerSec 2048000
#define spcCyclesPerSec 1024000
#define spcUpdatesPerSec 2048
#define spcCyclesPerUpdate (spcCyclesPerSec / spcUpdatesPerSec)

// 64Khz timer clock divisor
#define t64Shift 4
// 8Khz timer clock divisor
#define t8Shift 7

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void ApuExecute(uint32 cycles);
extern uint8 *APU_MEM;
extern uint8 APU_EXTRA_MEM[64];
extern uint32 APU_SP;
extern uint32 CpuJumpTable[];
extern uint32 APU_STATE[16];

// Memory post read/write functions
extern uint32 MemWriteDoNothing;
extern uint32 MemWriteApuControl;
extern uint32 MemWriteDspData;
extern uint32 MemWriteUpperByte;
extern uint32 MemWriteApuPort;
extern uint32 MemWriteCounter;
extern uint32 MemReadDoNothing;
extern uint32 MemReadCounter;
extern uint32 MemReadCounterFD;
extern uint32 MemReadCounterFE;
extern uint32 MemReadCounterFF;
extern uint32 MemReadApuPort;
extern uint32 MemReadDspData;
extern uint32 MemZeroPageReadTable;
extern uint32 MemZeroPageWriteTable;

// 0 - A, 1 - X, 2 - Y, 3 - RAMBASE, 4 - DP, 5 - PC (Adjusted into rambase)
// 6 - Cycles (bit 0 - C, bit 1 - v, bit 2 - h, bits 3+ cycles left)
// 7 - Optable
// 8 - NZ
extern uint8 MakeRawPSWFromState(uint32 state[16]);
extern void SetStateFromRawPSW(uint32 state[16], uint8 psw);
extern void ApuReset();
extern void ApuPrepareStateAfterReload();
extern void ApuUpdateTimers(uint32 cycles);
extern void ApuWriteControlByte(uint8 byte);
extern uint32 ApuReadCounter(uint32 address);
extern void ApuWriteUpperByte(uint8 byte, uint32 address);

#ifdef __cplusplus
}
#endif
