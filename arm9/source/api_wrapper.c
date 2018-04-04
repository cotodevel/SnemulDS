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
#include "api_wrapper.h"

#include <string.h>
#include <stdlib.h>
#include "cfg.h"
#include "gfx.h"
#include "core.h"
#include "engine.h"
#include "apu.h"
#include "ppu.h"
#include "main.h"
#include "conf.h"
#include "fs.h"
#include "memmap.h"
#include "crc32.h"
#include "guiTGDS.h"
#include "gui_console_connector.h"
#include "devoptab_devices.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "keypadTGDS.h"
#include "utilsTGDS.h"
#include "nds_cp15_misc.h"

//Preset BG Layering Config
__attribute__((section(".dtcm")))
uint8 LayersConf[10][4] =
{
{ 0, 1, 2, 3 },
{ 1, 2, 0, 3 },
{ 3, 3, 2, 3 },
{ 3, 3, 3, 3 },
{ 2, 2, 2, 2 },
{ 1, 1, 1, 1 },
{ 0, 0, 0, 0 },
{ 2, 3, 0, 1 },
{ 2, 0, 3, 1 },
{ 2, 1, 0, 3 }, 
};

void readOptionsFromConfig(sint8 *section)
{
	CFG.BG3Squish = get_config_int(section, "BG3Squish", CFG.BG3Squish) & 3;
	// FIXME 
	GFX.YScroll = get_config_int(section, "YScroll", GFX.YScroll);
	if (GFX.YScroll == 16)
		CFG.YScroll = 0;
	if (GFX.YScroll == 0)
		CFG.YScroll = 1;
	if (GFX.YScroll == 32)
		CFG.YScroll = 2;
	if (GFX.YScroll == 24)
		CFG.YScroll = 3;	
	
	CFG.Scaled = get_config_int(section, "Scaled", CFG.Scaled);
	CFG.Sound_output = get_config_int(section, "Sound", CFG.Sound_output) & 1;
	CFG.BG_Layer = (get_config_int(section, "HDMA", 1)&1) << 7;

	int BG_Layer = get_config_oct(section, "BGLayers", 010111);
	if ((BG_Layer & 7) == 1)
		CFG.BG_Layer |= 1;
	if (((BG_Layer>>3) & 7) == 1)
		CFG.BG_Layer |= 2;
	if (((BG_Layer>>6) & 7) == 1)
		CFG.BG_Layer |= 4;
	if (((BG_Layer>>9) & 7) == 1)
		CFG.BG_Layer |= 8;
	if (((BG_Layer>>12) & 7) == 1)
		CFG.BG_Layer |= 0x10;

	CFG.LayersConf = get_config_int(section, "BGPriorities", CFG.LayersConf);
	if (CFG.LayersConf == 10)
	{
		int BGManualPriority = get_config_oct(section, "BGManualPriority",
				00123);
		CFG.LayerPr[0] = (BGManualPriority) & 3;
		CFG.LayerPr[1] = (BGManualPriority>>3) & 3;
		CFG.LayerPr[2] = (BGManualPriority>>6) & 3;
		CFG.LayerPr[3] = (BGManualPriority>>9) & 3;

		CFG.LayerPr[0] = get_config_int(section, "BG1Pr", CFG.LayerPr[0]) & 3;
		CFG.LayerPr[1] = get_config_int(section, "BG2Pr", CFG.LayerPr[1]) & 3;
		CFG.LayerPr[2] = get_config_int(section, "BG3Pr", CFG.LayerPr[2]) & 3;
		CFG.LayerPr[3] = get_config_int(section, "BG4Pr", CFG.LayerPr[3]) & 3;

	}
	else
		PPU_ChangeLayerConf(CFG.LayersConf);

	CFG.Transparency
			= get_config_int(section, "Transparency", CFG.Transparency);
	CFG.WaitVBlank = get_config_int(section, "Vblank", CFG.WaitVBlank);
	CFG.CPU_speedhack
			= get_config_int(section, "SpeedHacks", CFG.CPU_speedhack);
	CFG.FastDMA = get_config_int(section, "FastDMA", CFG.FastDMA);

	CFG.MouseXAddr = get_config_hex(section, "MouseXAddr", 0);
	CFG.MouseYAddr = get_config_hex(section, "MouseYAddr", 0);
	CFG.MouseMode = get_config_int(section, "MouseMode", 0);
	CFG.MouseXOffset = get_config_int(section, "MouseXOffset", 0);
	CFG.MouseYOffset = get_config_int(section, "MouseYOffset", 0);

	CFG.SoundPortSync = 0;

	int SoundPortSync = get_config_oct(section, "SoundPortSync",
			CFG.SoundPortSync);
	if ((SoundPortSync & 7) == 1)
		CFG.SoundPortSync |= 8;
	if (((SoundPortSync>>3) & 7) == 1)
		CFG.SoundPortSync |= 4;
	if (((SoundPortSync>>6) & 7) == 1)
		CFG.SoundPortSync |= 2;
	if (((SoundPortSync>>9) & 7) == 1)
		CFG.SoundPortSync |= 1;
	if (((SoundPortSync>>12) & 7) == 1)
		CFG.SoundPortSync |= 0x80;
	if (((SoundPortSync>>15) & 7) == 1)
		CFG.SoundPortSync |= 0x40;
	if (((SoundPortSync>>18) & 7) == 1)
		CFG.SoundPortSync |= 0x20;
	if (((SoundPortSync>>21) & 7) == 1)
		CFG.SoundPortSync |= 0x10;

	CFG.TilePriorityBG = get_config_int(section, "TilePriorityBG",
			CFG.TilePriorityBG);
	CFG.BG3TilePriority = get_config_int(section, "BG3TilePriority",
			CFG.BG3TilePriority);
	CFG.Debug2 = get_config_int(section, "BlankTileNumber", CFG.Debug2);
	int SpritePriority = get_config_oct(section, "SpritePriority", 01123);
	CFG.SpritePr[0] = (SpritePriority) & 3;
	CFG.SpritePr[1] = (SpritePriority>>3) & 3;
	CFG.SpritePr[2] = (SpritePriority>>6) & 3;
	CFG.SpritePr[3] = (SpritePriority>>9) & 3;
	
	
	CFG.AutoSRAM = get_config_int(section, "AutoSRAM", CFG.AutoSRAM);
}


