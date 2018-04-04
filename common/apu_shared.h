//This file: offers Linkage from IPC (dynamic) memory to APU IO Ports (like timers)


#ifndef __apu_shared_h__
#define __apu_shared_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "specific_shared.h"

#endif



#ifdef __cplusplus
extern "C" {
#endif

extern uint32 APU_T0_ASM_ADDR;
extern uint32 APU_T1_ASM_ADDR;
extern uint32 APU_T2_ASM_ADDR;

extern uint32 APU_TIM0_ASM_ADDR;
extern uint32 APU_TIM1_ASM_ADDR;
extern uint32 APU_TIM2_ASM_ADDR;

extern uint32 APU_CNT0_ASM_ADDR;
extern uint32 APU_CNT1_ASM_ADDR;
extern uint32 APU_CNT2_ASM_ADDR;

extern uint32 ADDRPORT_SPC_TO_SNES;
extern uint32 ADDRPORT_SNES_TO_SPC;

extern uint32 ADDR_APU_PROGRAM_COUNTER;
extern uint32 ADDR_SNEMUL_CMD;				//APU_ADDR_CMD	//0x027FFFE8
extern uint32 ADDR_SNEMUL_ANS;				//APU_ADDR_ANS	//0x027fffec
extern uint32 ADDR_SNEMUL_BLK;				//APU_ADDR_BLK	//0x027fffe8


#ifdef ARM9
extern uint32 snes_ram_address;
#endif

//extern struct s_apu2 *APU2;

#ifdef __cplusplus
}
#endif