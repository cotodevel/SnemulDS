//***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006-2007 archeide, All rights reserved. */
/***********************************************************/
/*
This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License as 
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
GNU General Public License for more details.
*/

#include <string.h>
#include "cfg.h"
#include "apu.h"
#include "specific_shared.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"
#include "InterruptsARMCores_h.h"
#include "core.h"
#include "apu_shared.h"

////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////

//struct s_apu2 *APU2 = (struct s_apu2 *)(&SpecificIPC->APU2);

void	APU_reset()
{
    APU_command(SNEMULDS_APUCMD_RESET); //APU_command(0x00000001);
}


void	APU_nice_reset()
{
#ifndef IN_EMULATOR	
	APU_stop();
	APU_reset();
#endif
	
}

void	APU_pause()
{
    APU_command(SNEMULDS_APUCMD_PAUSE); //APU_command(0x00000002);
}

void	APU_stop()
{
#ifndef IN_EMULATOR	
	APU_command(SNEMULDS_APUCMD_SPCDISABLE); //APU_command(0x00000004);
	// Wait the APU disabling
	while (*APU_ADDR_ANS != 0xFF00FF00){
		IRQVBlankWait();
	}
#endif	
}

void	APU_playSpc()
{
    APU_command(SNEMULDS_APUCMD_PLAYSPC); //APU_command(0x00000003);
}

void	APU_saveSpc()
{
	APU_command(SNEMULDS_APUCMD_SAVESPC); //APU_command(0x00000006);
	// Wait the ARM7 to save the SPC
	// FIXME : replace this with a variable check
}

void	APU_loadSpc()
{
	APU_command(SNEMULDS_APUCMD_LOADSPC); //APU_command(0x00000007);
	// Wait the ARM7 to load the SPC
	// FIXME : replace this with a variable check
}


void	APU_clear()
{
	APU_command(SNEMULDS_APUCMD_CLRMIXERBUF); //APU_command(0x00000005);
	*APU_ADDR_CNT = 0;
}


void APU_playSong(uint8 *data, int size)
{
	CFG.Sound_output = 0; // Disable Sound emulation
	if (size > 0x10000 + 0x100 + 0x100)
		return;
	
    APU_stop();    // Disable APU
    memcpy(APU_RAM_ADDRESS, data, size);
    APU_playSpc();    // Put APU in PLAY MODE	
}


void APU_command(uint32 command)
{
	//prevent APU from desync
	SendMultipleWordACK(command, 0, 0, NULL);
}