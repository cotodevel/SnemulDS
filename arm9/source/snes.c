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

#include <string.h>
#include <stdlib.h>

#include "apu.h"
#include "gfx.h"
#include "cfg.h"
//#include "superfx.h"
#include "opcodes.h"
#include "core.h"
#include "ppu.h"
#include "memmap.h"
#include "specific_shared.h"

//GSU Memory Map (at SNES Side)
/*
  00-3F:3000-32FF  GSU I/O Ports
  00-3F:6000-7FFF  Mirror of fragment (err, which one(s)?) of Game Pak RAM
  00-3F:8000-FFFF  Game Pak ROM in LoRom mapping (2Mbyte max)
  40-5F:0000-FFFF  Game Pak ROM in HiRom mapping (mirror of above 2Mbyte)
  70-71:0000-FFFF  Game Pak RAM       (128Kbyte max, usually 32K or 64K)
  78-79:0000-FFFF  Additional "Backup" RAM  (128Kbyte max, usually none)
  80-BF:3000-32FF  Mirror of GSU I/O Ports
  80-BF:6000-7FFF  Mirror of fragment (err, which one(s)?) of Game Pak RAM
  80-BF:8000-FFFF  Additional "CPU" ROM LoROM (2Mbyte max, usually none)
  C0-FF:0000-FFFF  Additional "CPU" ROM HiROM (4Mbyte max, usually none)
For HiROM mapping the address bits are shifted, so both LoROM and HiROM are
linear (eg. Bank 40h contains mirrors of Bank 00h and 01h).
Although both LoROM and HiROM are supported, the header & exception vectors are
located at ROM Offset 7Fxxh (in LoROM fashion), accordingly the cartridge
header declares the cartridge as LoROM.
The additional ROM/RAM regions would be mapped to SNES CPU only (not to GSU),
they aren't installed in existing cartridges, that implies that the "Fast" ROM
banks (80h-FFh) are unused, so GSU games are restricted to "Slow" ROM.
The "unspecified" 8K fragment(s) at 6000h-7FFFh are used by Super Mario World
2; whereas, it seems as if ALL 8K fragments do mirror to the FIRST 8K of Game
Pak RAM?
*/

//GSU Memory Map (at GSU Side)
/*
  00-3F:0000-7FFF  Mirror of LoROM at 00-3F:8000-FFFF (for "GETB R15" vectors)
  00-3F:8000-FFFF  Game Pak ROM in LoRom mapping (2Mbyte max)
  40-5F:0000-FFFF  Game Pak ROM in HiRom mapping (mirror of above 2Mbyte)
  70-71:0000-FFFF  Game Pak RAM       (128Kbyte max, usually 32K or 64K)
  PBR:0000-01FF    Code-Cache (when having manually stored opcodes in it)
PBR can be set to both ROM/RAM regions (or cache region), ROMBR only to ROM
region (00h-5Fh), RAMBR only to RAM region (70h-71h).

GSU Interrupt Vectors
The SNES Exception Vectors (at FFE4h-FFFFh) are normally located in Game Pak
ROM. When the GSU is running (with GO=1 and RON=1), ROM isn't mapped to SNES
memory, instead, fixed values are appearing as ROM (depending of the lower 4bit
of the address):
  Any Address     Exception Vectors
  [xxx0h]=0100h   -
  [xxx2h]=0100h   -
  [xxx4h]=0104h   [FFE4h]=0104h  COP Vector in 65C816 mode (COP opcode)
  [xxx6h]=0100h   [FFE6h]=0100h  BRK Vector in 65C816 mode (BRK opcode)
  [xxx8h]=0100h   [FFE8h]=0100h  ABT Vector in 65C816 mode (Not used in SNES)
  [xxxAh]=0108h   [FFEAh]=0108h  NMI Vector in 65C816 mode (Vblank)
  [xxxCh]=0100h   -
  [xxxEh]=010Ch   [FFEEh]=010Ch  IRQ Vector in 65C816 mode (H/V-IRQ & GSU-STOP)
It'd be best to set the Game Pak ROM vectors to the same addresses, otherwise
the vectors would change when the GSU is running (or possibly, the fixed-LSBs
may be mixed-up with ROM-MSBs).

*/
#if 0
struct s_SuperFX	SuperFX;

sint8    _use_lfn(const sint8 *path)
{
  return (1);
}

void	SuperFX_init()
{
    SuperFX.Regs = malloc(768);
    SuperFX.RamBanks = 2;
    SuperFX.Ram = malloc(128*1024);
    SuperFX.RomBanks = (2 * 1024 * 1024) / (32 * 1024);
    SuperFX.Rom = (uint8 *)SNES.ROM;
    SNES.ROMSize = 0;
}

void	reset_SuperFX()
{
  SuperFX.RomBanks = SNES.ROMSize >> 15;
  SuperFX.vFlags = 0;
  SuperFXReset(&SuperFX);
}
#endif

void	init_GFX()
{
  init_render();  
}


