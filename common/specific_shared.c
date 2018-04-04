
/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/
#include "ipcfifoTGDS.h"
#include "specific_shared.h"
#include "apu_shared.h"

#ifdef ARM7
#include <string.h>

#include "pocketspc.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "mixrate.h"
#include "wifi_arm7.h"
#include "spifwTGDS.h"
#include "apu_shared.h"

#endif

#ifdef ARM9

#include <stdbool.h>

#include "memmap.h"
#include "common.h"
#include "cfg.h"
#include "main.h"
#include "core.h"

#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"

#include "wifi_arm9.h"

#endif


//inherits what is defined in: common_shared.c
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2,uint32 cmd3,uint32 cmd4){
	
	switch (cmd1) {		
		
		//ARM7 command handler
		#ifdef ARM7		
		case SNEMULDS_APUCMD_RESET: //case 0x00000001:
		{
			// Reset
			StopSound();
			memset(playBuffer, 0, MIXBUFSIZE * 8);
			*APU_ADDR_CNT = 0; 
			ApuReset();
			DspReset();
			SetupSound();
			paused = false;
			SPC_disable = false;
			SPC_freedom = false;
		}
		break;
		case SNEMULDS_APUCMD_PAUSE:{ //case 0x00000002:{
			// Pause/unpause
			if (!paused) {
				StopSound();
			} else {
				SetupSound();
			}
			if (SPC_disable)
				SPC_disable = false;        
			paused = !paused;
		}
		break;
		case SNEMULDS_APUCMD_PLAYSPC:{ //case 0x00000003:{ // PLAY SPC 	
			LoadSpc(APU_RAM_ADDRESS);
			SetupSound();   	
			*APU_ADDR_CNT = 0;             	
			paused = false;
			SPC_freedom = true;
			SPC_disable = false;
		}
		break;			
		case SNEMULDS_APUCMD_SPCDISABLE:{ //case 0x00000004:{ // DISABLE 
			SPC_disable = true;
			*APU_ADDR_CNT = 0;
		}
		break;        
		
		case SNEMULDS_APUCMD_CLRMIXERBUF:{ //case 0x00000005:{ // CLEAR MIXER BUFFER 
			memset(playBuffer, 0, MIXBUFSIZE * 8);
		}
		break;
		case SNEMULDS_APUCMD_SAVESPC:{ //case 0x00000006:{ // SAVE state 
			SaveSpc(APU_RAM_ADDRESS);
		}
		break;
		case SNEMULDS_APUCMD_LOADSPC:{ //case 0x00000007:{ // LOAD state 
			LoadSpc(APU_RAM_ADDRESS);
			*APU_ADDR_CNT = 0; 
		}
		break;
		#endif
		
	}
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2,uint32 cmd3,uint32 cmd4){
}

//project specific stuff
#ifdef ARM9

//small hack to update SNES_ADDRESS at opcodes2.s
void update_ram_snes(){
    snes_ram_address = (uint32)&snes_ram_bsram[0x6000];
}
#endif

//APU Ports from SnemulDS properly binded with Assembly APU Core
void update_spc_ports(){
    
	APU_T0_ASM_ADDR = (uint32)&APU2->T0;
	APU_T1_ASM_ADDR = (uint32)&APU2->T1;
	APU_T2_ASM_ADDR = (uint32)&APU2->T2;
	
	APU_TIM0_ASM_ADDR = (uint32)&APU2->TIM0;
	APU_TIM1_ASM_ADDR = (uint32)&APU2->TIM1;
	APU_TIM2_ASM_ADDR = (uint32)&APU2->TIM2;
	
	APU_CNT0_ASM_ADDR = (uint32)&APU2->CNT0;
	APU_CNT1_ASM_ADDR = (uint32)&APU2->CNT1;
	APU_CNT2_ASM_ADDR = (uint32)&APU2->CNT2;
	
	//must reflect to specific_shared.h defs
	ADDRPORT_SPC_TO_SNES	=	(uint32)(uint8*)PORT_SPC_TO_SNES;	
	ADDRPORT_SNES_TO_SPC	=	(uint32)(uint8*)PORT_SNES_TO_SPC; 
	ADDR_APU_PROGRAM_COUNTER=	(uint32)(volatile uint32*)APU_PROGRAM_COUNTER;	//0x27E0000	@APU PC
	
	ADDR_SNEMUL_CMD	=	(uint32)(volatile uint32*)APU_ADDR_CMD;	//0x027FFFE8	// SNEMUL_CMD
	ADDR_SNEMUL_ANS	=	(uint32)(volatile uint32*)APU_ADDR_ANS;	//0x027fffec	// SNEMUL_ANS
	ADDR_SNEMUL_BLK	=	(uint32)(volatile uint32*)APU_ADDR_BLK;	//0x027fffe8	// SNEMUL_BLK
	
	//todo: APU_ADDR_CNT: is unused by Assembly APU Core?
}
