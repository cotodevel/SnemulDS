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

#include "engine.h"

#ifdef WIN32
	#include <allegro.h>
#endif
#include <stdio.h>
#include <time.h>
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
#include "opcodes.h"
#include "common.h"
#include "specific_shared.h"
#include "nds_cp15_misc.h"
#include "fsfatlayerTGDS.h"
#include "about.h"
#include "utilsTGDS.h"

void writeSRAM(int offset, uint8* src, int size) {
        EXMEMCNT &= ~0x0880;
        uint8* dest = DS_SRAM+offset;
        while (size--)
        {
                *dest++ = *src++; 
        }
        EXMEMCNT |= 0x0880;
}

void readSRAM(int offset, uint8* dest, int size) {
        EXMEMCNT &= ~0x0880;
        uint8* src = DS_SRAM+offset;
        while (size--)
        {
                *dest++ = *src++;
        }
        EXMEMCNT |= 0x0880;
} 


int loadSRAM()
{
  sint8 sramTemp[100];
  sint8 sramFile[100];
  
  if (SNESC.SRAMMask)
    {
    	strcpy(sramTemp, CFG.ROMFile);
		strcpy(strrchr(sramTemp, '.'), ".SRM");
		sprintf(sramFile,"%s/%s",CFG.ROMPath,sramTemp);	//0:/snes/filename.srm
		//clrscr();
		//printf("loadSRAM");
		//printf("sramfile:%s",sramFile);
		//while(1);
    	FS_loadFile(sramFile, (sint8 *)SNESC.SRAM, SNESC.SRAMMask+1);
    }	
	return 0;    
}


int saveSRAM()
{
  sint8 sramTemp[100];
  sint8 sramFile[100];
  	
  if (SNESC.SRAMMask)
    {
    	strcpy(sramTemp, CFG.ROMFile);
		strcpy(strrchr(sramTemp, '.'), ".SRM");
		sprintf(sramFile,"%s/%s",CFG.ROMPath,sramTemp);	//0:/snes/filename.srm
		//clrscr();
		//printf("saveSRAM");
		//printf("sramfile:%s",sramFile);
		//while(1);
    	FS_saveFile(sramFile, (sint8 *)SNESC.SRAM, SNESC.SRAMMask+1,false);	//force_file_creation == false here (we could destroy or corrupt saves..)
    }	
	return 0;
}

__attribute__((section(".dtcm")))
uint8 interrupted;

//__attribute__((section(".dtcm")))	//could cause issues if disabled
//extern long Cycles;


int initSNESEmpty()
{
    CFG.BG3Squish = 0;
    CFG.WaitVBlank = 0;
    CFG.YScroll = 0;
    CFG.CPU_speedhack = 1;
    //CFG.TileMode = 1;
    CFG.Scaled = 0;
    CFG.LayersConf = 0;

    CFG.frame_rate = 1;
    CFG.DSP1 = CFG.SuperFX = 0;
    CFG.InterleavedROM = CFG.InterleavedROM2 = 0;
    CFG.Sound_output = 1;
    //CFG.Sound_output = 0;
    CFG.FastDMA = 1;
    CFG.Transparency = 1;

    memset((uint32*)&SNES, 0, sizeof(SNES));
    memset((uint32*)&SNESC, 0, sizeof(SNESC));
	
	SNESC.ROM = (uchar *)NULL;	//Rom is reallocated on file load
    SNESC.RAM = (uchar *)&snes_ram_bsram[0x6000];
    update_ram_snes();
	SNESC.VRAM = (uchar *)&snes_vram[0];
    SNESC.BSRAM = (uchar *)&snes_ram_bsram[0x0];
	
    init_GFX();

    GFX.Graph_enabled = 1;
	
    //  printf("Init OK...\n");
    return 0;
}

__attribute__((section(".dtcm")))
int OldPC;

