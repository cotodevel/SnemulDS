/***********************************************************/
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

#include "socket.h"
#include "in.h"
#include <netdb.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "guiTGDS.h"
#include "fs.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "ram.h"
#include "conf.h"
#include "console_str.h"
#include "frontend.h"
#include "main.h"
#include "ppu.h"
#include "InterruptsARMCores_h.h"
#include "specific_shared.h"
#include "ff.h"
#include "memoryHandleTGDS.h"
#include "reent.h"
#include "sys/types.h"
#include "engine.h"
#include "core.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"
#include "consoleTGDS.h"
#include "api_wrapper.h"
#include "apu_jukebox.h"
#include "utilsTGDS.h"
#include "about.h"

//wnifilib: multiplayer
#include "dswnifi_lib.h"
#include "dswnifi.h"

#include "devoptab_devices.h"
#include "fsfatlayerTGDS.h"
#include "usrsettingsTGDS.h"

#include "videoTGDS.h"
#include "keypadTGDS.h"
#include "gui_console_connector.h"

int argc;
sint8 **argv;
int main(int _argc, sint8 **_argv) {
	
	IRQInit();
	
	*APU_ADDR_CNT = 0;
	*APU_ADDR_CMD = 0;
	
	screen_mode = 0;
	
	bool project_specific_console = true;	//set default console or custom console: custom console
	GUI_init(project_specific_console);
	
	sint32 fwlanguage = (sint32)getLanguage();
	GUI_setLanguage(fwlanguage);
	
	//These are internal printf just use printf / iprintf if unsure
	GUI.printfy = 32;
	GUI_align_printf(GUI_TEXT_ALIGN_CENTER, RetTitleCompiledVersion());
	GUI_align_printf(GUI_TEXT_ALIGN_CENTER, (sint8*)SNEMULDS_SUBTITLE[0]);
    GUI.printfy += 32; // FIXME
	GUI_align_printf(GUI_TEXT_ALIGN_CENTER, _STR(4));
	
	update_spc_ports(); //APU Ports from SnemulDS properly binded with Assembly APU Core
	initSNESEmpty();
    	
	int i=0;
	// Clear "HDMA"
	for (i = 0; i < 192; i++){
		GFX.lineInfo[i].mode = -1;
	}
	
	//single player:
	switch_dswnifi_mode(dswifi_idlemode);
	//udp nifi:
	//switch_dswnifi_mode(dswifi_udpnifimode);	//UDP NIFI: Check readme
	//local nifi: 
	//switch_dswnifi_mode(dswifi_localnifimode);	//LOCAL NIFI:
	
	int ret=FS_init();
	if (ret == 0)
	{
		printf(_STR(IDS_FS_SUCCESS));
		//FRESULT res = FS_chdir("0:/");
	}
	else if(ret == -1)
	{
		printf(_STR(IDS_FS_FAILED));
	}
	
	// Load SNEMUL.CFG: 0:/snemul.cfg
	set_config_file(getfatfsPath("snemul.cfg"));
	
	for (i = 0; i < 100; i++)	GUI_clear();
	
	sint8 *ROMfile;
	if(readFrontend(&ROMfile,(sint8**)&CFG.ROMPath[0])){
		readOptionsFromConfig("Global");
		GUI_getConfig();
	}else{
		
		sprintf(CFG.ROMPath,"%s",getfatfsPath((char*)READ_GAME_DIR[0]));	//put 3rd arg into 2nd of CFG."Name".
		readOptionsFromConfig("Global");	//reload Global settings from file into CFG Program
		GUI_getConfig();
		GUI_getROM(CFG.ROMPath);	//read rom from (path)touchscreen:output rom -> CFG.ROMFile
	}
	
	//printf("GUI_getROM():%s",ROMfile);	//rets filename	/rom.smc ALONE
	loadROM(CFG.ROMFile, 0);	//loadROM reads name only CFG.ROMFile
	GUI_deleteROMSelector(); // Should also free ROMFile
	GUI_createMainMenu();
    
	while (1)
	{
        if (REG_POWERCNT & POWER_SWAP_LCDS){
			GUI_update();
		}
        
        if (keys & KEY_LID)
		{
			saveSRAM();
			APU_pause();
			//			APU_stop();
			// hinge is closed 
			// power off everything not needed 
			powerOFF(POWERMAN_ARM9 | POWER_2D_A | POWER_2D_B | POWER_SWAP_LCDS) ;
			// set system into sleep 
			while (keys & KEY_LID)
			{
				keys = keysPressed();
			}
			// wait a bit until returning power 
			// power on again 
			powerON(POWERMAN_ARM9 | POWER_2D_A | POWER_2D_B | POWER_SWAP_LCDS);
			// set up old irqs again 
			APU_pause();
		}
		
        if (!SNES.Stopped){
            go();   //boots here
        }
	}

}