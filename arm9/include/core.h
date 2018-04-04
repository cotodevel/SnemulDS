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

#ifndef __core_h__
#define __core_h__

#include "dsregs.h"
#include "typedefsTGDS.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#ifdef ASM_OPCODES

#define MAP_RELOAD      0x80000000
#define MAP_PPU         0x81000000
#define MAP_CPU         0x82000000
#define MAP_DSP         0x83000000
#define MAP_LOROM_SRAM  0x84000000
#define MAP_HIROM_SRAM  0x85000000
#define MAP_NONE        0x86000000
#define MAP_LAST        0x8F000000
#else

#define MAP_RELOAD      0x00000000
#define MAP_PPU         0x00000001
#define MAP_CPU         0x00000002
#define MAP_DSP         0x00000003
#define MAP_LOROM_SRAM  0x00000004
#define MAP_HIROM_SRAM  0x00000005
#define MAP_NONE        0x00000006
#define MAP_LAST        0x0000000F
#endif



#define bzero(p, s)	memset(p, 0, s)

typedef	struct {
			sint8            title[21];
			uint8   banksize:4;
			uint8   ROMspeed:4;
			uint8   ROMtype;
			uint8   ROMsize;
			uint8   SRAMsize;
			uint8   countrycode;
			uint8   license;
			uint8   version;
			unsigned short  checksum_c;
			unsigned short  checksum;
		}ROM_Info;

struct s_snescore
{
  uint8		*ROM;
  uint8		*RAM;
  uint8		*VRAM;
  uint8		*SRAM;
  uint8		*BSRAM; /* Battery-saved RAM of non-SuperFX ROM */  
  int		SRAMMask;  
};

#ifdef ARM9
// DS->Snes Memory
#define DS_SRAM          ((uint8*)0x0A000000)

#define MAP  ((uint8 **)(0x06898000))
#define WMAP ((uint8 **)(0x0689A000))

//Rom Page variables
#define ROM_MAX_SIZE	(sint32)(2*1024*1024)
#define ROM_STATIC_SIZE	(sint32)(64*1024)
#define ROM_PAGING_SIZE	(sint32)(ROM_MAX_SIZE-ROM_STATIC_SIZE)
#define PAGE_OFFSET		3	//page offset in SNES regs

#endif

//snes irqs
#define IRQ_PENDING_FLAG    (1 << 11)

#define PPU_H_BEAM_IRQ_SOURCE	(1 << 0)
#define PPU_V_BEAM_IRQ_SOURCE	(1 << 1)
#define GSU_IRQ_SOURCE		(1 << 2)
#define SA1_IRQ_SOURCE		(1 << 7)
#define SA1_DMA_IRQ_SOURCE	(1 << 5)

#define SNES_IRQ_SOURCE	    (1 << 7)
#define TIMER_IRQ_SOURCE    (1 << 6)
#define DMA_IRQ_SOURCE	    (1 << 5)

//ppu irq io
//5-4   H/V IRQ (0=Disable, 1=At H=H + V=Any, 2=At V=V + H=0, 3=At H=H + V=V)
#define HV_IRQ_H_V_DISABLED (0x00)
#define HV_IRQ_HH_V_ANY     (0x10)
#define HV_IRQ_VV_H_0       (0x20)
#define HV_IRQ_HH_HV        (0x30)
//7     VBlank NMI Enable  (0=Disable, 1=Enable) (Initially disabled on reset)
#define VBLANK_NMI_IRQENABLE          (0x80)

struct s_snes
{
/* memory mapping */
  uint8    	BlockIsRAM[256*8];
  uint8    	BlockIsROM[256*8];
/*  uint8    	*Map[256*8];
  uint8    	*WriteMap[256*8];*/
  int		HiROM;

  ROM_Info	ROM_info;

/* HDMA */
  uint8		*HDMA_values[256][8];
  uint8		HDMA_nblines[8], HDMA_port[8], HDMA_info[8];
  uint8		HDMA_line;
  int		UsedCycles;

/* MISC */
  int		NTSC;
  uint8		HIRQ_ok;
  uint8		HIRQ_value;
  uint8     PPU_NeedMultiply;
  uint32	JOY_PORT16;
  uint8		Joy1_cnt, Joy1_rdst;	//Joy1_cnt current bit field for DMA_JOYPAD1 
  uint8		Joy2_cnt, Joy2_rdst;	//same for joy2
  
  int		V_Count;
  int		Mode7Repeat;
  int		v_blank; /* screen ray outside of the draw area */
  int		h_blank;
  uint32	DelayedNMI;