void saveOptionsToConfig(sint8 *section)
{
	set_config_int(section, "BG3Squish", CFG.BG3Squish);
	// FIXME 
	set_config_int(section, "YScroll", GFX.YScroll);
	set_config_int(section, "Sound", CFG.Sound_output);
	
	set_config_int(section, "Scaled", CFG.Scaled);
	//	set_config_int(section, "GFXEngine", CFG.TileMode);
	//	set_config_int(section, "HDMA", CFG.BG_Layer>>7);

	set_config_oct(section, "BGLayers", 5, (CFG.BG_Layer & 1)|((CFG.BG_Layer
			& 2)<<2)|((CFG.BG_Layer & 4)<<4)|((CFG.BG_Layer & 8)<<6)
			|((CFG.BG_Layer & 0x10)<<8));

	set_config_int(section, "BGPriorities", CFG.LayersConf);

	//	set_config_int(section, "Transparency", CFG.Transparency);
	set_config_int(section, "Vblank", CFG.WaitVBlank);
	set_config_int(section, "SpeedHacks", CFG.CPU_speedhack);
	//	set_config_int(section, "FastDMA", CFG.FastDMA);

	/*	set_config_hex(section, "MouseXAddr", 0);
	 set_config_hex(section, "MouseYAddr", 0);
	 set_config_int(section, "MouseMode", 0);
	 set_config_int(section, "MouseXOffset", 0);
	 set_config_int(section, "MouseYOffset", 0);*/

	//	set_config_oct(section, "SoundPortSync", CFG.SoundPortSync);

	/*	set_config_int(section, "TilePriorityBG", CFG.TilePriorityBG);
	 set_config_int(section, "BG3TilePriority", CFG.BG3TilePriority);
	 set_config_int(section, "BlankTileNumber", CFG.Debug2);
	 set_config_oct(section, "SpritePriority", 01123);*/
	
	set_config_int(section, "AutoSRAM", CFG.AutoSRAM);
	save_config_file();
}

//from config -> PPU snes engine
void PPU_ChangeLayerConf(int i)
{
	CFG.LayersConf = i % 10;
	CFG.LayerPr[0] = LayersConf[CFG.LayersConf][0];
	CFG.LayerPr[1] = LayersConf[CFG.LayersConf][1];
	CFG.LayerPr[2] = LayersConf[CFG.LayersConf][2];
	CFG.LayerPr[3] = LayersConf[CFG.LayersConf][3];
}


void applyOptions()
{
	if (!CFG.Sound_output)
		APU_clear();

	if (CFG.LayersConf < 10)
		PPU_ChangeLayerConf(CFG.LayersConf);

	GFX.YScroll = _offsetY_tab[CFG.YScroll];
}




// FIXME : fix layersconf