int go()
{
    if (CPU.IsBreak) {
        return 0;
    }

    while (1)
    {			
        if ((GFX.v_blank) && (SNES.v_blank ==1))
        {
            if (GFX.need_update)
            {
                draw_screen();
                GFX.need_update = 0;
            }
            GFX.v_blank = 0;
            update_joypads();	
            return 0;
        }


        CPU.HCycles = SNES.UsedCycles;	
        if (CPU.DMA_PORT[0x00]&HV_IRQ_HH_V_ANY) 
        {
            if (!(CPU.DMA_PORT[0x00]&HV_IRQ_VV_H_0) || SNES.V_Count == (CPU.DMA_PORT[0x09]&0xff)+((CPU.DMA_PORT[0x0A]&0xff)<<8)) 
            {
                int H_cycles;
                H_cycles = (CPU.DMA_PORT[0x07]+(CPU.DMA_PORT[0x08]<<8))/2;
                SET_WAITCYCLES(H_cycles);
                CPU.WaitAddress = -1;

                CPU_goto(H_cycles); 
                if (CPU.IsBreak) 
                    return 0;
                CPU.HCycles += H_cycles;	
                //GoIRQ();
                setirq(PPU_H_BEAM_IRQ_SOURCE);
                CPU.DMA_PORT[0x11] = 0x80;
            }
        }

        SET_WAITCYCLES(NB_CYCLES-CPU.HCycles);
        CPU.WaitAddress = -1; 
        CPU_goto(NB_CYCLES-CPU.HCycles); 
        if (CPU.IsBreak) 
            return 0;
        CPU.HCycles = NB_CYCLES;

        SNES.UsedCycles = 0;

        // HBLANK Starts here
        SNES.V_Count++;
        if (SNES.V_Count > (SNES.NTSC ? 261 : 311))
        {
            SNES.V_Count = 0;
            update_joypads();
        }
        #if 0
        if (CFG.Sound_output) {
            if (SNES.V_Count & 63) {
                if (SNES.V_Count & 1) {
                    if (!SNES.NTSC && ((SNES.V_Count&7) == 1)) {
                        MyIPC->TIM0++; MyIPC->TIM1++; MyIPC->TIM2+=4;
                    }
                    if (++MyIPC->TIM0 >= MyIPC->T0) {
                        MyIPC->TIM0 -= MyIPC->T0; MyIPC->CNT0++;
                        //if (MyIPC->CONTROL&1) { SPC700_emu = 1; APU_WaitCounter++; }
                    }
                    if (++MyIPC->TIM1 >= MyIPC->T1) {
                        MyIPC->TIM1 -= MyIPC->T1; MyIPC->CNT1++;
                        //if (MyIPC->CONTROL&2) { SPC700_emu = 1; APU_WaitCounter++; }
                    }
                }
                MyIPC->TIM2 += 4;
                if (MyIPC->TIM2 >= MyIPC->T2) {
                    MyIPC->TIM2 -= MyIPC->T2; MyIPC->CNT2++;
                    //if (APU.CONTROL&4) { SPC700_emu = 1; APU_WaitCounter++; }
                }
            }
        }
        #endif
        if (SNES.V_Count < GFX.ScreenHeight) 
        {   	
            #if 0    	
            if (GFX.Sprites_table_dirty) {
                /*        if ((CFG.BG_Layer&0x10))
                draw_sprites();*/
                //draw_screen();    

                GFX.Sprites_table_dirty = 0;
            }
            #endif      
            if (!(CPU.PPU_PORT[0x00]&0x80) && CPU.DMA_PORT[0x0C] && CFG.BG_Layer&0x80)
                HDMA_write();
            SNES.HDMA_line++;

            // draw line
            if (CFG.Scaled)
                PPU_line_render_scaled();
            else
                PPU_line_render();	//generates GFX.lineInfo[index] (populates SNES IO -> DS PPU IO background) + refill sprites from SNES -> DS PPU format
        }
        if (
            (CPU.DMA_PORT[0x00]&HV_IRQ_VV_H_0) && !(CPU.DMA_PORT[0x00]&HV_IRQ_HH_V_ANY) 
            && 
            SNES.V_Count == ((CPU.DMA_PORT[0x09]&0xff)+((CPU.DMA_PORT[0x0A]&0xff)<<8))
           ){
            //GoIRQ();
            setirq(PPU_V_BEAM_IRQ_SOURCE);
        }
        
        if (SNES.V_Count == (SNES.NTSC ? 261 : 311)) {
            // V_blank
            SNES.v_blank = 0; CPU.DMA_PORT[0x10] = 0;
            if (CPU.DMA_PORT[0x0C] && CFG.BG_Layer&0x80) {
                if (CPU.DMA_PORT[0x0C]&0x01) HDMA_transfert(0);
                if (CPU.DMA_PORT[0x0C]&0x02) HDMA_transfert(1);
                if (CPU.DMA_PORT[0x0C]&0x04) HDMA_transfert(2);
                if (CPU.DMA_PORT[0x0C]&0x08) HDMA_transfert(3);
                if (CPU.DMA_PORT[0x0C]&0x10) HDMA_transfert(4);
                if (CPU.DMA_PORT[0x0C]&0x20) HDMA_transfert(5);
                if (CPU.DMA_PORT[0x0C]&0x40) HDMA_transfert(6);
                if (CPU.DMA_PORT[0x0C]&0x80) HDMA_transfert(7);
            }
            //memset(GFX.tiles_ry,8,4);
            //if (GFX.frame_d && (CFG.BG_Layer & 0x10))
            //draw_sprites();

            draw_screen();
            GFX.BG_scroll_reg = 0;
            SNES.HDMA_line = 0;
            CPU.PPU_PORT[0x02] = GFX.Old_SpriteAddress;
            GFX.OAM_upper_byte = 0;
            GFX.brightness = CPU.PPU_PORT[0x00]&0xF;
        }
    
        if (SNES.V_Count == GFX.ScreenHeight || SNES.DelayedNMI) {
            
            if (CPU.DMA_PORT[0x00]&VBLANK_NMI_IRQENABLE){ 
                //GoNMI();
                setirq(SNES_IRQ_SOURCE);
            }
            
            SNES.HIRQ_ok = 1;
            GFX.nb_frames++;

            if (GFX.nb_frames >= 100 && !GUI.log && !CFG.Debug)
            {
                //scanKeys();
                GFX.speed = GFX.nb_frames * 100 / (GFX.DSFrame - GFX.DSLastFrame);
                GFX.DSLastFrame = GFX.DSFrame;  
                //GUI_console_printf(0, 23, "% 3d%%             ",  GFX.speed);
                //        GUI_console_printf(20, 23, "DBG=%d", CFG.Debug2),

                //time_t unixTime = time(NULL);
                //struct tm* timeStruct = gmtime((const time_t *)&unixTime);
                //GUI_console_printf(26, 23, "%02d:%02d", timeStruct->tm_hour, timeStruct->tm_min);	        	

                GFX.nb_frames = 0;

                if (CFG.AutoSRAM && SNES.SRAMWritten)
                {
                    saveSRAM();
                    //GUI_console_printf(0, 23, "Auto SRAM written");
                    SNES.SRAMWritten = 0;
                }
            }
            
            if (!CPU.NMIActive) 
                CPU.DMA_PORT[0x10] |= VBLANK_NMI_IRQENABLE;
            SNES.v_blank = 1; CPU.NMIActive = 0;
            update_joypads();
            SNES.DelayedNMI = 0;
        }
    }

    return 0;
}