  int		ROMSize;
  int		ROMHeader;

  int		mouse_x;
  int		mouse_y;
  int		mouse_b;
  int		mouse_speed;
  int		prev_mouse_x;
  int		prev_mouse_y;
  
  //coto: I removed these because I will use #defines for further readwrites to IO ports..
  //int		joypads[4];	//SNES has 4 joypads after all. 1 player 1, 2 player 2/scope/misc , adapter uses dma port 2 and maps joypads 2,3,4
  
  int		Stopped;
  int		SRAMWritten;  
  
  int		stat_before;
  int		stat_before2;
  int		stat_before3;
  int		stat_before4;
  int		stat_CPU;
  int		stat_GFX;
  int		stat_IOREGS;
  int		stat_DMA;
/*  int		stat_OPC[256];
  int		stat_OPC_cnt[256];*/
};


typedef struct s_Options
{
	uint8 BG3Squish :2;
	uint8 SoundOutput :1;
	uint8 LayersConf :6;
	uint8 TileMode :1;
	uint8 BG_Layer :8;
	uint8 YScroll :2;
	uint8 WaitVBlank :1;
	uint8 SpeedHack :3;
} t_Options;

typedef int (*intfuncptr)();
typedef uint32 (*u32funcptr)();
typedef void (*voidfuncptr)();

//ReadWrite mapped table defs
typedef void (*IOWriteFunc)(uint32 addr, uint32 byte);
typedef uint32 (*IOReadFunc)(uint32 addr);


//masked bits from a joypad port

//Old Style Joypad Registers	$4016	JOYSER0	single (write)	read/write	any time that is not auto-joypad
//Old Style Joypad Registers	$4017	JOYSER1	many (read)	read	any time that is not auto-joypad


//int get_joypad() is the source for these write joypad callbacks. (any DS source)
//the whole emulator will read always from read_joypad1 and read_joypad2 same for write_joypad1 and write_joypad2

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern void fillMemory( void * addr, uint32 count, uint32 value );
extern void zeroMemory( void * addr, uint32 count );

//Snes Hardware
extern struct s_snes	SNES;
extern struct s_cpu	CPU;
extern struct s_snescore	SNESC;
extern struct s_gfx	GFX;
extern struct s_cfg	CFG;
extern volatile uint8 snes_ram_bsram[0x20000+0x6000];    //128K SNES RAM + 8K (Big) SNES SRAM
extern volatile uint8 snes_vram[0x010000];
extern uint8 * rom_page;        //second slot of rombuffer
extern uint8 * rom_buffer;

extern int _offsetY_tab[4];
extern uint32 screen_mode;
extern int APU_MAX;
extern uint32 keys;
extern int	SPC700_emu;