void packOptions(uint8 *ptr)
{
	t_Options *opt = (t_Options *)ptr;

	opt->BG3Squish = CFG.BG3Squish;
	opt->SoundOutput = CFG.Sound_output;
	if (CFG.LayersConf == 0)
		opt->LayersConf = 0x24; // 0/1/2
	else
		opt->LayersConf = CFG.LayerPr[0] | (CFG.LayerPr[1] << 2)
				| (CFG.LayerPr[2] << 4);
//	opt->TileMode = CFG.TileMode;
	opt->BG_Layer = CFG.BG_Layer;
	opt->YScroll = CFG.YScroll;
	opt->WaitVBlank = CFG.WaitVBlank;
	opt->SpeedHack = CFG.CPU_speedhack;
}

void unpackOptions(int version, uint8 *ptr)
{
	t_Options *opt = (t_Options *)ptr;

	if (version == 1)
		CFG.BG3Squish = 2-opt->BG3Squish;
	else
		CFG.BG3Squish = opt->BG3Squish;
	CFG.Sound_output = opt->SoundOutput;
	if (version == 1)
		CFG.LayersConf = opt->LayersConf;
	else
	{
		if (opt->LayersConf == 0x24) // 0/1/2 == automatic layer
		{
			CFG.LayersConf = 0;
		}
		else
		{
			CFG.LayersConf = 10;
			CFG.LayerPr[0] = opt->LayersConf&3;
			CFG.LayerPr[1] = (opt->LayersConf>>2)&3;
			CFG.LayerPr[2] = (opt->LayersConf>>4)&3;
			CFG.LayerPr[3] = 3;
		}
	}
/*	if (version == 1)
		CFG.TileMode = 0; // Force line by line mode
	else
		CFG.TileMode = opt->TileMode;*/
	CFG.BG_Layer = opt->BG_Layer;
	CFG.YScroll = opt->YScroll;
	CFG.WaitVBlank = opt->WaitVBlank;
	CFG.CPU_speedhack = opt->SpeedHack;

	applyOptions();
}

int checkConfiguration(sint8 *name, int crc)
{
	// Check configuration file
	readOptionsFromConfig("Global");

	sint8 *section= NULL;
	if (is_section_exists(SNES.ROM_info.title))
	{
		section = SNES.ROM_info.title;
	}
	else if (is_section_exists(FS_getFileName(name)))
	{
		section = FS_getFileName(name);
	}
	else if ((section = find_config_section_with_hex("crc", crc)))
	{
	}
	else if ((section = find_config_section_with_string("title2", SNES.ROM_info.title)))
	{
	}
	else if ((section = find_config_section_with_hex("crc2", crc)))
	{
	}
	else if ((section = find_config_section_with_string("title3", SNES.ROM_info.title)))
	{
	}
	else if ((section = find_config_section_with_hex("crc3", crc)))
	{
	}
	else if ((section = find_config_section_with_string("title4", SNES.ROM_info.title)))
	{
	}
	else if ((section = find_config_section_with_hex("crc4", crc)))
	{
	}

	if (section != NULL)
	{
		printf("Section : %s", section);
		readOptionsFromConfig(section);
	}
	
	return 0;
}