void	reset_GFX()
{
  memset(&GFX, 0, sizeof(GFX));

//  bzero(GFX.line_spr, 240*128*sizeof(line_sprite_Info));
/*  bzero(GFX.tiles_def[0], 4096);
  bzero(GFX.tiles_def[1], 2048);
  bzero(GFX.tiles_def[2], 1024);*/

  bzero(GFX.spr_info, 128*(sizeof(sprite_Info)));
  bzero(GFX.spr_info_ext, 32);
  bzero(GFX.SNESPal, 256*2);
  //bzero(GFX.buf_screen.line[0], 256*240);
  GFX.BG_scroll_reg = 0;
  GFX.FS_incr = GFX.OAM_upper_byte = 0;
  GFX.ScreenHeight = 224;
  //bzero(GFX.spr_cnt,240);
  GFX.Sprites_table_dirty = 0;
  GFX.SC_incr = 1;
//  memset(GFX.tiles_ry, 8, 4);
  GFX.nb_frames = 0;
  GFX.YScroll = _offsetY_tab[CFG.YScroll];
  GFX.BG3YScroll = 16;
  
  GFX.tile_slot[0] = 4;          	
  GFX.tile_slot[1] = 4;
  GFX.tile_slot[2] = 4;          	
  GFX.tile_slot[3] = 4;  
    
//  update_palette(0);
}

void	reset_CPU()
{
  CPU.IRQ = mem_getword(0xffee, 0);
  CPU.NMI = mem_getword(0xffea, 0);
  CPU.BRK = mem_getword(0xffe6, 0);
  BRKaddress = CPU.BRK; 
  CPU.COP = mem_getword(0xffe4, 0);
  COPaddress = CPU.COP;
#ifndef ASM_OPCODES  
  PC  = mem_getword(0xfffc, 0);
  D = PB = DB = 0;
  P = P_E | P_M | P_X | P_I;
  A = X = Y = 0;
  S = 0x1ff;
#else
  CPU.PC  = mem_getword(0xfffc, 0);
  CPU.D = CPU.PB = CPU.DB = 0;
  CPU.P = P_E | P_M | P_X | P_I;
  CPU.A = CPU.X = CPU.Y = 0;
  CPU.S = 0x1ff;

  CPU_init();	
  PCptr = map_memory(CPU.PC, CPU.PB);
  SnesPCOffset = -((sint32)mem_getbaseaddress(CPU.PC, CPU.PB));
  //printf("PCptr = %08x\n", PCptr);
#endif  
  CPU.IsBreak = 0;
  CPU.packed = CPU.unpacked = 0;
  
}

void	reset_SNES()
{
#if 0
if (CFG.SuperFX)
	reset_SuperFX();

if (CFG.DSP1)
	reset_DSP1();
#endif

  reset_GFX();

  bzero(SNES.HDMA_values, 8*256*4);
  bzero(SNES.HDMA_nblines, 8);
  bzero(SNES.HDMA_port, 8);
  bzero(SNES.HDMA_info, 8);

  memset(SNESC.RAM, 0xFF, 0x20000);
  bzero(SNESC.VRAM, 0x10000);
    //  bzero(CPU.PPU_PORT, 0x2000*2);
    //  bzero(CPU.DMA_PORT, 0x2000*2);  
  memset(CPU.PPU_PORT, 0, 0x90*2);
  memset(CPU.DMA_PORT, 0, 0x100*2);
  memset(CPU.DMA_PORT+0x100, 0xFF, 0x80*2);
  SNES.JOY_PORT16 = 0;
    
    
    
    //if (!CFG.SuperFX)
    {
		SNESC.SRAMMask = SNES.ROM_info.SRAMsize ?
        ((1 << (SNES.ROM_info.SRAMsize + 3)) * 128) - 1 : 0;
		SNESC.SRAM = SNESC.BSRAM;
		memset(SNESC.SRAM, 0xAA, 0x8000);
    }
    
	//printf("test\n");
	
	APU2->counter = 0;
	//  if (CFG.Sound_output) 
  	APU_nice_reset();
	//printf("test1\n");

	InitMap();
  
	//printf("test2\n");
	//mem_reset_paging(); 
	reset_CPU();
	//printf("test3\n");

	if (!CFG.Timing)
		SNES.NTSC = (SNES.ROM_info.countrycode < 2);
	else
		SNES.NTSC = CFG.Timing-1;
    
	SNES.PPU_NeedMultiply = 0;
	SNES.DelayedNMI = 0;    

	CFG.BG_Layer = 0xd7; //FIXME: BG3 is not used (MODE 0 broken)

	APU2->skipper_cnt1 = 0;  
	APU2->skipper_cnt2 = 0;  
	APU2->skipper_cnt3 = 0;  
	APU2->skipper_cnt4 = 0;
	SNES.V_Count = 0;
  
	//printf("test4\n");
	PPU_reset();  
  
	//printf("test5\n");
  
	SNES.mouse_x = 128;
	SNES.mouse_y = 112;
	SNES.mouse_speed = 0;
  
	SNES.Stopped = 0;
	SNES.SRAMWritten = 0;
}