extern int	PPU_fastDMA_2118_1(int offs, int bank, int len);
extern void DMA_transfert(uchar port);
extern void		HDMA_transfert(uint8 port);
extern uint32	IONOP_DMA_READ(uint32 addr);
extern uint32	IONOP_PPU_READ(uint32 addr);
extern void	IONOP_PPU_WRITE(uint32 addr, uint32 byte);
extern void	IONOP_DMA_WRITE(uint32 addr, uint32 byte);
extern void	W4016(uint32 addr, uint32 value);
extern void	W4017(uint32 addr, uint32 value);
extern void	W4200(uint32 addr, uint32 value);
extern void	W4203(uint32 addr, uint32 value);
extern void	W4206(uint32 addr, uint32 value);
extern void	W4207(uint32 addr, uint32 value);
extern void	W4208(uint32 addr, uint32 value);
extern void	W420B(uint32 addr, uint32 value);
extern void	W420C(uint32 addr, uint32 value);
extern uint32	R4016(uint32 addr);
extern uint32	R4017(uint32 addr);
extern uint32	R4210(uint32 addr);
extern uint32	R4211(uint32 addr);
extern uint32	R4212(uint32 addr);
extern uint32	R2121(uint32 addr);
extern uint32	R213X(uint32 addr);
extern uint32	R2137(uint32 addr);
extern uint32	R2138(uint32 addr);
extern uint32	R2139(uint32 addr);
extern uint32	R213A(uint32 addr);
extern uint32	R213B(uint32 addr);
extern uint32	R213C(uint32 addr);
extern uint32	R213D(uint32 addr);
extern uint32	R213F(uint32 addr);
extern uint32	R2140(uint32 addr);
extern uint32	R2141(uint32 addr);
extern uint32	R2142(uint32 addr);
extern uint32	R2143(uint32 addr);
extern uint32	R2180(uint32 addr);
extern void	W2100(uint32 addr, uint32 value);
extern void	W2101(uint32 addr, uint32 value);
extern void	W2102(uint32 addr, uint32 value);
extern void	W2103(uint32 addr, uint32 value);
extern void	W2104(uint32 addr, uint32 value);
extern void	W2105(uint32 addr, uint32 value);
extern void	W2107(uint32 addr, uint32 value);
extern void	W2108(uint32 addr, uint32 value);
extern void	W2109(uint32 addr, uint32 value);
extern void	W210A(uint32 addr, uint32 value);
extern void	W210B(uint32 addr, uint32 value);
extern void	W210C(uint32 addr, uint32 value);
extern void	W210D(uint32 addr, uint32 value);
extern void	W210E(uint32 addr, uint32 value);
extern void	W210F(uint32 addr, uint32 value);
extern void	W2110(uint32 addr, uint32 value);
extern void	W2111(uint32 addr, uint32 value);
extern void	W2112(uint32 addr, uint32 value);
extern void	W2113(uint32 addr, uint32 value);
extern void	W2114(uint32 addr, uint32 value);
extern void	W2115(uint32 addr, uint32 value);
extern void	W2116(uint32 addr, uint32 value);
extern void	W2117(uint32 addr, uint32 value);
extern void	W2118(uint32 addr, uint32 value);
extern void	W2119(uint32 addr, uint32 value);
extern void	W211A(uint32 addr, uint32 value);
extern void	W211B(uint32 addr, uint32 value);
extern void	W211C(uint32 addr, uint32 value);
extern void	W211D(uint32 addr, uint32 value);
extern void	W211E(uint32 addr, uint32 value);
extern void	W211F(uint32 addr, uint32 value);
extern void	W2120(uint32 addr, uint32 value);
extern void	W2121(uint32 addr, uint32 value);
extern void	W2122(uint32 addr, uint32 value);
extern void	W2132(uint32 addr, uint32 value);
extern void	W2133(uint32 addr, uint32 value);
extern void	pseudoSleep(int d);
extern volatile uint32 dummy;
extern void	W2140(uint32 addr, uint32 value);
extern void	W2141(uint32 addr, uint32 value);
extern void	W2142(uint32 addr, uint32 value);
extern void	W2143(uint32 addr, uint32 value);
extern void	W2180(uint32 addr, uint32 value);
extern void	WW210D(uint32 addr, uint32 value);
extern void	WW210E(uint32 addr, uint32 value);
extern void	WW210F(uint32 addr, uint32 value);
extern void	WW2110(uint32 addr, uint32 value);
extern void	WW2111(uint32 addr, uint32 value);
extern void	WW2112(uint32 addr, uint32 value);
extern void	WW2113(uint32 addr, uint32 value);
extern void	WW2114(uint32 addr, uint32 value);
extern void	WW2122(uint32 addr, uint32 value);


extern IOWriteFunc	IOWrite_PPU[0x90];
extern IOWriteFunc	IOWriteWord_PPU[0x90];
extern IOReadFunc	IORead_PPU[0x90];
extern IOWriteFunc	IOWrite_DMA[0x20];
extern IOReadFunc	IORead_DMA[0x20];
extern void	PPU_port_write(uint32 address, uint8 byte);
extern uint8	PPU_port_read(uint32 address);
extern void	DMA_port_write(uint32 address, uint8 byte);
extern uint8	DMA_port_read(uint32 address);
extern void HDMA_write_port(uchar port, uint8 *data);
extern void	HDMA_write();
extern void	read_mouse();
extern void read_scope();
extern void	update_joypads();
extern void SNES_update();
extern void GoNMI();
extern void GoIRQ();
extern void setirq(uint32 irqs_to_set);
extern void clear_irq_source (uint32 M);
extern void CHECK_FOR_IRQ();

extern uint32 snes_ram_address;
extern void CHECK_FOR_IRQ();
extern void clear_irq_source (uint32 M);
extern void setirq(uint32 irqs_to_set);


//snes.c
extern void	init_GFX();
extern void	reset_GFX();
extern void	reset_CPU();
extern void	reset_SNES();
extern int cnt_alphachar(const sint8 str_buf[]);
extern void	UnInterleaveROM();
extern void	load_ROM(sint8 *ROM, int ROM_size);

//input.c
extern int get_joypad();

extern uint16 read_joypad1();
extern uint16 read_joypad2();
extern void write_joypad1(uint16 bits);
extern void write_joypad2(uint16 bits);

#ifdef __cplusplus
}
#endif
