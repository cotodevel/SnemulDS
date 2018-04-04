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

// SNEMUL (c) 1997 Nicolas

#ifndef __opcodes_h__
#define __opcodes_h__

#include	"common.h"
#include	"typedefsTGDS.h"

#define P_C  0x01
#define P_Z  0x02
#define P_I  0x04
#define P_D  0x08
#define P_X  0x10
#define P_M  0x20
#define P_V  0x40
#define P_N  0x80

#define P_E  0x100

#endif


#ifdef ASM_OPCODES

	#define REAL_A ((SaveR8 & 0x00000080) ? \
				(A >> 24 | (SnesB&0xFF000000) >> 16) : (A >> 16))
	#define REAL_CYCLES (-((sint32)SaveR8 >> 14))
	#define HCYCLES (CPU.HCycles+CPU.Cycles+((sint32)SaveR8 >> 14))
	#define ADD_CYCLES(x)	(SaveR8 +=((x)<<14))
	//#define FIX_VCOUNT		{ if ((sint32)SaveR8 > 0) { SNES.VCount++; (sint32)SaveR8 -= NB_CYCLES; ) }  
	#define SET_WAITCYCLES(c) {	\
		CPU_WaitAddress = CPU_LoopAddress; \
		CPU_NextCycles = 0; }

	#define SET_WAITCYCLESDELAY(delay) {	\
		CPU_WaitAddress = CPU_LoopAddress; \
		uint32 tmp = NB_CYCLES-delay-CPU.Cycles-CPU.HCycles; \
		if (tmp < 0) CPU_NextCycles = (tmp) << 14; \
		else CPU_NextCycles = 0;	\
	}

#else
	

	#define REAL_A	A
	#define REAL_CYCLES	Cycles
	#define HCYCLES	(CPU.HCycles+Cycles)
	#define ADD_CYCLES(x)	(Cycles +=(x))
	#define SET_WAITCYCLES(c) { CPU.WaitAddress = CPU.LastAddress; \
								CPU.WaitCycles = c; }
	#define SET_WAITCYCLESDELAY(delay) { CPU.WaitAddress = CPU.LastAddress; \
										 CPU.WaitCycles = CPU.Cycles-(delay); }

#endif

//CPU Hardware
#ifndef __opcodes_cpu_snemul__
#define __opcodes_cpu_snemul__

#include "common.h"

#define NB_CYCLES 180

struct s_cpu
{
    uint16	IRQ, NMI, BRK, COP; /* interruption address */
    int		cycles_tot;
    int		NMIActive;
    uchar		WAI_state;

    /* debug */
    int		Trace_flag;
    int		Trace;
    int		Cycles2;

/* registers */
#define P_C  0x01
#define P_Z  0x02
#define P_I  0x04
#define P_D  0x08
#define P_X  0x10
#define P_M  0x20
#define P_V  0x40
#define P_N  0x80
#define P_E  0x100
    uint16        P; /* Flags Register */
    uint16        PC; /* Program Counter */
    uint16        PB, DB; /* Bank Registers */
    uint16        A, X, Y, D, S;

    int           Cycles;

#define IRQ_GSU	1
    int		IRQState;

    /* speed hack */
    int           LastAddress;
    int           WaitAddress;
    int           WaitCycles;
    uint32		HCycles;

    int 			IsBreak;

    int			unpacked;
    int			packed;

    uint16	PPU_PORT[0x90]; // 2100 -> 2183
    uint16	DMA_PORT[0x180]; // 4200 -> 437F

    //coto:new
    uint16      cpuflags;   //new: tells wether its time to raise or serve an interrupt
    uint16      irqactive;  //new: IRQs currently active
    int SavedCycles;
    uint32 SavedIRQState;
    int initialPC;          //for co processor operation
};

#endif

#ifdef __cplusplus
extern "C" {
#endif



//todo: check later if SNES CPU Core (non assembler snezzi core) compiles and work

//Snezzi Assembler Snes Core
#ifdef ASM_OPCODES

extern unsigned short P;
extern unsigned short PC;
extern uint8  PB, DB, t;
extern unsigned int A, X, Y, D, S;
extern long Cycles;
extern uint8	*PCptr;

extern unsigned int		SnesPCOffset;
extern unsigned int		SaveR6;
extern unsigned int		SaveR8;

extern unsigned int		SnesB;

extern	uint32			CPU_log;
extern	uint32			AsmDebug[16];

extern	sint32			CPU_NextCycles;
extern	uint32			CPU_LoopSpeedHacks;
extern	uint8	*CPU_WaitAddress;
extern	uint8	*CPU_LoopAddress;

extern	uint32			BRKaddress;
extern	uint32			COPaddress;

extern void CPU_pack();
extern void CPU_unpack();
extern void	pushb(uint8 b);
extern void pushw(uint16 w);
extern uint8	pullb();
extern uint16	pullw();
extern void CPU_goto(int cycles);


	
//SnemulDS high level core (slower)
#else

extern 	uint32 PCptr;
extern 	uint32 SnesPCOffset;
extern	uint32	BRKaddress;
extern	uint32	COPaddress;

extern unsigned short P;
extern unsigned short PC;
extern uint8  PB, DB, t;
extern unsigned short A, X, Y, D, S;
extern long Cycles;

extern uint8 OpCycles_MX[256];
extern uint8 OpCycles_mX[256];
extern uint8 OpCycles_Mx[256];
extern uint8 OpCycles_mx[256];
extern uint32	F_C;
extern uint32	F_Z;
extern uint32	F_N;
extern uint32	F_V;

extern void		pushb(uint8 b);
extern void		pushw(uint16 w);
extern uint8	pullb();
extern uint16	pullw();
extern uchar   stack_getbyte(uint8 offset);
extern void	stack_setbyte(uint8 offset, uchar byte);
extern ushort  stack_getword(uint8 offset);
extern void  stack_setword(uint8 offset, uint16 word);
extern uchar   direct_getbyte(uint32 offset);
extern uchar   direct_getbyte2(uint32 offset);
extern void	direct_setbyte(uint32 offset, uchar byte);
extern ushort  direct_getword(uint32 offset);
extern void  direct_setword(uint32 offset, uint16 word);
extern uint8 rol_b(uint8 a);
extern uint16 rol_w(uint16 a);
extern uint8 ror_b(uint8 a);
extern uint16 ror_w(uint16 a);
extern uint8			*OpCycles;
extern void ADC16(uint16 Work16);
extern void ADC8(uint8 Work8);
extern void SBC16(uint16 Work16);
extern void SBC8(uint8 Work8);
extern void	RTI();
extern void	XCE();
extern void	MVN(uint8 SB);
extern void	MVP(uint8 SB);
extern void	BRK();
extern void	COP();
extern void CPU_goto(int cycles);
extern void do_branch();
extern void CPU_pack();
extern void CPU_unpack();
#endif


//opcodes2.s
extern void 	CPU_goto2(int CyclesToCrunch);
extern void 	CPU_update();
extern void		CPU_init();	
#ifdef __cplusplus
}
#endif