int cnt_alphachar(const sint8 str_buf[])
{
	int i = 0, cnt = 0;

	for (i=0;i <= 20;i++) {
		if ((str_buf[i] >= 'a' && str_buf[i] <= 'z') || (str_buf[i] >= 'A' && str_buf[i] <= 'Z')){
			cnt++;
		}
	}
return cnt;
}

void	UnInterleaveROM()
{
	int	i, j;
	int	nblocks = SNES.ROMSize >> 15;
	int	step = 64;
	uchar blocks[256], b;
	uchar	*tmp;

	while (nblocks <= step)
		step >>= 1;
	   
	nblocks = step;

	if (CFG.InterleavedROM2)
	{
		for (i = 0; i < nblocks * 2; i++)
			blocks[i] = (i&~0x1e)|((i&2)<<2)|((i&4)<<2)|((i&8)>>2)|((i&16)>>2);
    }
	else
    {
		SNES.HiROM ^= 1;

		for (i = 0; i < nblocks; i++)
		{
			blocks[i*2] = i + nblocks;
			blocks[i*2+1] = i;
        }
	}

	tmp = (uchar*)malloc(0x8000);
	for (i = 0;i < nblocks*2;i++)
    {
		for (j = i;j < nblocks*2;j++)
        {
			if (blocks[j] == i)
			{
				memmove(tmp, &SNESC.ROM[blocks[j]*0x8000], 0x8000);
				memmove(&SNESC.ROM[blocks[j]*0x8000],
				&SNESC.ROM[blocks[i]*0x8000], 0x8000);
				memmove(&SNESC.ROM[blocks[i]*0x8000], tmp, 0x8000);
				b = blocks[j];
				blocks[j] = blocks [i];
				blocks [i] = b;
				break;
			}
		}
    }
	free(tmp);
}

void	load_ROM(sint8 *ROM, int ROM_size)
{
  int       fileheader, filesize;
  int		cnt1, cnt2;
  sint8		jap;
  ROM_Info	LoROM_info;
  ROM_Info	HiROM_info;

  filesize = ROM_size;
  fileheader = filesize & 8191;

  if (fileheader != 0 && fileheader != 512)
    fileheader = 512;

  SNES.ROMHeader = fileheader;
  SNES.ROMSize = filesize-fileheader;
  SNESC.ROM = (uchar *)ROM+fileheader;

  if (CFG.InterleavedROM)
    UnInterleaveROM();

  memcpy(&LoROM_info, SNESC.ROM+0x7FC0, sizeof(ROM_Info));
  memcpy(&HiROM_info, SNESC.ROM+0xFFC0, sizeof(ROM_Info));

// conditions necessaires
  if (filesize < 0x80000 ||
      *(unsigned short *)&SNESC.ROM[0xfffc] == 0xFFFF ||
      *(unsigned short *)&SNESC.ROM[0xfffc] < 0x8000) {
    SNES.HiROM = 0; 
    memcpy(&SNES.ROM_info, (void*)&LoROM_info, sizeof(ROM_Info));
    return;
  }
  if (*(unsigned short *)&SNESC.ROM[0x7ffc] == 0xFFFF ||
      *(unsigned short *)&SNESC.ROM[0x7ffc] < 0x8000) {
    SNES.HiROM = 1; 
    memcpy(&SNES.ROM_info, (void*)&HiROM_info, sizeof(ROM_Info));
    return;
  }

// conditions suffisantes
  if ((LoROM_info.checksum ^ LoROM_info.checksum_c) == 0xFFFF) {
    SNES.HiROM = 0;
    memcpy(&SNES.ROM_info, (void*)&LoROM_info, sizeof(ROM_Info));
    return;
  }
  if ((HiROM_info.checksum ^ HiROM_info.checksum_c) == 0xFFFF) {
    SNES.HiROM = 1; 
    memcpy(&SNES.ROM_info, (void*)&HiROM_info, sizeof(ROM_Info));
    return;
  }

// algorithme de ZoOp
  SNES.HiROM = 0;

  cnt1 = cnt_alphachar(LoROM_info.title);
  cnt2 = cnt_alphachar(HiROM_info.title);

  jap = 1;
  if (cnt1 > 0 || cnt2 > 0)
    {
      if (cnt1 == 0 && cnt2 >  0)
        {
          SNES.HiROM = 1; jap = 0;
        }
      if (cnt1 >  0 && cnt2 == 0)
        {
          SNES.HiROM = 0; jap = 0;
        }
      if (cnt1 == cnt2)
        jap = 0;
      if (SNES.HiROM == 0 && cnt2 > cnt1)
        SNES.HiROM = 1;
      if (SNES.HiROM == 1 && jap && (HiROM_info.banksize != 1))
        SNES.HiROM = 0;
    }

  if (SNES.HiROM == 0) {
    memcpy(&SNES.ROM_info, &LoROM_info, sizeof(ROM_Info));
  } else {
    memcpy(&SNES.ROM_info, &HiROM_info, sizeof(ROM_Info));
  }
}