char tmpFile[512];
bool zipFileLoaded = false;
//requires sint8 * name to be romname.ext
int loadROM(sint8 *name, int confirm)
{
	// Save SRAM of previous game first
	saveSRAM();
	
	//update name to ROMFile so current RomFile is used as global
	sprintf(CFG.ROMFile,"%s",(char*)name);
	
	//Build full path to load (new file)
	sprintf(CFG.Fullpath,"%s/%s",CFG.ROMPath,CFG.ROMFile);	//0:/snes + /rom.smc <--- this format!!
	
	if(strstr (_FS_getFileExtension(name),"ZIP")){	
		zipFileLoaded = true;
	}
	else{
		zipFileLoaded = false;
	}
	
	if(zipFileLoaded == true){
		sprintf(CFG.ZipFullpath,"%s/%s",CFG.ROMPath,tmpFile);	//rets path+rom.smc	/ok
		
		//Decompress File for reload later
		volatile char buftemp[0x200];
		sprintf((char*)buftemp,"%s%s",getfatfsPath("snes/"),tmpFile);
		
		volatile char bufzip[0x200];
		sprintf((char*)bufzip,"%s%s",getfatfsPath("snes/"),CFG.ROMFile);
		int stat = load_gz((char*)bufzip, (char*)buftemp);
		
		sprintf(CFG.Fullpath,"%s",bufzip);	//fullpath .zip
		//CFG.ZipFullpath	//tempfile load/streamed
		
		//clrscr();
		//printf("zip file:");
		//printf("%s",bufzip);
		
		//printf("%s",CFG.ZipFullpath);
		//sprintf((char*)buftemp,"%s",CFG.Fullpath);
		//char * result = str_replace((char*)buftemp, (char *)".zip", (char *)".smc");
		
		//if(result){
		//	snprintf ((char*)&CFG.ZipFullpathRealName[0], strlen(result)+1,"%s ",result);
		//	free(result);
		//}
		//else{
		//	clrscr();
		//	printf("INVALID ZIP NAME");
		//	while(1){}
		//}
		
		//clrscr();
		//printf("1:%s",(char*)CFG.ZipFullpath);			
		//printf("2:%s",(char*)CFG.Fullpath);				
		//printf("3:%s",(char*)CFG.ZipFullpathRealName);	//fullpath .smc
		
	}
	/*
	clrscr();
	if(zipFileLoaded == true){
		printf("ISZIP");
		printf("file:%s",CFG.ZipFullpath);
	}
	else{
		printf("NOTZIP");
		printf("file:%s",CFG.Fullpath);
	}
	while (1)
	{
		if ((keysPressed() & KEY_A)){
			break;
		}
		IRQWait(1,IRQ_VBLANK);
	}
	*/
	if(rom_buffer){
		free(rom_buffer);
		mem_clear_paging();
	}
	rom_buffer = (uint8*)malloc(ROM_MAX_SIZE);
	
	if(!rom_buffer){
		printf("CRITICAL COULDNT ALLOCATE MEM");	//this should never happen
		while(1){}
	}
	rom_page = rom_buffer + (ROM_STATIC_SIZE*1);
	
	int size;
	int ROMheader;
	sint8 *ROM = (sint8*)(uint8*)rom_buffer;
	SNESC.ROM = (uchar*)rom_buffer;
	int crc;
	
	GUI_clear();
	CFG.LargeROM = 0;
	
	if(zipFileLoaded == true){
		size = FS_getFileSize(CFG.ZipFullpath);
	}
	else{
		size = FS_getFileSize(CFG.Fullpath);
	}
	
	ROMheader = size & 8191;
	if (ROMheader != 0&& ROMheader != 512)
		ROMheader = 512;

	//fit EWRAM? if not use ROM_STATIC_SIZE page size
	if (size-ROMheader > ROM_MAX_SIZE)
	{
		if(zipFileLoaded == true){
			FS_loadROMForPaging(ROM-ROMheader, CFG.ZipFullpath, ROM_STATIC_SIZE+ROMheader);
		}
		else{
			FS_loadROMForPaging(ROM-ROMheader, CFG.Fullpath, ROM_STATIC_SIZE+ROMheader);
		}
		CFG.LargeROM = 1;
		crc = crc32(0, ROM, ROM_STATIC_SIZE);
		printf("Large ROM detected. CRC(1Mb) = %08x ", crc);
	}
	//ewram size is enough here so read data entirely
	else
	{
		if(zipFileLoaded == true){
			FS_loadROM(ROM-ROMheader, CFG.ZipFullpath);
		}
		else{
			FS_loadROM(ROM-ROMheader, CFG.Fullpath);
		}
		CFG.LargeROM = 0;
		crc = crc32(0, ROM, size-ROMheader);
		printf("CRC = %08x ", crc);
	}

	changeROM(ROM-ROMheader, size);
	
	if(zipFileLoaded == true){
		checkConfiguration(CFG.ZipFullpath, crc);
	}
	else{
		checkConfiguration(CFG.Fullpath, crc);
	}
    if(SNES.HiROM == 0){
        printf("SNESROM is LoROM.Press A");
    }
    else if(SNES.HiROM == 1)
        printf("SNESROM is SNES2.HiROM.Press A");
    else{
        printf("An error as ocurred SNES2.HiROM: %d.Press A",SNES.HiROM);
    }
	
    while(1 == 1){    
        if (keysPressed()&KEY_A){
            break;
        }
		IRQWait(1,IRQ_VBLANK);
    }
    
	return 0;
}

int	changeROM(sint8 *ROM, int size)
{
  CFG.frame_rate = 1;
  CFG.DSP1 = CFG.SuperFX = 0;
  CFG.InterleavedROM = CFG.InterleavedROM2 = 0;
  CFG.MouseXAddr = CFG.MouseYAddr = CFG.MouseMode = 0;
  CFG.SoundPortSync = 0;
  
  CFG.TilePriorityBG = -1; CFG.Debug2 = 0;
  
  CFG.SpritePr[0] = 3;
  CFG.SpritePr[1] = 2;
  CFG.SpritePr[2] = 1;
  CFG.SpritePr[3] = 1;
  
#ifdef IN_EMULATOR
  CFG.Sound_output = 0;
#endif

	// Write SRAM
    load_ROM(ROM, size);
    SNES.ROM_info.title[20] = '\0';
    int i = 20;
    while (i >= 0 && SNES.ROM_info.title[i] == ' '){
    	SNES.ROM_info.title[i--] = '\0';
	}
	GUI_showrominfo(size);
	
    reset_SNES();	
	// Clear screen
	// Read SRAM
    loadSRAM();	
	return 0;
}