void show_opcode(sint8 *buf, uint8 opcode, int pc, int pb, unsigned short flags)
{
  switch(opcode) {
    case 0xEA : sprintf(buf, "NOP"); break;
    case 0x14 : sprintf(buf, "TRB $%02X",mem_getbyte(pc+1,pb)); break;
    case 0x1C : sprintf(buf, "TRB $%04X", mem_getword(pc+1,pb)); break;
    case 0x04 : sprintf(buf, "TSB $%02X",mem_getbyte(pc+1,pb)); break;
    case 0x0C : sprintf(buf, "TSB $%04X", mem_getword(pc+1,pb)); break;
    case 0x29 :
	if (flags&P_M)  {
		sprintf(buf, "AND #%02X", mem_getbyte(pc+1,pb));
	}
	else {
		sprintf(buf, "AND #%04X", mem_getword(pc+1,pb));
	}
	break;
    case 0x21 : sprintf(buf, "AND ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0x31 : sprintf(buf, "AND ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0x32 : sprintf(buf, "AND ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0x33 : sprintf(buf, "AND ($%02X,S),Y", mem_getbyte(pc+1,pb)); break;
    case 0x27 : sprintf(buf, "AND [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0x37 : sprintf(buf, "AND [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0x23 : sprintf(buf, "AND $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0x25 : sprintf(buf, "AND $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x35 : sprintf(buf, "AND $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x3D : sprintf(buf, "AND $%04X,X", mem_getword(pc+1,pb)); break;
    case 0x39 : sprintf(buf, "AND $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0x2D : sprintf(buf, "AND $%04X", mem_getword(pc+1,pb)); break;
    case 0x2F : sprintf(buf, "AND $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x3F : sprintf(buf, "AND $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x09 :
	if (flags&P_M){
		sprintf(buf, "ORA #%02X", mem_getbyte(pc+1,pb));
	}
	else {
		sprintf(buf, "ORA #%04X", mem_getword(pc+1,pb)); 
	}
	break;
    case 0x01 : sprintf(buf, "ORA ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0x13 : sprintf(buf, "ORA ($%02X,S),Y", mem_getbyte(pc+1,pb)); break;
    case 0x07 : sprintf(buf, "ORA [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0x17 : sprintf(buf, "ORA [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0x03 : sprintf(buf, "ORA $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0x05 : sprintf(buf, "ORA $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x15 : sprintf(buf, "ORA $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x1D : sprintf(buf, "ORA $%04X,X", mem_getword(pc+1,pb)); break;
    case 0x19 : sprintf(buf, "ORA $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0x0D : sprintf(buf, "ORA $%04X", mem_getword(pc+1,pb)); break;
    case 0x0F : sprintf(buf, "ORA $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x1F : sprintf(buf, "ORA $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x49 :
	if (flags&P_M){
		sprintf(buf, "EOR #%02X", mem_getbyte(pc+1,pb));
	}
	else {
		sprintf(buf, "EOR #%04X", mem_getword(pc+1,pb)); 
	}	
	break;
    case 0x41 : sprintf(buf, "EOR ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0x53 : sprintf(buf, "EOR ($%02X,S),Y", mem_getbyte(pc+1,pb)); break;
    case 0x45 : sprintf(buf, "EOR $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x47 : sprintf(buf, "EOR [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0x43 : sprintf(buf, "EOR $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0x57 : sprintf(buf, "EOR [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0x55 : sprintf(buf, "EOR $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x4D : sprintf(buf, "EOR $%04X", mem_getbyte(pc+1,pb)); break;
    case 0x5D : sprintf(buf, "EOR $%04X,X", mem_getbyte(pc+1,pb)); break;
    case 0x59 : sprintf(buf, "EOR $%04X,Y", mem_getbyte(pc+1,pb)); break;
    case 0x5F : sprintf(buf, "EOR $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x06 : sprintf(buf, "ASL $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x16 : sprintf(buf, "ASL $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0x0E : sprintf(buf, "ASL $%04X", mem_getword(pc+1, pb)); break;
    case 0x1E : sprintf(buf, "ASL $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x0A : sprintf(buf, "ASL A"); break;
    case 0x46 : sprintf(buf, "LSR $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x56 : sprintf(buf, "LSR $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0x4E : sprintf(buf, "LSR $%04X", mem_getword(pc+1, pb)); break;
    case 0x5E : sprintf(buf, "LSR $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x4A : sprintf(buf, "LSR A"); break;
    case 0xE6 : sprintf(buf, "INC $%02X", mem_getbyte(pc+1, pb)); break;
    case 0xF6 : sprintf(buf, "INC $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0xEE : sprintf(buf, "INC $%04X", mem_getword(pc+1, pb)); break;
    case 0xFE : sprintf(buf, "INC $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x1A : sprintf(buf, "INC A"); break;
    case 0xE8 : sprintf(buf, "INX"); break;
    case 0xC8 : sprintf(buf, "INY"); break;
    case 0xC6 : sprintf(buf, "DEC $%02X", mem_getbyte(pc+1, pb)); break;
    case 0xD6 : sprintf(buf, "DEC $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0xCE : sprintf(buf, "DEC $%04X", mem_getword(pc+1, pb)); break;
    case 0xDE : sprintf(buf, "DEC $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x3A : sprintf(buf, "DEC A"); break;
    case 0xCA : sprintf(buf, "DEX"); break;
    case 0x88 : sprintf(buf, "DEY"); break;
    case 0x89 :
	if (flags&P_M){
		sprintf(buf, "BIT #%02X", mem_getbyte(pc+1,pb));
	}
	else{ 
		sprintf(buf, "BIT #%04X", mem_getword(pc+1,pb)); 
	}	
	break;
    case 0x24 : sprintf(buf, "BIT $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x34 : sprintf(buf, "BIT $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x2C : sprintf(buf, "BIT $%04X", mem_getword(pc+1,pb)); break;
    case 0x3C : sprintf(buf, "BIT $%04X,X", mem_getword(pc+1,pb)); break;
    case 0x26 : sprintf(buf, "ROL $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x36 : sprintf(buf, "ROL $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0x2E : sprintf(buf, "ROL $%04X", mem_getword(pc+1, pb)); break;
    case 0x3E : sprintf(buf, "ROL $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x2A : sprintf(buf, "ROL A"); break;
    case 0x66 : sprintf(buf, "ROR $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x76 : sprintf(buf, "ROR $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0x6E : sprintf(buf, "ROR $%04X", mem_getword(pc+1, pb)); break;
    case 0x7E : sprintf(buf, "ROR $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x6A : sprintf(buf, "ROR A"); break;
    case 0x64 : sprintf(buf, "STZ $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x74 : sprintf(buf, "STZ $%02X,X", mem_getbyte(pc+1, pb)); break;
    case 0x9C : sprintf(buf, "STZ $%04X", mem_getword(pc+1, pb)); break;
    case 0x9E : sprintf(buf, "STZ $%04X,X", mem_getword(pc+1, pb)); break;
    case 0x69 :
	if (flags&P_M){
		sprintf(buf, "ADC #%02X", mem_getbyte(pc+1,pb));
	}
    else {
		sprintf(buf, "ADC #%04X", mem_getword(pc+1,pb)); 
	}	
	break;
    case 0x61 : sprintf(buf, "ADC ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0x71 : sprintf(buf, "ADC ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0x72 : sprintf(buf, "ADC ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0x73 : sprintf(buf, "ADC ($%02X,S),Y", mem_getbyte(pc+1,pb)); break;
    case 0x67 : sprintf(buf, "ADC [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0x77 : sprintf(buf, "ADC [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0x63 : sprintf(buf, "ADC $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0x65 : sprintf(buf, "ADC $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x75 : sprintf(buf, "ADC $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x7D : sprintf(buf, "ADC $%04X,X", mem_getword(pc+1,pb)); break;
    case 0x79 : sprintf(buf, "ADC $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0x6D : sprintf(buf, "ADC $%04X", mem_getword(pc+1,pb)); break;
    case 0x6F : sprintf(buf, "ADC $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x7F : sprintf(buf, "ADC $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xE9 :
	if (flags&P_M){
		sprintf(buf, "SBC #%02X", mem_getbyte(pc+1,pb));
	}
	else {
		sprintf(buf, "SBC #%04X", mem_getword(pc+1,pb)); 
	}
	break;
    case 0xE1 : sprintf(buf, "SBC ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0xF1 : sprintf(buf, "SBC ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0xF2 : sprintf(buf, "SBC ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0xF3 : sprintf(buf, "SBC ($%02X,S),Y", mem_getbyte(pc+1,pb)); break;
    case 0xE7 : sprintf(buf, "SBC [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0xF7 : sprintf(buf, "SBC [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0xE3 : sprintf(buf, "SBC $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0xE5 : sprintf(buf, "SBC $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xF5 : sprintf(buf, "SBC $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0xFD : sprintf(buf, "SBC $%04X,X", mem_getword(pc+1,pb)); break;
    case 0xF9 : sprintf(buf, "SBC $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0xED : sprintf(buf, "SBC $%04X", mem_getword(pc+1,pb)); break;
    case 0xEF : sprintf(buf, "SBC $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xFF : sprintf(buf, "SBC $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x81 : sprintf(buf, "STA ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0x91 : sprintf(buf, "STA ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0x92 : sprintf(buf, "STA ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0x87 : sprintf(buf, "STA [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0x97 : sprintf(buf, "STA [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0x83 : sprintf(buf, "STA $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0x85 : sprintf(buf, "STA $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x95 : sprintf(buf, "STA $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x9D : sprintf(buf, "STA $%04X,X", mem_getword(pc+1,pb)); break;
    case 0x99 : sprintf(buf, "STA $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0x8D : sprintf(buf, "STA $%04X", mem_getword(pc+1,pb)); break;
    case 0x8F : sprintf(buf, "STA $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x9F : sprintf(buf, "STA $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0x84 : sprintf(buf, "STY $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x94 : sprintf(buf, "STY $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0x8C : sprintf(buf, "STY $%04X", mem_getword(pc+1,pb)); break;
    case 0x86 : sprintf(buf, "STX $%02X", mem_getbyte(pc+1,pb)); break;
    case 0x96 : sprintf(buf, "STX $%02X,Y", mem_getbyte(pc+1,pb)); break;
    case 0x8E : sprintf(buf, "STX $%04X", mem_getword(pc+1,pb)); break;
    case 0xA9 :
	if (flags&P_M){
		sprintf(buf, "LDA #%02X", mem_getbyte(pc+1,pb));
	}
    else {
		sprintf(buf, "LDA #%04X", mem_getword(pc+1,pb)); 
	}
	break;
    case 0xA1 : sprintf(buf, "LDA ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0xB1 : sprintf(buf, "LDA ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0xB2 : sprintf(buf, "LDA ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0xA7 : sprintf(buf, "LDA [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0xB7 : sprintf(buf, "LDA [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0xA3 : sprintf(buf, "LDA $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0xA5 : sprintf(buf, "LDA $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xB5 : sprintf(buf, "LDA $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0xBD : sprintf(buf, "LDA $%04X,X", mem_getword(pc+1,pb)); break;
    case 0xB9 : sprintf(buf, "LDA $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0xAD : sprintf(buf, "LDA $%04X", mem_getword(pc+1,pb)); break;
    case 0xAF : sprintf(buf, "LDA $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xBF : sprintf(buf, "LDA $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xA0 :
	if (flags&P_X){
		sprintf(buf, "LDY #%02X", mem_getbyte(pc+1,pb));
	}
    else {
		sprintf(buf, "LDY #%04X", mem_getword(pc+1,pb)); 
	}
	break;
    case 0xA4 : sprintf(buf, "LDY $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xB4 : sprintf(buf, "LDY $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0xAC : sprintf(buf, "LDY $%04X", mem_getword(pc+1,pb)); break;
    case 0xBC : sprintf(buf, "LDY $%04X,X", mem_getword(pc+1,pb)); break;
    case 0xA2 :
	if (flags&P_X){
		sprintf(buf, "LDX #%02X", mem_getbyte(pc+1,pb));
	}
	else {
		sprintf(buf, "LDX #%04X", mem_getword(pc+1,pb)); 
	}
	break;
    case 0xA6 : sprintf(buf, "LDX $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xB6 : sprintf(buf, "LDX $%02X,Y", mem_getbyte(pc+1,pb)); break;
    case 0xAE : sprintf(buf, "LDX $%04X", mem_getword(pc+1,pb)); break;
    case 0xC9 :
	if (flags&P_M){
		sprintf(buf, "CMP #%02X", mem_getbyte(pc+1,pb));
	}
    else {
		sprintf(buf, "CMP #%04X", mem_getword(pc+1,pb)); 
	}	
	break;
    case 0xC1 : sprintf(buf, "CMP ($%02X,X)", mem_getbyte(pc+1,pb)); break;
    case 0xD1 : sprintf(buf, "CMP ($%02X),Y", mem_getbyte(pc+1,pb)); break;
    case 0xD2 : sprintf(buf, "CMP ($%02X)", mem_getbyte(pc+1,pb)); break;
    case 0xC7 : sprintf(buf, "CMP [$%02X]", mem_getbyte(pc+1,pb)); break;
    case 0xD7 : sprintf(buf, "CMP [$%02X],Y", mem_getbyte(pc+1,pb)); break;
    case 0xC3 : sprintf(buf, "CMP $%02X,S", mem_getbyte(pc+1, pb)); break;
    case 0xC5 : sprintf(buf, "CMP $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xD5 : sprintf(buf, "CMP $%02X,X", mem_getbyte(pc+1,pb)); break;
    case 0xDD : sprintf(buf, "CMP $%04X,X", mem_getword(pc+1,pb)); break;
    case 0xD9 : sprintf(buf, "CMP $%04X,Y", mem_getword(pc+1,pb)); break;
    case 0xCD : sprintf(buf, "CMP $%04X", mem_getword(pc+1,pb)); break;
    case 0xCF : sprintf(buf, "CMP $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xDF : sprintf(buf, "CMP $%02X:%04X,X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1,pb)); break;
    case 0xC0 :
	if (flags&P_X){
		sprintf(buf, "CPY #%02X", mem_getbyte(pc+1,pb));
	}
    else{
		sprintf(buf, "CPY #%04X", mem_getword(pc+1,pb)); 
	}	
	break;
    case 0xC4 : sprintf(buf, "CPY $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xCC : sprintf(buf, "CPY $%04X", mem_getword(pc+1,pb)); break;
    case 0xE0 :
	if (flags&P_X){
		sprintf(buf, "CPX #%02X", mem_getbyte(pc+1,pb));
	}
    else{
		sprintf(buf, "CPX #%04X", mem_getword(pc+1,pb)); 
	}
	break;
    case 0xE4 : sprintf(buf, "CPX $%02X", mem_getbyte(pc+1,pb)); break;
    case 0xEC : sprintf(buf, "CPX $%04X", mem_getword(pc+1,pb)); break;
    case 0xC2 : sprintf(buf, "REP #%02X", mem_getbyte(pc+1,pb)); break;
    case 0xE2 : sprintf(buf, "SEP #%02X", mem_getbyte(pc+1,pb)); break;
    case 0xD0 : sprintf(buf, "BNE %04X", pc+1+(sint8)mem_getbyte(pc+1, pb)+1); break;
    case 0xF0 : sprintf(buf, "BEQ %04X", pc+1+(sint8)mem_getbyte(pc+1, pb)+1); break;
    case 0x10 : sprintf(buf, "BPL %04X", pc+1+(sint8)mem_getbyte(pc+1, pb)+1); break;
    case 0x30 : sprintf(buf, "BMI %04X", pc+1+(sint8)mem_getbyte(pc+1, pb)+1); break;
    case 0x90 : sprintf(buf, "BCC %04X", pc+1+(sint8)mem_getbyte(pc+1, pb)+1); break;
    case 0xB0 : sprintf(buf, "BCS %04X", pc+1+(sint8)mem_getbyte(pc+1, pb)+1); break;
    case 0x80 : sprintf(buf, "BRA %04X", pc+1+(sint8)mem_getbyte(pc+1, pb)+1); break;
    case 0x70 : sprintf(buf, "BVS %04X", pc+1+(sint8)mem_getbyte(pc+1, pb)+1); break;
    case 0x82 : sprintf(buf, "BRL %04X", pc+1+(short)mem_getword(pc+1, pb)+2); break;
    case 0x08 : sprintf(buf, "PHP"); break;
    case 0x48 : sprintf(buf, "PHA"); break;
    case 0xDA : sprintf(buf, "PHX"); break;
    case 0x5A : sprintf(buf, "PHY"); break;
    case 0x4B : sprintf(buf, "PHK"); break;
    case 0x68 : sprintf(buf, "PLA"); break;
    case 0xFA : sprintf(buf, "PLX"); break;
    case 0x7A : sprintf(buf, "PLY"); break;
    case 0xAB : sprintf(buf, "PLB"); break;
    case 0x28 : sprintf(buf, "PLP"); break;
    case 0xEB : sprintf(buf, "XBA"); break;
    case 0x22 : sprintf(buf, "JSL $%02X:%04X", mem_getbyte(pc+1+2, pb), mem_getword(pc+1, pb)); break;
    case 0x20 : sprintf(buf, "JSR $%04X", mem_getword(pc+1, pb)); break;
    case 0x40 : sprintf(buf, "RTI"); break;
    case 0x60 : sprintf(buf, "RTS"); break;
    case 0x6B : sprintf(buf, "RTL"); break;
    case 0x7B : sprintf(buf, "TDC"); break;
    case 0x98 : sprintf(buf, "TYA"); break;
    case 0x9A : sprintf(buf, "TXS"); break;
    case 0xA8 : sprintf(buf, "TAY"); break;
    case 0xAA : sprintf(buf, "TAX"); break;
    case 0xBA : sprintf(buf, "TSX"); break;
    case 0x8A : sprintf(buf, "TXA"); break;
    case 0x38 : sprintf(buf, "SEC"); break;
    case 0x78 : sprintf(buf, "SEI"); break;
    case 0x18 : sprintf(buf, "CLC"); break;
    case 0x58 : sprintf(buf, "CLI"); break;
    case 0xB8 : sprintf(buf, "CLV"); break;
	case 0xD8 : sprintf(buf, "CLD"); break;    
    case 0xFB : sprintf(buf, "XCE"); break;
    case 0x8B : sprintf(buf, "PHB"); break;
    case 0x0B : sprintf(buf, "PHD"); break;
    case 0x2B : sprintf(buf, "PLD"); break;
    case 0x1B : sprintf(buf, "TCS"); break;
    case 0x3B : sprintf(buf, "TSC"); break;
    case 0x5B : sprintf(buf, "TCD"); break;
    case 0xF8 : sprintf(buf, "SED"); break;    
    case 0xF4 : sprintf(buf, "PEA #%04X", mem_getword(pc+1, pb)); break;
    case 0xD4 : sprintf(buf, "PEI $%02X", mem_getbyte(pc+1, pb)); break;
    case 0x4C : sprintf(buf, "JMP $%04X", mem_getword(pc+1, pb)); break;
    case 0x5C : sprintf(buf, "JMP $%02X:%04X", mem_getbyte(pc+1+2,pb), mem_getword(pc+1, pb)); break;
    case 0x6C : sprintf(buf, "JMP ($%04X)", mem_getword(pc+1, pb)); break;
    case 0x7C : sprintf(buf, "JMP ($%04X,X)", mem_getword(pc+1, pb)); break;
    case 0xDC : sprintf(buf, "JML (%X)", mem_getword(pc+1, pb)); break;
    case 0xFC : sprintf(buf, "JSR ($%04X,X)", mem_getword(pc+1, pb)); break;
    case 0x54 : sprintf(buf, "MVN %X:%X->%X:%X %x", mem_getbyte(pc+1+1, pb),
                          CPU.X, mem_getbyte(pc+1, pb), CPU.Y, CPU.A); break;
    case 0x44 : sprintf(buf, "MVP %X:%X->%X:%X %x", mem_getbyte(pc+1+1, pb),
                          CPU.X, mem_getbyte(pc+1, pb), CPU.Y, CPU.A); break;
    case 0x00 : sprintf(buf, "BRK $%04X", CPU.BRK); break;
    case 0x02 : sprintf(buf, "COP $%04X", CPU.COP); break;
	case 0x50 : sprintf(buf, "BVC %04X", pc+1+(sint8)mem_getbyte(pc+1, pb)+1); break;
    case 0xCB : sprintf(buf, "WAI"); break;
    default :   sprintf(buf, "\?\?(%02X)",opcode);
  }
}

#if 0
__attribute__((section(".dtcm")))
uint32	addrbuf[100025];

__attribute__((section(".dtcm")))
uint32	addri = 0;
#endif

int trace_CPU()
{
	sint8	buf[64];
	sint8	buf2[256];

#ifdef ASM_OPCODES
  Cycles = -((sint32)SaveR8 >> 14);
  
  if (Cycles <= 0)
  	return 0;
	
  CPU.packed = 0;	
  CPU_pack();
  P = CPU.P;
#else
  CPU.PC = CPU.LastAddress;
  CPU.A = A;
  CPU.X = X;
  CPU.Y = Y;
//  CPU.Cycles = Cycles;
  CPU.S = S;
  CPU.P = P;
  CPU.D = D;
  CPU.DB = DB;
  CPU.PB = PB;
#endif  

#if 1
  show_opcode(buf, mem_getbyte(CPU.PC, CPU.PB), CPU.PC, CPU.PB, P);
  sprintf(buf2,
			"A:%04X X:%04X Y:%04X S:%04X D:%02X/%04X VC:%03d ?:%08lx/%04x/%04x %d%d%d%d%d%d%d%d %02X:%04X %s\n",
			(unsigned int)CPU.A, (unsigned int)CPU.X, (unsigned int)CPU.Y, (unsigned int)CPU.S, (unsigned int)CPU.DB, 
			(unsigned int)CPU.D, (int)SNES.V_Count,(long unsigned int)Cycles, (unsigned int)HCYCLES, (unsigned int)CPU.HCycles, 
			//&BRKaddress, BRKaddress, CPU.BRK,
			(unsigned int)((P>>7)&1),(unsigned int)((P>>6)&1),(int)((P>>5)&1),(int)((P>>4)&1),
			(int)((P/8)&1),(int)((P/4)&1),(int)((P/2)&1),(int)(P&1),
			(int)CPU.PB,(int)CPU.PC,(const sint8*)buf
		);
  FS_printlog(buf2);
  
/*  sprintf(buf2,
          "r0=%08x r1=%08x r2=%08x r3=%08x r4=%08x r5=%08x r6=%08x r7=%08x r8=%08x r9=%08x r10=%08x r11=%08x\n",
          AsmDebug[0],AsmDebug[1],AsmDebug[2],AsmDebug[3],
          AsmDebug[4],AsmDebug[5],AsmDebug[6],AsmDebug[7],
          AsmDebug[8],AsmDebug[9],AsmDebug[10],AsmDebug[11] );*/
/* sprintf(buf2,
          "r0=%08x r1=%08x r2=%08x r3=%08x / r0=%08x r1=%08x r2=%08x r3=%08x / r0=%08x r1=%08x r2=%08x r3=%08x\n",
          AsmDebug[0],AsmDebug[1],AsmDebug[2],AsmDebug[3],
          AsmDebug[4],AsmDebug[5],AsmDebug[6],AsmDebug[7],
          AsmDebug[8],AsmDebug[9],AsmDebug[10],AsmDebug[11] );
        
  FS_printlog(buf2);*/  
  
//#else
  sprintf(buf2,"%02X:%04X ; ", CPU.PB, CPU.PC);
  printf("%s", buf2);
  //swiDelay(10000000);
#endif          

	if (CPU.PB == 0 && CPU.PC == 0x8F15)
		CFG.CPU_log = 0;
  
  return 0; 
}

void trace_CPUFast()
{
	uint32 PC = ((S&0xFFFF) << 16)|
				(uint32)((sint32)PCptr+(sint32)SnesPCOffset);
	/*if (PC == 0xCD3B92)
		CPU_log = 1; */
	/*if (mem_getbyte(0xA234, 0xB5) == 0)
		CPU_log = 1;*/

	if (PC == 0x008ECE)
		CFG.CPU_log = 1;
	if (PC == 0x008F15)
		CFG.CPU_log = 0;

#if 0
	addrbuf[addri++] = PC;
	if (addri == 100024)
		addri = 1;
#endif		
  /*sint8	buf2[256];	
	sprintf(buf2,"%02X:%04X ",
		S&0xFFFF, 
		(uint32)((sint32)PCptr+(sint32)SnesPCOffset)
		);
	FS_printlogBufOnly(buf2);*/
}