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

#include "common.h"
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "opcodes.h"
#include "cfg.h"
#include "apu.h"
#include "core.h"
#include "memmap.h"
//
#ifdef WIN32
#define OPCODE _inline
#else
#define OPCODE static inline
#endif

//Non Assembler snezzi core 
#ifndef ASM_OPCODES

uint32 PCptr=0;
uint32 SnesPCOffset=0;
uint32	BRKaddress = 0;
uint32	COPaddress = 0;
	
__attribute__((section(".dtcm")))
uint8 OpCycles_MX[256] = {
	8, 6, 8, 4, 5, 3, 5, 6, 3, 2, 2, 4, 6, 4, 6, 5,		/* e=0, m=1, x=1 */
	2, 5, 5, 7, 5, 4, 6, 6, 2, 4, 2, 2, 6, 4, 7, 5,
	6, 6, 8, 4, 3, 3, 5, 6, 4, 2, 2, 5, 4, 4, 6, 5,
	2, 5, 5, 7, 4, 4, 6, 6, 2, 4, 2, 2, 4, 4, 7, 5,
	7, 6, 2, 4, 0, 3, 5, 6, 3, 2, 2, 3, 3, 4, 6, 5,
	2, 5, 5, 7, 0, 4, 6, 6, 2, 4, 3, 2, 4, 4, 7, 5,
	6, 6, 6, 4, 3, 3, 5, 6, 4, 2, 2, 6, 5, 4, 6, 5,
	2, 5, 5, 7, 4, 4, 6, 6, 2, 4, 4, 2, 6, 4, 7, 5,
	2, 6, 3, 4, 3, 3, 3, 6, 2, 2, 2, 3, 4, 4, 4, 5,
	2, 5, 5, 7, 4, 4, 4, 6, 2, 4, 2, 2, 4, 4, 4, 5,
	2, 6, 2, 4, 3, 3, 3, 6, 2, 2, 2, 4, 4, 4, 4, 5,
	2, 5, 5, 7, 4, 4, 4, 6, 2, 4, 2, 2, 4, 4, 4, 5,
	2, 6, 3, 4, 3, 3, 5, 6, 2, 2, 2, 3, 4, 4, 6, 5,
	2, 5, 5, 7, 6, 4, 6, 6, 2, 4, 3, 3, 6, 4, 7, 5,
	2, 6, 3, 4, 3, 3, 5, 6, 2, 2, 2, 3, 4, 4, 6, 5,
	2, 5, 5, 7, 5, 4, 6, 6, 2, 4, 4, 2, 6, 4, 7, 5
};

__attribute__((section(".dtcm")))
uint8 OpCycles_mX[256] = {
	8, 7, 8, 5, 7, 4, 7, 7, 3, 3, 2, 4, 8, 5, 8, 6,		/* e=0, m=0, x=1 */
	2, 6, 6, 8, 7, 5, 8, 7, 2, 5, 2, 2, 8, 5, 9, 6,
	6, 7, 8, 5, 4, 4, 7, 7, 4, 3, 2, 5, 5, 5, 8, 6,
	2, 6, 6, 8, 5, 5, 8, 7, 2, 5, 2, 2, 5, 5, 9, 6,
	7, 7, 2, 5, 0, 4, 7, 7, 4, 3, 2, 3, 3, 5, 8, 6,
	2, 6, 6, 8, 0, 5, 8, 7, 2, 5, 3, 2, 4, 5, 9, 6,
	6, 7, 6, 5, 4, 4, 7, 7, 5, 3, 2, 6, 5, 5, 8, 6,
	2, 6, 6, 8, 5, 5, 8, 7, 2, 5, 4, 2, 6, 5, 9, 6,
	2, 7, 3, 5, 3, 4, 3, 7, 2, 3, 2, 3, 4, 5, 4, 6,
	2, 6, 6, 8, 4, 5, 4, 7, 2, 5, 2, 2, 5, 5, 5, 6,
	2, 7, 2, 5, 3, 4, 3, 7, 2, 3, 2, 4, 4, 5, 4, 6,
	2, 6, 6, 8, 4, 5, 4, 7, 2, 5, 2, 2, 4, 5, 4, 6,
	2, 7, 3, 5, 3, 4, 7, 7, 2, 3, 2, 3, 4, 5, 8, 6,
	2, 6, 6, 8, 6, 5, 8, 7, 2, 5, 3, 3, 6, 5, 9, 6,
	2, 7, 3, 5, 3, 4, 7, 7, 2, 3, 2, 3, 4, 5, 8, 6,
	2, 6, 6, 8, 5, 5, 8, 7, 2, 5, 4, 2, 6, 5, 9, 6
};

__attribute__((section(".dtcm")))
uint8 OpCycles_Mx[256] = {
	8, 6, 8, 4, 5, 3, 5, 6, 3, 2, 2, 4, 6, 4, 6, 5,		/* e=0, m=1, x=0 */
	2, 6, 5, 7, 5, 4, 6, 6, 2, 5, 2, 2, 6, 5, 7, 5,
	6, 6, 8, 4, 3, 3, 5, 6, 4, 2, 2, 5, 4, 4, 6, 5,
	2, 6, 5, 7, 4, 4, 6, 6, 2, 5, 2, 2, 5, 5, 7, 5,
	7, 6, 2, 4, 0, 3, 5, 6, 4, 2, 2, 3, 3, 4, 6, 5,
	2, 6, 5, 7, 0, 4, 6, 6, 2, 5, 4, 2, 4, 5, 7, 5,
	6, 6, 6, 4, 3, 3, 5, 6, 5, 2, 2, 6, 5, 4, 6, 5,
	2, 6, 5, 7, 4, 4, 6, 6, 2, 5, 5, 2, 6, 5, 7, 5,
	2, 6, 3, 4, 4, 3, 4, 6, 2, 2, 2, 3, 5, 4, 5, 5,
	2, 6, 5, 7, 5, 4, 5, 6, 2, 5, 2, 2, 4, 5, 5, 5,
	3, 6, 3, 4, 4, 3, 4, 6, 2, 2, 2, 4, 5, 4, 5, 5,
	2, 6, 5, 7, 5, 4, 5, 6, 2, 5, 2, 2, 5, 5, 5, 5,
	3, 6, 3, 4, 4, 3, 6, 6, 2, 2, 2, 3, 5, 4, 6, 5,
	2, 6, 5, 7, 6, 4, 8, 6, 2, 5, 4, 3, 6, 5, 7, 5,
	3, 6, 3, 4, 4, 3, 6, 6, 2, 2, 2, 3, 5, 4, 6, 5,
	2, 6, 5, 7, 5, 4, 8, 6, 2, 5, 5, 2, 6, 5, 7, 5
};

__attribute__((section(".dtcm"))) 
uint8 OpCycles_mx[256] = {
	8, 7, 8, 5, 7, 4, 7, 7, 3, 3, 2, 4, 8, 5, 8, 6,		/* e=0, m=0, x=0 */
	2, 7, 6, 8, 7, 5, 8, 7, 2, 6, 2, 2, 8, 6, 9, 6,
	6, 7, 8, 5, 4, 4, 7, 7, 4, 3, 2, 5, 5, 5, 8, 6,
	2, 7, 6, 8, 5, 5, 8, 7, 2, 6, 2, 2, 6, 6, 9, 6,
	7, 7, 2, 5, 0, 4, 7, 7, 3, 3, 2, 3, 3, 5, 8, 6,
	2, 7, 6, 8, 0, 5, 8, 7, 2, 6, 4, 2, 4, 6, 9, 6,
	6, 7, 6, 5, 4, 4, 7, 7, 4, 3, 2, 6, 5, 5, 8, 6,
	2, 7, 6, 8, 5, 5, 8, 7, 2, 6, 5, 2, 6, 6, 9, 6,
	2, 7, 3, 5, 4, 4, 4, 7, 2, 3, 2, 3, 5, 5, 5, 6,
	2, 7, 6, 8, 5, 5, 5, 7, 2, 6, 2, 2, 5, 6, 6, 6,
	3, 7, 3, 5, 4, 4, 4, 7, 2, 3, 2, 4, 5, 5, 5, 6,
	2, 7, 6, 8, 5, 5, 5, 7, 2, 6, 2, 2, 5, 6, 5, 6,
	3, 7, 3, 5, 4, 4, 7, 7, 2, 3, 2, 3, 5, 5, 8, 6,
	2, 7, 6, 8, 6, 5, 8, 7, 2, 6, 4, 3, 6, 6, 9, 6,
	3, 7, 3, 5, 4, 4, 7, 7, 2, 3, 2, 3, 5, 5, 8, 6,
	2, 7, 6, 8, 5, 5, 8, 7, 2, 6, 5, 2, 6, 6, 9, 6
};


__attribute__((section(".dtcm")))
uint32	F_C;
__attribute__((section(".dtcm")))
uint32	F_Z;
__attribute__((section(".dtcm")))
uint32	F_N;
__attribute__((section(".dtcm")))
uint32	F_V;

//todo: move this to headers and check compatibility.
#define UPDATE_FLAGS \
{ \
	F_C = (P & P_C); \
	F_Z = !(P & P_Z); \
	F_N = (P & P_N); \
	F_V = (P & P_V) == P_V; \
	if (!(P&P_M) && !(P&P_X)) \
	  OpCycles = OpCycles_mx; \
	if (!(P&P_M) && (P&P_X)) \
	  OpCycles = OpCycles_mX; \
	if ((P&P_M) && !(P&P_X)) \
	  OpCycles = OpCycles_Mx; \
	if ((P&P_M) && (P&P_X)) \
	  OpCycles = OpCycles_MX; \
}

#define UPDATE_P \
{ \
	P &= ~(P_C|P_Z|P_N|P_V); \
	P |= F_C; \
	P |= (F_Z == 0) << 1; \
	P |= F_N & 0x80; \
	P |= (F_V != 0) << 6; \
}

__attribute__((section(".dtcm")))
unsigned short P;
__attribute__((section(".dtcm")))
unsigned short PC;
__attribute__((section(".dtcm")))
uint8  PB, DB, t;
__attribute__((section(".dtcm")))
unsigned short A, X, Y, D, S;
__attribute__((section(".dtcm")))
long Cycles;


void	pushb(uint8 b)
{
	SNESC.RAM[S] = b;
	S--;
}

void pushw(uint16 w)
{
	S--;
	SET_WORD16(SNESC.RAM+S, w);
	S--;
}

uint8	pullb()
{
	S++;
	return SNESC.RAM[S];
}

uint16	pullw()
{
	uint16 w;
	
	S++;	
	w = GET_WORD16(SNESC.RAM+S);
	S++;
	return w;
}

uchar   stack_getbyte(uint8 offset)
{
  return SNESC.RAM[S+offset];
}

void	stack_setbyte(uint8 offset, uchar byte)
{
  SNESC.RAM[S+offset] = byte;
}

ushort  stack_getword(uint8 offset)
{
  return GET_WORD16(SNESC.RAM+S+offset);	
}

void  stack_setword(uint8 offset, uint16 word)
{
  SET_WORD16(SNESC.RAM+S+offset, word);
}

uchar   direct_getbyte(uint32 offset)
{
  uint16 _offset = (uint16)(D+offset);	 
  if (_offset < 0x2000)
  {	
  	return SNESC.RAM[_offset];
  }
  else
  	return mem_getbyte(_offset, 0);	
}

uchar   direct_getbyte2(uint32 offset)
{
  uint16 _offset = (uint16)(D+offset);	
  if (_offset+2 < 0x2000)
  {
  	return SNESC.RAM[_offset+2];
  }
  else
  	return mem_getbyte(_offset+2, 0);	
}

void	direct_setbyte(uint32 offset, uchar byte)
{
  uint16 _offset = (uint16)(D+offset);	
  if (_offset < 0x2000)
  	SNESC.RAM[_offset] = byte;
  else
  	mem_setbyte(_offset, 0, byte);	
}

ushort  direct_getword(uint32 offset)
{
  uint16 _offset = (uint16)(D+offset);	
  if (_offset < 0x2000)
  {
  	return GET_WORD16(SNESC.RAM+_offset);
  }
  else
  	return mem_getword(_offset, 0);	
}

void  direct_setword(uint32 offset, uint16 word)
{
  uint16 _offset = (uint16)(D+offset);
  if (_offset < 0x2000)
  	SET_WORD16(SNESC.RAM+_offset, word)
  else
  	mem_setword(_offset, 0, word);	
}

uint8 rol_b(uint8 a)
{
	uint16 t = a;
	t <<= 1;
	t |= F_C;
	F_C = (t >= 0x100);
	return (uint8)t;
}

uint16 rol_w(uint16 a)
{
	uint32 t = a;
	t <<= 1;
	t |= F_C;
	F_C = (t >= 0x10000);
	return (uint16)t;
}

uint8 ror_b(uint8 a)
{
	uint16 t = a;
	t |= F_C << 8;
	F_C = (t & 1);
	t >>= 1;		
	return (uint8)t;
}

uint16 ror_w(uint16 a)
{
	uint32 t = a;
	t |= F_C << 16;
	F_C = (t & 1);
	t >>= 1;
	return (uint16)t;
}

//todo: move to headers

// addressing mode A ( rA )
#define GETBYTE_rA       (rA&0xFF)
#define GETWORD_rA       rA
#define SETBYTE_rA(var)  rA = (rA&0xff00)|((var)&0xff)
#define SETWORD_rA(var)  rA = (var)
#define UPDATEPC_rA      

// addressing mode i ( i )

#define GETBYTE_i       FETCHB
#define GETWORD_i       FETCHW
#define SETBYTE_i(var)  *PCptr = var
#define SETWORD_i(var)  SET_WORD16(PCptr, var)
#define GETBYTE_iX       FETCHB
#define GETWORD_iX       FETCHW
#define SETBYTE_iX(var)  *PCptr = var
#define SETWORD_iX(var)  SET_WORD16(PCptr, var)


#define UPDATEPC_i      if (P&P_M) PCptr++; else PCptr+=2;
#define UPDATEPC_iX      if (P&P_X) PCptr++; else PCptr+=2;


/*
#define direct_getbyte(x) mem_getbyte((D+(x))&0xFFFF, 0)
#define direct_getbyte2(x) mem_getbyte(((D+(x))&0xFFFF)+2, 0)
#define direct_setbyte(x, var) mem_setbyte((D+(x))&0xFFFF, 0, var)
#define direct_getword(x) mem_getword((D+(x))&0xFFFF, 0)
#define direct_setword(x, var) mem_setword((D+(x))&0xFFFF, 0, var)
*/

// addressing mode d ( d )

#define GETBYTE_d       direct_getbyte(FETCHB)
#define GETWORD_d       direct_getword(FETCHB)
#define SETBYTE_d(var)  direct_setbyte(FETCHB, var)
#define SETWORD_d(var)  direct_setword(FETCHB, var)
#define UPDATEPC_d      PCptr++;

// addressing mode d,x ( d_x )

#define GETBYTE_d_x       direct_getbyte(FETCHB+X)
#define GETWORD_d_x       direct_getword(FETCHB+X)
#define SETBYTE_d_x(var)  direct_setbyte(FETCHB+X, var)
#define SETWORD_d_x(var)  direct_setword(FETCHB+X, var)
#define UPDATEPC_d_x      PCptr++;

// addressing mode d,y ( d_y )

#define GETBYTE_d_y       direct_getbyte(FETCHB+Y)
#define GETWORD_d_y       direct_getword(FETCHB+Y)
#define SETBYTE_d_y(var)  direct_setbyte(FETCHB+Y, var)
#define SETWORD_d_y(var)  direct_setword(FETCHB+Y, var)
#define UPDATEPC_d_y      PCptr++;

// addressing mode d,s ( d_s )
// direct read to S

#define GETBYTE_d_s       stack_getbyte(FETCHB)
#define GETWORD_d_s       stack_getword(FETCHB)
#define SETBYTE_d_s(var)  stack_setbyte(FETCHB, var)
#define SETWORD_d_s(var)  stack_setword(FETCHB, var)
#define UPDATEPC_d_s      PCptr++;

// addressing mode a ( a )

#define GETBYTE_a       mem_getbyte(FETCHW, DB)
#define GETWORD_a       mem_getword(FETCHW, DB)
#define SETBYTE_a(var)  mem_setbyte(FETCHW, DB, (var))
#define SETWORD_a(var)  mem_setword(FETCHW, DB, (var))
#define UPDATEPC_a      PCptr+=2;

// addressing mode a,x ( a_x )

#define GETBYTE_a_x       mem_getbyte(FETCHW+X, DB)
#define GETWORD_a_x       mem_getword(FETCHW+X, DB)
#define SETBYTE_a_x(var)  mem_setbyte(FETCHW+X, DB, (var))
#define SETWORD_a_x(var)  mem_setword(FETCHW+X, DB, (var))
#define UPDATEPC_a_x      PCptr+=2;

// addressing mode a,y ( a_y )

#define GETBYTE_a_y       mem_getbyte(FETCHW+Y, DB)
#define GETWORD_a_y       mem_getword(FETCHW+Y, DB)
#define SETBYTE_a_y(var)  mem_setbyte(FETCHW+Y, DB, (var))
#define SETWORD_a_y(var)  mem_setword(FETCHW+Y, DB, (var))
#define UPDATEPC_a_y      PCptr+=2;

// addressing mode al ( al )

#define GETBYTE_al       mem_getbyte(FETCHW, FETCHB2)
#define GETWORD_al       mem_getword(FETCHW, FETCHB2)
#define SETBYTE_al(var)  mem_setbyte(FETCHW, FETCHB2, (var))
#define SETWORD_al(var)  mem_setword(FETCHW, FETCHB2, (var))
#define UPDATEPC_al      PCptr+=3;

// addressing mode al,x ( al_x )

#define GETBYTE_al_x       mem_getbyte(FETCHW+X, FETCHB2)
#define GETWORD_al_x       mem_getword(FETCHW+X, FETCHB2)
#define SETBYTE_al_x(var)  mem_setbyte(FETCHW+X, FETCHB2, (var))
#define SETWORD_al_x(var)  mem_setword(FETCHW+X, FETCHB2, (var))
#define UPDATEPC_al_x      PCptr+=3;

// addressing mode [d] ( d_i_l )

#define GETBYTE_d_i_l \
  mem_getbyte(direct_getword(FETCHB), direct_getbyte2(FETCHB))
#define GETWORD_d_i_l \
  mem_getword(direct_getword(FETCHB), direct_getbyte2(FETCHB))
#define SETBYTE_d_i_l(var) \
  mem_setbyte(direct_getword(FETCHB), direct_getbyte2(FETCHB), var)
#define SETWORD_d_i_l(var) \
  mem_setword(direct_getword(FETCHB), direct_getbyte2(FETCHB), var)
#define UPDATEPC_d_i_l      PCptr++;              

// addressing mode [d],y ( d_i_l_y )

#define GETBYTE_d_i_l_y \
  mem_getbyte(direct_getword(FETCHB)+Y, direct_getbyte2(FETCHB))
#define GETWORD_d_i_l_y \
  mem_getword(direct_getword(FETCHB)+Y, direct_getbyte2(FETCHB))
#define SETBYTE_d_i_l_y(var) \
  mem_setbyte(direct_getword(FETCHB)+Y, direct_getbyte2(FETCHB), var)
#define SETWORD_d_i_l_y(var) \
  mem_setword(direct_getword(FETCHB)+Y, direct_getbyte2(FETCHB), var)
#define UPDATEPC_d_i_l_y      PCptr++;              

// addressing mode (d,x) ( d_x_i )

#define GETBYTE_d_x_i		mem_getbyte(direct_getword(FETCHB+X), DB)
#define GETWORD_d_x_i		mem_getword(direct_getword(FETCHB+X), DB)
#define SETBYTE_d_x_i(var)  mem_setbyte(direct_getword(FETCHB+X), DB, var)
#define SETWORD_d_x_i(var)  mem_setword(direct_getword(FETCHB+X), DB, var)
#define UPDATEPC_d_x_i      PCptr++;              

// addressing mode (d,s),y ( d_s_i_y )

#define GETBYTE_d_s_i_y		  mem_getbyte(stack_getword(FETCHB)+Y, DB)
#define GETWORD_d_s_i_y		  mem_getword(stack_getword(FETCHB)+Y, DB)
#define SETBYTE_d_s_i_y(var)  mem_setbyte(stack_getword(FETCHB)+Y, DB, var)
#define SETWORD_d_s_i_y(var)  mem_setword(stack_getword(FETCHB)+Y, DB, var)
#define UPDATEPC_d_s_i_y      PCptr++;                

// addressing mode (d),y ( d_i_y )

#define GETBYTE_d_i_y		mem_getbyte(direct_getword(FETCHB)+Y, DB)
#define GETWORD_d_i_y		mem_getword(direct_getword(FETCHB)+Y, DB)
#define SETBYTE_d_i_y(var)  mem_setbyte(direct_getword(FETCHB)+Y, DB, var)
#define SETWORD_d_i_y(var)  mem_setword(direct_getword(FETCHB)+Y, DB, var)
#define UPDATEPC_d_i_y      PCptr++;                

// addressing mode (d) ( d_i )

#define GETBYTE_d_i		mem_getbyte(direct_getword(FETCHB), DB)
#define GETWORD_d_i		mem_getword(direct_getword(FETCHB), DB)
#define SETBYTE_d_i(var)  mem_setbyte(direct_getword(FETCHB), DB, var)
#define SETWORD_d_i(var)  mem_setword(direct_getword(FETCHB), DB, var)
#define UPDATEPC_d_i      PCptr++;              

#define TESTB_3(var, value) \
{ \
  sint16 tmp; \
  tmp = (sint16)((var)&0xFF) - (sint16)(value); \
  F_C = (tmp >= 0); \
  F_N = (uint8)tmp; \
  F_Z = tmp; \
}

#define TESTW_3(var, value) \
{ \
  sint32 tmp; \
  tmp = (sint32)(var) - (sint32)(value); \
  F_C = (tmp >= 0); \
  F_N = (uint8)(tmp >> 8); \
  F_Z = tmp; \
}
#define TESTB_2(var) \
  F_Z = (uint8)(var); \
  F_N = (uint8)(var);

#define TESTW_2(var) \
  F_Z = (uint16)(var); \
  F_N = (uint8)((var) >> 8);

#define TESTB_1(var) \
  F_Z = (uint8)(var)

#define TESTW_1(var) \
  F_Z = (uint16)(var);

#define FETCHB (*PCptr)
#define FETCHBA (*PCptr++)
#define FETCHW (GET_WORD16(PCptr))
#define FETCHB2 (*(PCptr+2))
#define FETCHB1 (*(PCptr+1))

#define UPDATE_PC { PCptr = map_memory(PC, PB); PC_base = PCptr - PC; }
#define UPDATE_SP SPptr = SNESC.RAM+S; 

#define REAL_PC ((uint32)(PCptr-PC_base))

#define T(x) asm("# X " x);
#define OPBEGIN(n) n: T(#n) {
#define OPEND(x) /*Cycles += (x);*/ goto loop_back; }
#define rA A

__attribute__((section(".dtcm")))
uint8			*OpCycles;

void ADC16(uint16 Work16)
{
	uint8 A1, A2, A3, A4;
	uint8 W1, W2, W3, W4;
	uint16 Ans16;

	if (P & P_D)
	{
		A1 = rA & 0xF;
		A2 = (rA >> 4) & 0xF;
		A3 = (rA >> 8) & 0xF;
		A4 = (rA >> 12) & 0xF;
		W1 = Work16 & 0xF;
		W2 = (Work16 >> 4) & 0xF;
		W3 = (Work16 >> 8) & 0xF;
		W4 = (Work16 >> 12) & 0xF;
		A1 += W1 + F_C;
		A2 += W2;
		A3 += W3;
		A4 += W4;
		if (A1 > 9) { A1 -= 10; A1 &= 0xF; A2++; }
		if (A2 > 9) { A2 -= 10; A2 &= 0xF; A3++; }
		if (A3 > 9) { A3 -= 10; A3 &= 0xF; A4++; }
		if (A4 > 9) { A4 -= 10; A4 &= 0xF; F_C = 1; } else { F_C = 0; }
		Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
		if (~(rA ^ Work16) & (Work16 ^ Ans16) & 0x8000) 
			F_V = 1; else F_V = 0;
		rA = Ans16;
		TESTW_2(rA);
	}
	else
	{
		sint32 tmp;
		tmp = rA + Work16 + F_C;
		F_C = (tmp >= 0x10000);
		if (~(rA ^ Work16) & (Work16 ^ (uint16) tmp) & 0x8000) 
			F_V = 1; else F_V = 0;
		rA = (uint16) tmp;
		TESTW_2(rA);
	}
}

void ADC8(uint8 Work8)
{
	uint8 A1, A2, W1, W2;
	uint8 Ans8;

	if (P&P_D)
	{
		A1 = rA & 0xF;
		A2 = (rA >> 4) & 0xF;
		W1 = Work8 & 0xF;
		W2 = (Work8 >> 4) & 0xF;
		A1 += W1 + F_C;
		A2 += W2;
		if (A1 > 9) { A1 -= 10; A1 &= 0xF; A2++; }
		if (A2 > 9) { A2 -= 10; A2 &= 0xF;	F_C = 1; } else { F_C = 0; }
		Ans8 = (A2 << 4) | A1;
		if (~((rA&0xFF) ^ Work8) & (Work8 ^ Ans8) & 0x80) 
			F_V = 1; else F_V = 0;
		rA = (rA&0xFF00) | Ans8;
		TESTB_2(rA);
	}
	else
	{
		sint16 tmp;
		tmp = (rA&0xFF) + Work8 + F_C;
		F_C = (tmp >= 0x100);
		if (~((rA & 0xFF) ^ Work8) & (Work8 ^ (uint8) tmp) & 0x80)
			F_V = 1; else F_V = 0;
		rA = (rA&0xFF00) | (uint8)tmp;
		TESTB_2(rA);
	}
}

void SBC16(uint16 Work16)
{
	uint8 A1, A2, A3, A4;
	uint8 W1, W2, W3, W4;
	uint16 Ans16;

	if (P & P_D)
	{
		A1 = rA & 0xF;
		A2 = (rA >> 4) & 0xF;
		A3 = (rA >> 8) & 0xF;
		A4 = (rA >> 12) & 0xF;
		W1 = Work16 & 0xF;
		W2 = (Work16 >> 4) & 0xF;
		W3 = (Work16 >> 8) & 0xF;
		W4 = (Work16 >> 12) & 0xF;
		A1 -= W1 + !F_C;
		A2 -= W2;
		A3 -= W3;
		A4 -= W4;
		if (A1 > 9) { A1 += 10;  A2--; }
		if (A2 > 9) { A2 += 10;  A3--; }
		if (A3 > 9) { A3 += 10;  A4--; }
		if (A4 > 9) { A4 += 10; F_C = 0; } else { F_C = 1; }
		Ans16 = (A4 << 12) | (A3 << 8) | (A2 << 4) | (A1);
		if ((rA ^ Work16) & (rA ^ Ans16) & 0x8000) 
			F_V = 1; else F_V = 0;
		rA = Ans16;
		TESTW_2(rA);
	}
	else
	{
		sint32 tmp;
		tmp = (sint32)rA - (sint32) Work16 + (sint32)F_C - 1;
		F_C = (tmp >= 0);
		if ((rA ^ Work16) & (rA ^ (uint16) tmp) & 0x8000)
			F_V = 1; else F_V = 0;
		rA = (uint16) tmp;
		TESTW_2(rA);
	}
}

void SBC8(uint8 Work8)
{
	uint8 A1, A2, W1, W2;
	uint8 Ans8;

	if (P&P_D)
	{
		A1 = rA & 0xF;
		A2 = (rA >> 4) & 0xF;
		W1 = Work8 & 0xF;
		W2 = (Work8 >> 4) & 0xF;
		A1 -= W1 + !F_C;
		A2 -= W2;
		if (A1 > 9) { A1 += 10; A2--; }
		if (A2 > 9) { A2 += 10; F_C = 0; } else { F_C = 1; }
		Ans8 = (A2 << 4) | A1;
		if (((rA&0xFF) ^ Work8) & ((rA&0xFF) ^ Ans8) & 0x80) 
			F_V = 1; else F_V = 0;
		rA = (rA&0xFF00) | Ans8;
		TESTB_2(rA);
	}
	else
	{
		sint16 tmp;
		tmp = (sint16)(rA&0xFF) - (sint16)Work8 + (sint16)F_C - 1;
		F_C = (tmp >= 0);
		if (((rA & 0xFF) ^ Work8) & ((rA & 0xFF) ^ (uint8) tmp) & 0x80)
			F_V = 1; else F_V = 0;
		rA = (rA&0xFF00) | (uint8)tmp;
		TESTB_2(rA);
	}
}

void	RTI()
{
  P = (P&0xff00) + pullb();
  UPDATE_FLAGS;
  PC = pullw();
  PB = pullb();
}

void	XCE()
{
// SNEmulDS doesn't support emulation mode	
  UPDATE_P;
  if ((P&P_C) && !(P&P_E))
    P = (P | P_E) & ~P_C;
  else
    if (!(P&P_C) && (P&P_E)) P = (P | P_C) & ~P_E;
  UPDATE_FLAGS;
}

void	MVN(uint8 SB)
{
  Cycles+=7*rA;
  do {
     mem_setbyte(Y, DB, mem_getbyte(X, SB));
     X++; Y++; rA--;
  } while (rA != 0xffff);
}

void	MVP(uint8 SB)
{
  Cycles+=7*rA;
  do {
     mem_setbyte(Y, DB, mem_getbyte(X, SB));
     X--; Y--; rA--;
  } while (rA != 0xffff);
}

void	BRK()
{
  pushb(PB);
  pushw(PC);
  UPDATE_P;
  pushb(P);
  
  PC = CPU.BRK;
  PB = 0;
}

void	COP()
{
  pushb(PB);
  pushw(PC);
  UPDATE_P;
  pushb(P);
  
  PC = CPU.COP;
  PB = 0;
}

void CPU_goto(int cycles)
{	
	uint8 			*PC_base;
	register uint8	*PCptr;
	uint8 			OpCode;	

/* Internal sub functions */
void do_branch()
{
  sint8 move = (sint8)FETCHBA; 
#if 0
  if (move < 0)
  {
  	PCptr += move;
// FIXME check PC: si changement du quartet de poid fort -> cycles = 4
// (PC&0xFF)+move < 0 ou (PC&0xFF)+move >= 0x100  
  	Cycles += 3;
  	return;
  }
#endif
  PCptr += move;
  if (CPU.WaitAddress != REAL_PC)
  {
  	Cycles++;
  	return;
  }
  if (CPU.WaitCycles > Cycles)
  {
  	Cycles = CPU.WaitCycles; 
  }  	
}
#include "opc_macros.h"	

/* end of internal sub functions */
	
#include "opc_defs.h"

/* ---- Main CPU loop starts here  ---- */	
	Cycles = 0;
	PCptr = map_memory(PC, PB);
	PC_base = PCptr-PC;
	UPDATE_FLAGS;
	if (CFG.CPU_speedhack != 0)
		CPU.Cycles = (CPU.Cycles+CPU.Cycles+CPU.Cycles) >> 2; 
loop_back:		
	while (Cycles < cycles)
	{
		CPU.LastAddress = REAL_PC; // Je ne suis pas convaincu de l'interet de ca
//		if (CFG.CPU_log)
/*		{
			trace_CPU();
		}*/
			
		OpCode = *PCptr++;		
#if 1		
		Cycles += OpCycles[OpCode];
#else		
		Cycles += 3;
#endif		
				
#ifdef SNEMUL_LOGGING
START_PROFILE(OPC[OpCode], 3);
#endif
		goto *OpCodes[OpCode];		
/*loop_back:		
		#ifdef SNEMUL_LOGGING
		END_PROFILE(OPC[OpCode], 3);
		#endif
		SNES.stat_OPC_cnt[OpCode]++;*/
//    	if (CPU_break) return;	//CPU_break a variable breakpoint (prevents execution in emu context)
	}
	PC = REAL_PC;
	UPDATE_P;
	
/*	
#ifdef  	
	if (SNES.stat_before > DISP_Y)
	   SNES.stat_CPU += 262+DISP_Y-SNES.stat_before;
	else
	   SNES.stat_CPU += DISP_Y-SNES.stat_before;
#else
	if (SNES.stat_before > TIMER3_DATA)
		SNES.stat_CPU += 65536+TIMER3_DATA-SNES.stat_before;
	else
		SNES.stat_CPU += TIMER3_DATA-SNES.stat_before;
#endif
*/
    return;

illegal:
	printf("\n%02x:%04x Invalid opcode : %x\n", PB, CPU.LastAddress, OpCode);
	printf("Try this: Disable speed hack + reset\n");
	CPU.IsBreak = 1;
//	while (1);
	return;

/* Opcodes start here */

// 00 : BRK 2/8 A FAIRE !
I_BRK: T("I_BRK"); {
  PCptr++;
  PC = REAL_PC;
  BRK();
  UPDATE_PC;
OPEND(8)

// 02 : COP 2/8
OPBEGIN(I_COP)
  PCptr++;
  PC = REAL_PC;
  COP();
  UPDATE_PC;
OPEND(8)

// EA : NOP 1/2
I_NOP: T("I_NOP"); {
  /*Cycles+=2;*/
  goto loop_back; }

// 14 : TRB d 2/5
I_TRB_d: T("I_TRB_d"); {
  _TRB_(d); 
  /*Cycles+=5;*/
  goto loop_back; }

// 1C : TRB a 3/6
I_TRB_a: T("I_TRB_a"); {
  _TRB_(a); 
  /*Cycles+=6;*/
  goto loop_back; }

// 04 : TSB d 2/5
I_TSB_d: T("I_TSB_d"); {
  _TSB_(d); 
  /*Cycles+=5;*/
  goto loop_back; }

// 0C : TSB a 3/6
I_TSB_a: T("I_TSB_a"); {
  _TSB_(a); 
  /*Cycles+=6;*/
  goto loop_back; }

// 09 : ORA # 2/2
I_ORA_i: T("I_ORA_i"); {
  _ORA_(i);
  /*Cycles+=2;*/
  goto loop_back; }

// 01 : ORA (d,x) 2/6
I_ORA_d_x_i: T("I_ORA_d_x_i"); {
  _ORA_(d_x_i); 
  /*Cycles+=6;*/
  goto loop_back; }

// 13 : ORA (d,s),y 2/7
I_ORA_d_s_i_y: T("I_ORA_d_s_i_y"); {
  _ORA_(d_s_i_y); 
  /*Cycles+=7;*/
  goto loop_back; }

// 07 : ORA [d] 2/6
I_ORA_d_i_l: T("I_ORA_d_i_l"); {
  _ORA_(d_i_l); 
  /*Cycles+=6;*/
  goto loop_back; }

// 03 : ORA d,s 2/4
I_ORA_d_s: T("I_ORA_d_s"); {
  _ORA_(d_s);
  /*Cycles+=4;*/
  goto loop_back; }

// 05 : ORA d 2/3
I_ORA_d: T("I_ORA_d"); {
  _ORA_(d);
  /*Cycles+=3;*/
  goto loop_back; }

// 15 : ORA d,x 2/4
I_ORA_d_x: T("I_ORA_d_x"); {
  _ORA_(d_x);
  /*Cycles+=4;*/
  goto loop_back; }

// 1D : ORA a,x 3/4
I_ORA_a_x: T("I_ORA_a_x"); {
  _ORA_(a_x);
  /*Cycles+=4;*/
  goto loop_back; }

// 19 : ORA a,y 3/4
I_ORA_a_y: T("I_ORA_a_y"); {
  _ORA_(a_y);
  /*Cycles+=4;*/
  goto loop_back; }

// 0D : ORA a 3/4
I_ORA_a: T("I_ORA_a"); {
  _ORA_(a);
  /*Cycles+=4;*/
  goto loop_back; }

// 0F : ORA al 4/5
I_ORA_al: T("I_ORA_al"); {
  _ORA_(al);
  /*Cycles+=5;*/
  goto loop_back; }

// 1F : ORA al_x 4/5
I_ORA_al_x: T("I_ORA_al_x"); {
  _ORA_(al_x);
  /*Cycles+=5;*/
  goto loop_back; }

// ?? : ORA (d),y ?/5
I_ORA_d_i_y: T("I_ORA_d_i_y"); {
  _ORA_(d_i_y);
  /*Cycles+=5;*/
  goto loop_back; }

// ?? : ORA (d) ?/5
I_ORA_d_i: T("I_ORA_d_i"); {
  _ORA_(d_i);
  /*Cycles+=5;*/
  goto loop_back; }

// ?? : ORA [d],y ?/7
I_ORA_d_i_l_y: T("I_ORA_d_i_l_y"); {
  _ORA_(d_i_l_y);
  /*Cycles+=5;*/
  goto loop_back; }

 
// 06 : ASL d 2/5
I_ASL_d: T("I_ASL_d"); {
  _ASL_(d);
  /*Cycles+=5;*/
  goto loop_back; }

// 16 : ASL d,x 2/6
I_ASL_d_x: T("I_ASL_d_x"); {
  _ASL_(d_x);
  /*Cycles+=6;*/
  goto loop_back; }

// 0E : ASL a 3/6
I_ASL_a: T("I_ASL_a"); {
  _ASL_(a);  
  /*Cycles+=6;*/
  goto loop_back; }


// 1E : ASL a,x 3/7
I_ASL_a_x: T("I_ASL_a_x"); {
  _ASL_(a_x);  
  /*Cycles+=7;*/
  goto loop_back; }

// 0A : ASL A 1/2
I_ASL_A: T("I_ASL_A"); {
  if (P&P_M) {
    F_C = (rA&0x80) >> 7;
    rA = (rA&0xff00) | ((rA << 1)&0xff); TESTB_2(rA);
  } else {
	F_C = (rA&0x8000) >> 15;
    rA <<= 1; TESTW_2(rA);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// 4A : LSR rA 1/2
I_LSR_A: T("I_LSR_A"); {
  if (P&P_M) {
    F_C = (rA&0x1);
    rA = (rA&0xff00) | ((rA&0xff) >> 1); TESTB_2(rA);
  } else {
	F_C = (rA&0x1);
    rA >>= 1;  TESTW_2(rA);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// 46 : LSR d 2/5
I_LSR_d: T("I_LSR_d"); {
  _LSR_(d);
  /*Cycles+=5;*/
  goto loop_back; }

// ?? : LSR d,x ?
I_LSR_d_x: T("I_LSR_d_x"); {
  _LSR_(d_x);
  /*Cycles+=6;*/
  goto loop_back; }

// 5E : LSR a,x 3/7
I_LSR_a_x: T("I_LSR_a_x"); {
  _LSR_(a_x);  
  /*Cycles+=7;*/
  goto loop_back; }

// 4E : LSR a 3/6
I_LSR_a: T("I_LSR_a"); {
  _LSR_(a);   
  /*Cycles+=6;*/
  goto loop_back; }

// 1A : INC rA 1/2
I_INC_A: T("I_INC_A"); {
  _INC_(rA);
   /*Cycles+=2;*/
  goto loop_back; }

// F6 : INC d,x 2/6
I_INC_d_x: T("I_INC_d_x"); {
  _INC_(d_x);
  /*Cycles+=6;*/
  goto loop_back; }

// E6 : INC d 2/5
I_INC_d: T("I_INC_d"); {
  _INC_(d);
  /*Cycles+=5;*/
  goto loop_back; }

// EE : INC a 3/6
I_INC_a: T("I_INC_a"); {
  _INC_(a);
  /*Cycles+=6;*/
  goto loop_back; }

// FE : INC a,x 3/7
I_INC_a_x: T("I_INC_a_x"); {
  _INC_(a_x);
  /*Cycles+=7;*/
  goto loop_back; }

// E8 : INX i 1/2
I_INX: T("I_INX"); {
  if (P&P_X) {
    X = (X&0xff00) | ((X+1)&0xff); TESTB_2(X);
  } else {
    X++; TESTW_2(X);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// C8 : INY i 1/2
I_INY: T("I_INY"); {
  if (P&P_X) {
    Y = (Y&0xff00) | ((Y+1)&0xff); TESTB_2(Y);
  } else {
    Y++; TESTW_2(Y);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// 21 : AND (d,x) 2/6
I_AND_d_x_i: T("I_AND_d_x_i"); {
  _AND_(d_x_i); 
  /*Cycles+=6;*/
  goto loop_back; }

// 32 : AND (d) 2/5
I_AND_d_i: T("I_AND_d_i"); {
  _AND_(d_i); 
  /*Cycles+=5;*/
  goto loop_back; }

// 29 : AND # 2/2
I_AND_i: T("I_AND_i"); {
  _AND_(i);
  /*Cycles+=2;*/
  goto loop_back; }

// ?? : AND [d] 2/6
I_AND_d_i_l: T("I_AND_d_i_l"); {
  _AND_(d_i_l);
  /*Cycles+=6;*/
  goto loop_back; }

// ?? : AND (d,s),y ?/?
I_AND_d_s_i_y: T("I_AND_d_s_i_y"); {
  _AND_(d_s_i_y); 
  /*Cycles+=7;*/
  goto loop_back; }

// 37 : AND [d],y 2/6
I_AND_d_i_l_y: T("I_AND_d_i_l_y"); {
  _AND_(d_i_l_y);
  /*Cycles+=6;*/
  goto loop_back; }

// 31 : AND (d),y 2/5
I_AND_d_i_y: T("I_AND_d_i_y"); {
  _AND_(d_i_y);
  /*Cycles+=5;*/
  goto loop_back; }

// 25 : AND d 2/3
I_AND_d: T("I_AND_d"); {
  _AND_(d); 
  /*Cycles+=3;*/
  goto loop_back; }

// ?? : AND d,s ?/?
I_AND_d_s: T("I_AND_d_s"); {
  _AND_(d_s); 
  /*Cycles+=3;*/
  goto loop_back; }

// 35 : AND d,x 2/4
I_AND_d_x: T("I_AND_d_x"); {
  _AND_(d_x); 
  /*Cycles+=4;*/
  goto loop_back; }

// 2D : AND a 3/4
I_AND_a: T("I_AND_a"); {
  _AND_(a); 
  /*Cycles+=4;*/
  goto loop_back; }

// 2F : AND al 4/5
I_AND_al: T("I_AND_al"); {
  _AND_(al); 
  /*Cycles+=5;*/
  goto loop_back; }

// 3F : AND al,x 4/5
I_AND_al_x: T("I_AND_al_x"); {
  _AND_(al_x); 
  /*Cycles+=5;*/
  goto loop_back; }

// 3D : AND a,x 3/4
I_AND_a_x: T("I_AND_a_x"); {
  _AND_(a_x);
  /*Cycles+=4;*/
  goto loop_back; }

// 39 : AND a,y 3/4
I_AND_a_y: T("I_AND_a_y"); {
  _AND_(a_y);
  /*Cycles+=4;*/
  goto loop_back; }

// 24 : BIT d 2/3
I_BIT_d: T("I_BIT_d"); {
  _BIT_(d); 
  /*Cycles+=3;*/
  goto loop_back; }

// 34 : BIT d,x 2/4
I_BIT_d_x: T("I_BIT_d_x"); {
  _BIT_(d_x); 
  /*Cycles+=4;*/
  goto loop_back; }

// 3C : BIT a,x 3/4
I_BIT_a_x: T("I_BIT_a_x"); {
  _BIT_(a_x); 
  /*Cycles+=4;*/
  goto loop_back; }

// 2C : BIT a 3/4
I_BIT_a: T("I_BIT_a"); {
  _BIT_(a);
  /*Cycles+=4;*/
  goto loop_back; }

// 89 : BIT # 2/2
I_BIT: T("I_BIT"); {
  _BIT_(i);
  /*Cycles+=2;*/
  goto loop_back; }

// 2A : ROL rA 1/2
I_ROL_A: T("I_ROL_A"); {
  _ROL_(rA); /*Cycles+=2;*/
  goto loop_back; }

// 2E : ROL a 3/7
I_ROL_a: T("I_ROL_a"); {
  _ROL_(a); /*Cycles+=7;*/
  goto loop_back; }

// 3E : ROL a,x 3/7
I_ROL_a_x: T("I_ROL_a_x"); {
  _ROL_(a_x); /*Cycles+=7;*/
  goto loop_back; }

// 26 : ROL d 2/5
I_ROL_d: T("I_ROL_d"); {
  _ROL_(d); /*Cycles+=5;*/
  goto loop_back; }

// 36 : ROL d,x 2/6
I_ROL_d_x: T("I_ROL_d_x"); {
  _ROL_(d_x); /*Cycles+=6;*/
  goto loop_back; }

// 3A : DEC rA 1/2
I_DEC: T("I_DEC"); {
  _DEC_(rA); /*Cycles+=2;*/
  goto loop_back; }

// C6 : DEC d 2/5
I_DEC_d: T("I_DEC_d"); {
  _DEC_(d); /*Cycles+=5;*/
  goto loop_back; }

// D6 : DEC d,x 2/6
I_DEC_d_x: T("I_DEC_d_x"); {
  _DEC_(d_x); /*Cycles+=6;*/
  goto loop_back; }

// CE : DEC a 3/4
I_DEC_a: T("I_DEC_a"); {
  _DEC_(a); /*Cycles+=4;*/
  goto loop_back; }

// DE : DEC a,x 3/7
I_DEC_a_x: T("I_DEC_a_x"); {
  _DEC_(a_x); /*Cycles+=7;*/
  goto loop_back; }

// 88 : DEY i 1/2
I_DEY: T("I_DEY"); {
  if (P&P_X) {
    Y = (Y&0xff00)|((Y-1)&0xff); TESTB_2(Y)
  } else {
    Y--; TESTW_2(Y);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// CA : DEX i 1/2
I_DEX: T("I_DEX"); {
  if (P&P_X) {
    X = (X&0xff00)|((X-1)&0xff); TESTB_2(X)
  } else {
    X--; TESTW_2(X);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// 64 : STZ d 2/3
I_STZ_d: T("I_STZ_d"); {
  _STZ_(d); /*Cycles+=3;*/
  goto loop_back; }

// 74 : STZ d,x 2/4
I_STZ_d_x: T("I_STZ_d_x"); {
  _STZ_(d_x); /*Cycles+=4;*/
  goto loop_back; }

// 9C : STZ a 3/4
I_STZ_a: T("I_STZ_a"); {
  _STZ_(a); /*Cycles+=4;*/
  goto loop_back; }

// 9E : STZ a,x 3/5
I_STZ_a_x: T("I_STZ_a_x"); {
  _STZ_(a_x); /*Cycles+=5;*/
  goto loop_back; }

// 69 : ADC # 2/2
I_ADC: T("I_ADC"); {
  _ADC_(i); 
  /*Cycles+=2;*/
  goto loop_back; }

// 65 : ADC d 2/3
I_ADC_d: T("I_ADC_d"); {
  _ADC_(d); /*Cycles+=3;*/
  goto loop_back; }

// 72 : ADC (d) 2/5
I_ADC_d_i: T("I_ADC_d_i"); {
  _ADC_(d_i); /*Cycles+=5;*/
  goto loop_back; }

// 67 : ADC [d] 2/6
I_ADC_d_i_l: T("I_ADC_d_i_l"); {
  _ADC_(d_i_l); /*Cycles+=6;*/
  goto loop_back; }

// 77 : ADC [d],y 2/6
I_ADC_d_i_l_y: T("I_ADC_d_i_l_y"); {
  _ADC_(d_i_l_y); /*Cycles+=6;*/
  goto loop_back; }

// 71 : ADC d_i_y 2/5
I_ADC_d_i_y: T("I_ADC_d_i_y"); {
  _ADC_(d_i_y); /*Cycles+=5;*/
  goto loop_back; }


// 61 : ADC d_x_i 2/6
I_ADC_d_x_i: T("I_ADC_d_x_i"); {
  _ADC_(d_x_i); /*Cycles+=6;*/
  goto loop_back; }

// 75 : ADC d_x 2/4
I_ADC_d_x: T("I_ADC_d_x"); {
  _ADC_(d_x); /*Cycles+=4;*/
  goto loop_back; }

// 63 : ADC d_s 2/4
I_ADC_d_s: T("I_ADC_d_s"); {
  _ADC_(d_s); /*Cycles+=4;*/
  goto loop_back; }

// ?? : ADC d_s_i_y 2/7
I_ADC_d_s_i_y: T("I_ADC_d_s_i_y"); {
  _ADC_(d_s_i_y); /*Cycles+=7;*/
  goto loop_back; }

// 79 : ADC a,y 3/4
I_ADC_a_y: T("I_ADC_a_y"); {
  _ADC_(a_y); /*Cycles+=4;*/
  goto loop_back; }

// 7D : ADC a,x 3/4
I_ADC_a_x: T("I_ADC_a_x"); {
  _ADC_(a_x); /*Cycles+=4;*/
  goto loop_back; }

// 6F : ADC al 4/5
I_ADC_al: T("I_ADC_al"); {
  _ADC_(al); /*Cycles+=5;*/
  goto loop_back; }

// 7F : ADC al,x 4/5
I_ADC_al_x: T("I_ADC_al_x"); {
  _ADC_(al_x); /*Cycles+=5;*/
  goto loop_back; }

// 6D : ADC a 3/4
I_ADC_a: T("I_ADC_a"); {
  _ADC_(a); /*Cycles+=4;*/
  goto loop_back; }

// E9 : SBC # 2/2
I_SBC: T("I_SBC"); {
  _SBC_(i);
  /*Cycles+=2;*/
  goto loop_back; }

// E5 : SBC d 2/3
I_SBC_d: T("I_SBC_d"); {
  _SBC_(d); 
  /*Cycles+=3;*/
  goto loop_back; }

// F2 : SBC (d) 2/5
I_SBC_d_i: T("I_SBC_d_i"); {
  _SBC_(d_i); 
  /*Cycles+=5;*/
  goto loop_back; }

// E3 : SBC d,s 2/4
I_SBC_d_s: T("I_SBC_d_s"); {
  _SBC_(d_s); 
  /*Cycles+=4;*/
  goto loop_back; }

// F1 : SBC d_i_y 2/5
I_SBC_d_i_y: T("I_SBC_d_i_y"); {
  _SBC_(d_i_y); 
  /*Cycles+=5;*/
  goto loop_back; }

// F7 : SBC d_i_l_y 2/6
I_SBC_d_i_l_y: T("I_SBC_d_i_l_y"); {
  _SBC_(d_i_l_y); 
  /*Cycles+=6;*/
  goto loop_back; }

// F5 : SBC d,x 2/4
I_SBC_d_x: T("I_SBC_d_x"); {
  _SBC_(d_x); 
  /*Cycles+=4;*/
  goto loop_back; }

// ?? : SBC (d,x) ?
I_SBC_d_x_i: T("I_SBC_d_x_i"); {
  _SBC_(d_x_i); 
  /*Cycles+=5;*/
  goto loop_back; }

// ?? : SBC [d] ?
I_SBC_d_i_l: T("I_SBC_d_i_l"); {
  _SBC_(d_i_l); 
  /*Cycles+=6;*/
  goto loop_back; }

// ?? : SBC (d,s),y ?
I_SBC_d_s_i_y: T("I_SBC_d_s_i_y"); {
  _SBC_(d_s_i_y); 
  /*Cycles+=7;*/
  goto loop_back; }

// F9 : SBC a,y 3/4
I_SBC_a_y: T("I_SBC_a_y"); {
  _SBC_(a_y); 
  /*Cycles+=4;*/
  goto loop_back; }

// FD : SBC a,x 3/4
I_SBC_a_x: T("I_SBC_a_x"); {
  _SBC_(a_x); 
  /*Cycles+=4;*/
  goto loop_back; }

// ED : SBC a 3/4
I_SBC_a: T("I_SBC_a"); {
  _SBC_(a); 
  /*Cycles+=4;*/
  goto loop_back; }

// EF : SBC al 4/5
I_SBC_al: T("I_SBC_al"); {
  _SBC_(al); 
  /*Cycles+=5;*/
  goto loop_back; }

// FF : SBC al,x 4/5
I_SBC_al_x: T("I_SBC_al_x"); {
  _SBC_(al_x); 
  /*Cycles+=5;*/
  goto loop_back; } 

// 6A : ROR rA 1/2
I_ROR_A: T("I_ROR_A"); {
  _ROR_(rA); /*Cycles+=2;*/
  goto loop_back; }

// 6E : ROR a 3/6
I_ROR_a: T("I_ROR_a"); {
  _ROR_(a); 
  /*Cycles+=6;*/
  goto loop_back; }

// 7E : ROR a,x 3/7
I_ROR_a_x: T("I_ROR_a_x"); {
  _ROR_(a_x); 
  /*Cycles+=7;*/
  goto loop_back; }

// 66 : ROR d 2/5
I_ROR_d: T("I_ROR_d"); {
  _ROR_(d); 
  /*Cycles+=5;*/
  goto loop_back; }

// 76 : ROR d,x 2/6
I_ROR_d_x: T("I_ROR_d_x"); {
  _ROR_(d_x); 
  /*Cycles+=6;*/
  goto loop_back; }

// 84 : STY d 2/3
I_STY_d: T("I_STY_d"); {
  _STY_(d); /*Cycles+=3;*/
  goto loop_back; }

// 8C : STY a 3/4
I_STY_a: T("I_STY_a"); {
  _STY_(a); /*Cycles+=4;*/
  goto loop_back; }

// 94 : STY d,x 2/4
I_STY_d_x: T("I_STY_d_x"); {
  _STY_(d_x); /*Cycles+=4;*/
  goto loop_back; }


// 86 : STX d 2/3
I_STX_d: T("I_STX_d"); {
  _STX_(d);
OPEND(3)

// 8E : STX a 3/4
I_STX_a: T("I_STX_a"); {
  _STX_(a);
OPEND(4)

// 85 : STA d 2/3
I_STA_d: T("I_STA_d"); {
  _STA_(d); /*Cycles+=3;*/
  goto loop_back; }

// 83 : STA d_s 2/4
I_STA_d_s: T("I_STA_d_s"); {
  _STA_(d_s); /*Cycles+=4;*/
  goto loop_back; }

// ?? : STA d_s_i_y ?
I_STA_d_s_i_y: T("I_STA_d_s_i_y"); {
  _STA_(d_s_i_y); /*Cycles+=7;*/
  goto loop_back; }

// 8D : STA a 3/4
I_STA_a: T("I_STA_a"); {
//  sta_a();
  _STA_(a); /*Cycles+=4;*/
  goto loop_back; }

// 8F : STA al 4/5
I_STA_al: T("I_STA_al"); {
  _STA_(al); /*Cycles+=5;*/
  goto loop_back; }

// 95 : STA d,x 2/3
I_STA_d_x: T("I_STA_d_x"); {
  _STA_(d_x); /*Cycles+=3;*/
  goto loop_back; }

// 96 : STX d,y 2/4
I_STX_d_y: T("I_STX_d_y"); {
  _STX_(d_y); /*Cycles+=4;*/
  goto loop_back; }

// 9D : STA a,x 3/5
I_STA_a_x: T("I_STA_a_x"); {
  _STA_(a_x); /*Cycles+=5;*/
  goto loop_back; }

// 99 : STA a,y 3/5
I_STA_a_y: T("I_STA_a_y"); {
  _STA_(a_y); /*Cycles+=5;*/
  goto loop_back; }

// 9F : STA al,x 4/5
I_STA_al_x: T("I_STA_al_x"); {
//  sta_al_x();
  _STA_(al_x); /*Cycles+=5;*/
  goto loop_back; }

// 97 : STA [d],y 2/6
I_STA_d_i_l_y: T("I_STA_d_i_l_y"); {
  _STA_(d_i_l_y); /*Cycles+=6;*/
  goto loop_back; }

// 87 : STA [d] 2/6
I_STA_d_i_l: T("I_STA_d_i_l"); {
  _STA_(d_i_l); /*Cycles+=6;*/
  goto loop_back; }

// 91 : STA (d),y 2/6
I_STA_d_i_y: T("I_STA_d_i_y"); {
  _STA_(d_i_y); /*Cycles+=6;*/
  goto loop_back; }

// 92 : STA (d) 2/5
I_STA_d_i: T("I_STA_d_i"); {
  _STA_(d_i); /*Cycles+=5;*/
  goto loop_back; }

// 81 : STA (d,x) 2/6
I_STA_d_x_i: T("I_STA_d_x_i"); {
  _STA_(d_x_i); /*Cycles+=6;*/
  goto loop_back; }

// A2 : LDX # 2/2
I_LDX: T("I_LDX"); {
  _LDX_(iX);
  /*Cycles+=2;*/
  goto loop_back; }

// A6 : LDX d 2/3
I_LDX_d: T("I_LDX_d"); {
  _LDX_(d); /*Cycles+=3;*/
  goto loop_back; }

// B6 : LDX d,y 2/4
I_LDX_d_y: T("I_LDX_d_y"); {
  _LDX_(d_y); /*Cycles+=4;*/
  goto loop_back; }

// A0 : LDY # 2/2
I_LDY: T("I_LDY"); {
 _LDY_(iX);
 /*Cycles+=2;*/
  goto loop_back; }

// A4 : LDY d 2/3
I_LDY_d: T("I_LDY_d"); {
  _LDY_(d); /*Cycles+=3;*/
  goto loop_back; }

// A3 : LDA d,s 2/4
I_LDA_d_s: T("I_LDA_d_s"); {
  _LDA_(d_s); /*Cycles+=4;*/
  goto loop_back; }

// B3 : LDA (d,s),y 2/7
I_LDA_d_s_i_y: T("I_LDA_d_s_i_y"); {
  _LDA_(d_s_i_y); /*Cycles+=7;*/
  goto loop_back; }

// A5 : LDA d 2/3
I_LDA_d: T("I_LDA_d"); {
  _LDA_(d); /*Cycles+=3;*/
  goto loop_back; }

// A9 : LDA # 2/2
I_LDA: T("I_LDA"); {
  _LDA_(i);
  /*Cycles+=2;*/
  goto loop_back; }

// B4 : LDY d,x 2/4
I_LDY_d_x: T("I_LDY_d_x"); {
  _LDY_(d_x); /*Cycles+=4;*/
  goto loop_back; }

// A1 : LDA (d,x) 2/6
I_LDA_d_x_i: T("I_LDA_d_x_i"); {
  _LDA_(d_x_i); /*Cycles+=6;*/
  goto loop_back; }

// BC : LDY a,x 3/4
I_LDY_a_x: T("I_LDY_a_x"); {
  _LDY_(a_x); /*Cycles+=4;*/
  goto loop_back; }

// AC : LDY a 3/4
I_LDY_a: T("I_LDY_a"); {
  _LDY_(a); /*Cycles+=4;*/
  goto loop_back; }

// AD : LDA a 3/4
I_LDA_a: T("I_LDA_a"); {
  _LDA_(a); /*Cycles+=4;*/
  goto loop_back; }

// AF : LDA al 4/5
I_LDA_al: T("I_LDA_al"); {
  _LDA_(al); /*Cycles+=5;*/
  goto loop_back; }

// AE : LDX a 3/4
I_LDX_a: T("I_LDX_a"); {
  _LDX_(a); /*Cycles+=4;*/
  goto loop_back; }

// BE : LDX a,y 3/4
I_LDX_a_y: T("I_LDX_a_y"); {
  _LDX_(a_y); /*Cycles+=4;*/
  goto loop_back; }

// B1 : LDA (d),y 2/5
I_LDA_d_i_y: T("I_LDA_d_i_y"); {
  _LDA_(d_i_y); /*Cycles+=5;*/
  goto loop_back; }

// B2 : LDA (d) 3/4
I_LDA_d_i: T("I_LDA_d_i"); {
  _LDA_(d_i); /*Cycles+=4;*/
  goto loop_back; }

// B7 : LDA [d],y 2/6
I_LDA_d_i_l_y: T("I_LDA_d_i_l_y"); {
  _LDA_(d_i_l_y); /*Cycles+=6;*/
  goto loop_back; }

// A7 : LDA [d] 2/6
I_LDA_d_i_l: T("I_LDA_d_i_l"); {
  _LDA_(d_i_l); /*Cycles+=6;*/
  goto loop_back; }

// B9 : LDA a,y 3/4
I_LDA_a_y: T("I_LDA_a_y"); {
  _LDA_(a_y); /*Cycles+=4;*/
  goto loop_back; }

// B5 : LDA d,x 2/4
I_LDA_d_x: T("I_LDA_d_x"); {
  _LDA_(d_x); /*Cycles+=4;*/
  goto loop_back; }

// BD : LDA a,x 3/4
I_LDA_a_x: T("I_LDA_a_x"); {
  _LDA_(a_x); /*Cycles+=4;*/
  goto loop_back; }

// BF : LDA al,x 4/5
I_LDA_al_x: T("I_LDA_al_x"); {
  _LDA_(al_x); /*Cycles+=5;*/
  goto loop_back; }

// C0 : CPY # 2/2
I_CPY: T("I_CPY"); {
  _CPY_(iX);
  /*Cycles+=2;*/
  goto loop_back; }

// CC : CPY a 3/4
I_CPY_a: T("I_CPY_a"); {
  _CPY_(a); /*Cycles+=4;*/
  goto loop_back; }

// C4 : CPY d 2/3
I_CPY_d: T("I_CPY_d"); {
  _CPY_(d); /*Cycles+=3;*/
  goto loop_back; }

// C5 : CMP d 2/3
I_CMP_d: T("I_CMP_d"); {
  _CMP_(d); /*Cycles+=3;*/
  goto loop_back; }

// C3 : CMP d,s 2/4
I_CMP_d_s: T("I_CMP_d_s"); {
  _CMP_(d_s); /*Cycles+=4;*/
  goto loop_back; }

// C7 : CMP [d] 2/6
I_CMP_d_i_l: T("I_CMP_d_i_l"); {
  _CMP_(d_i_l); /*Cycles+=6;*/
  goto loop_back; }

// D2 : CMP (d) 2/5
I_CMP_d_i: T("I_CMP_d_i"); {
  _CMP_(d_i); /*Cycles+=5;*/
  goto loop_back; }

// C9 : CMP # 2/2
I_CMP: T("I_CMP"); {
  _CMP_(i);
  /*Cycles+=2;*/
  goto loop_back; }

// D7 : CMP [d],y 2/6
I_CMP_d_i_l_y: T("I_CMP_d_i_l_y"); {
  _CMP_(d_i_l_y); /*Cycles+=6;*/
  goto loop_back; }

// CD : CMP a 3/4
I_CMP_a: T("I_CMP_a"); {
  _CMP_(a); /*Cycles+=4;*/
  goto loop_back; }

// D5 : CMP d,x 2/4
I_CMP_d_x: T("I_CMP_d_x"); {
  _CMP_(d_x); /*Cycles+=4;*/
  goto loop_back; }

// ?? : CMP (d,x) ?
I_CMP_d_x_i: T("I_CMP_d_x_i"); {
  _CMP_(d_x_i); /*Cycles+=5;*/
  goto loop_back; }

// ?? : CMP (d),y ?
I_CMP_d_s_i_y: T("I_CMP_d_s_i_y"); {
  _CMP_(d_s_i_y); /*Cycles+=7;*/
  goto loop_back; }

// D1 : CMP (d),y 2/5
I_CMP_d_i_y: T("I_CMP_d_i_y"); {
  _CMP_(d_i_y); /*Cycles+=5;*/
  goto loop_back; }

// D9 : CMP a,y 3/4
I_CMP_a_y: T("I_CMP_a_y"); {
  _CMP_(a_y);  /*Cycles+=4;*/
  goto loop_back; }

// DD : CMP a,x 3/4
I_CMP_a_x: T("I_CMP_a_x"); {
  _CMP_(a_x); /*Cycles+=4;*/
  goto loop_back; }

// DF : CMP al,x 4/5
I_CMP_al_x: T("I_CMP_al_x"); {
  _CMP_(al_x); /*Cycles+=5;*/
  goto loop_back; }

// CF : CMP al 4/5
I_CMP_al: T("I_CMP_al"); {
  _CMP_(al);  /*Cycles+=5;*/
  goto loop_back; }

// E0 : CPX # 2/2
I_CPX: T("I_CPX"); {
  _CPX_(iX);
  /*Cycles+=2;*/
  goto loop_back; }

// E4 : CPX d 2/3
I_CPX_d: T("I_CPX_d"); {
  _CPX_(d); /*Cycles+=3;*/
  goto loop_back; }

// EC : CPX a 3/4
I_CPX_a: T("I_CPX_a"); {
  _CPX_(a); /*Cycles+=4;*/
  goto loop_back; }

// 49 : EOR # 2/2
I_EOR: T("I_EOR"); {
  _EOR_(i);
  /*Cycles+=2;*/
  goto loop_back; }

// 45 : EOR d 2/3
I_EOR_d: T("I_EOR_d"); {
  _EOR_(d); /*Cycles+=3;*/
  goto loop_back; }

// 47 : EOR [d] 2/6
I_EOR_d_i_l: T("I_EOR_d_i_l"); {
  _EOR_(d_i_l); /*Cycles+=6;*/
  goto loop_back; }

// 57 : EOR [d],y 2/6
I_EOR_d_i_l_y: T("I_EOR_d_i_l_y"); {
  _EOR_(d_i_l_y); /*Cycles+=6;*/
  goto loop_back; }

// 55 : EOR d,x 2/4
I_EOR_d_x: T("I_EOR_d_x"); {
  _EOR_(d_x); /*Cycles+=4;*/
  goto loop_back; }

// ?? : EOR (d,x) ?
I_EOR_d_x_i: T("I_EOR_d_x_i"); {
  _EOR_(d_x_i); /*Cycles+=6;*/
  goto loop_back; }

// ?? : EOR (d,s) ?
I_EOR_d_s: T("I_EOR_d_s"); {
  _EOR_(d_s); /*Cycles+=4;*/
  goto loop_back; }

// ?? : EOR al ?
I_EOR_al: T("I_EOR_al"); {
  _EOR_(al); /*Cycles+=5;*/
  goto loop_back; }

// ?? : EOR (d),y ?
I_EOR_d_i_y: T("I_EOR_d_i_y"); {
  _EOR_(d_i_y); /*Cycles+=5;*/
  goto loop_back; }

// ?? : EOR (d) ?
I_EOR_d_i: T("I_EOR_d_i"); {
  _EOR_(d_i); /*Cycles+=5;*/
  goto loop_back; }

// ?? : EOR (d,s),y ?
I_EOR_d_s_i_y: T("I_EOR_d_s_i_y"); {
  _EOR_(d_s_i_y); /*Cycles+=7;*/
  goto loop_back; }


// 4D : EOR a 3/4
I_EOR_a: T("I_EOR_a"); {
  _EOR_(a); /*Cycles+=4;*/
  goto loop_back; }

// 59 : EOR a,y 3/4
I_EOR_a_y: T("I_EOR_a_y"); {
  _EOR_(a_y); /*Cycles+=4;*/
  goto loop_back; }

// 5D : EOR a,x 3/4
I_EOR_a_x: T("I_EOR_a_x"); {
  _EOR_(a_x); /*Cycles+=4;*/
  goto loop_back; }

// 5F : EOR al,x 4/5
I_EOR_al_x: T("I_EOR_al_x"); {
  _EOR_(al_x); /*Cycles+=5;*/
  goto loop_back; }

// C2 : REP # 2/3
I_REP: T("I_REP"); {
  UPDATE_P;	
  P &= ~FETCHBA;
  UPDATE_FLAGS;  
  /*Cycles += 3;*/
  goto loop_back; }

// D0 : BNE r 2/2
I_BNE: T("I_BNE"); {
  if (F_Z) { do_branch(); goto loop_back; }
  PCptr++; /*Cycles += 2;*/
  goto loop_back; }

// D8 : CLD i 1/2
I_CLD: T("I_CLD"); {
  P &= ~P_D;
  /*Cycles+=2;*/
  goto loop_back; }

// DA : PHX i 1/3
I_PHX: T("I_PHX"); {
  if (P&P_X)
    pushb((uint8)X);
  else {
    pushw(X);
  }
  /*Cycles+=3;*/
  goto loop_back; }

// E2 : SEP #  2/3
I_SEP: T("I_SEP"); {
  UPDATE_P;
  P |= FETCHB;
  UPDATE_FLAGS;
  if (P&P_X) { Y &= 0xff; X &= 0xff; }
  PCptr++; /*Cycles += 3;*/
  goto loop_back; }

// EB : XBA i 1/3
I_XBA: T("I_XBA"); {
  rA = (rA >> 8) | (rA << 8); 
  TESTB_2(rA);
  /*Cycles+=3;*/
  goto loop_back; }

// F0 : BEQ r 2/2
I_BEQ: T("I_BEQ"); {
  if (!F_Z) { do_branch(); goto loop_back; }
  PCptr++; /*Cycles += 2;*/
  goto loop_back; }

// F8 : SED 1/2
I_SED: T("I_SED"); {
  P |= P_D;
  /*Cycles+=2;*/
  goto loop_back; }

// FA : PLX s 1/4
I_PLX: T("I_PLX"); {
  if (P&P_X) {
    X = (X&0xff00) + pullb(); TESTB_2(X);
  } else {
    X = pullw(); TESTW_2(X);
  }
  /*Cycles+=4;*/
  goto loop_back; }

// FB : XCE i 1/2
I_XCE: T("I_XCE"); {
  XCE();
OPEND(2)

// 08 : PHP s 1/3
I_PHP: T("I_PHP"); {
  UPDATE_P;
  pushb(P);
  /*Cycles+=3;*/
  goto loop_back; }

// 10 : BPL r 2/2
I_BPL: T("I_BPL"); {
  if (!(F_N & 0x80)) { do_branch(); goto loop_back; }
  PCptr++; /*Cycles += 2;*/	
  goto loop_back; }

// 18 : CLC i 1/2
I_CLC: T("I_CLC"); {
  F_C = 0;
  /*Cycles+=2;*/
  goto loop_back; }

// 1B : TCS i 1/2
I_TCS: T("I_TCS"); {
  S = rA;
  /*Cycles+=2;*/
  goto loop_back; }

// 3B : TSC i 1/2
I_TSC: T("I_TSC"); {
  rA = S;
  TESTW_2(rA);
  /*Cycles+=2;*/
  goto loop_back; }

// 20 : JSR a 3/6
I_JSR_a: T("I_JSR_a"); {
  pushw(REAL_PC+1);
  PC = FETCHW;
  UPDATE_PC;
  /*Cycles+=6;*/
  goto loop_back; }

// FC : JSR (a,x) 3/6
I_JSR_a_x_i: T("I_JSR_a_x_i"); {
  pushw(REAL_PC+1);
  PC = mem_getword(FETCHW+X, PB);
  UPDATE_PC;
  /*Cycles+=6;*/
  goto loop_back; }

// 28 : PLP s 1/4
I_PLP: T("I_PLP"); {
  P = (P&0xff00)+pullb();
  UPDATE_FLAGS;
  if (P&P_X) { Y &= 0xff; X &= 0xff; }
  /*Cycles+=4;*/
  goto loop_back; }

// 30 : BMI r 2/2
I_BMI: T("I_BMI"); {
  if (F_N&0x80) { do_branch(); goto loop_back; }
  PCptr++; /* Cycles += 2;*/
  goto loop_back; }

// 38 : SEC i 1/2
I_SEC: T("I_SEC"); {
  F_C = 1;
  /*Cycles+=2;*/
  goto loop_back; }

// 40 : RTI s 1/6
I_RTI: T("I_RTI"); {
  RTI();
  UPDATE_PC;
OPEND(6)

// 48 : PHA s 1/3
I_PHA: T("I_PHA"); {
  if (P&P_M)
    pushb(rA&0xFF);
  else
    pushw(rA);
  /*Cycles+=3;*/
  goto loop_back; }

// 4B : PHK s 1/3
I_PHK: T("I_PHK"); {
  pushb(PB);
  /*Cycles+=3;*/
  goto loop_back; }

// 4C : JMP a 3/3
I_JMP_a: T("I_JMP_a"); {
  PC = FETCHW;
  UPDATE_PC;
  /*Cycles+=3;*/
  goto loop_back; }

// 22 : JSL al 4/8
I_JSL_al: T("I_JSL_al"); {
//  uint8 tmp;
  pushb(PB);
  pushw(REAL_PC+2);
  PB = FETCHB2;
  PC = FETCHW;
  UPDATE_PC;
  /*Cycles+=8;*/
  goto loop_back; }

// 6B : RTL s 1/6
I_RTL: T("I_RTL"); {
  PC = pullw()+1;
  PB = pullb();
  UPDATE_PC;
  /*Cycles+=6;*/
  goto loop_back; }


// 5B : TCD i 1/2
I_TCD: T("I_TCD"); {
  D = rA;
  TESTW_2(rA);
  /*Cycles+=2;*/
  goto loop_back; }

// 5C : JMP al 4/4
I_JMP_al: T("I_JMP_al"); {
  uint8 tmp;
  tmp = FETCHB2;
  PC = FETCHW;
  PB = tmp;
  UPDATE_PC;
  /*Cycles+=4;*/
  goto loop_back; }

// DC : JML (a) 3/6
I_JML_a_i: T("I_JML_a_i"); {
  uint8 tmp;
  tmp = mem_getbyte(FETCHW+2, 0);
  PC  = mem_getword(FETCHW, 0);
  PB  = tmp;
  UPDATE_PC;
  /*Cycles+=6;*/
  goto loop_back; }


// 7C : JMP (a,x) 3/6
I_JMP_a_x_i: T("I_JMP_a_x_i"); {
  PC  = mem_getword(FETCHW+X, PB);
  UPDATE_PC;    	  
  /*Cycles+=6;*/
  goto loop_back; }

// 6C : JMP (a) 3/5
I_JMP_a_i: T("I_JMP_a_i"); {
  PC  = mem_getword(FETCHW, 0);
  UPDATE_PC;  
  /*Cycles+=5;*/
  goto loop_back; }

// 60 : RTS s 1/6
I_RTS: T("I_RTS"); {
  PC = pullw()+1;
  UPDATE_PC;
  /*Cycles+=6;*/
  goto loop_back; }

// 68 : PLA s 1/4
I_PLA: T("I_PLA"); {
  if (P&P_M) {
    rA = (rA&0xff00)+pullb(); TESTB_2(rA);
  } else {
    rA = pullw(); TESTW_2(rA);
  }
  /*Cycles+=4;*/
  goto loop_back; }

// 70 : BVS r 2/2
I_BVS: T("I_BVS"); {
   if (F_V) { do_branch(); goto loop_back; }
  PCptr++; /*Cycles += 2;*/	
  goto loop_back; }

// 50 : BVC r 2/2
I_BVC: T("I_BVC"); {
  if (!F_V) { do_branch(); goto loop_back; }
  PCptr++; /*Cycles += 2;*/	
  goto loop_back; }

// 78 : SEI i 1/2
I_SEI: T("I_SEI"); {
  P |= P_I;
  /*Cycles+=2;*/
  goto loop_back; }

// 7A : PLY s 1/4
I_PLY: T("I_PLY"); {
  if (P&P_X) {
    Y = (Y&0xff00)+pullb(); TESTB_2(Y);
  } else {
    Y = pullw(); TESTW_2(Y);
  }
  /*Cycles+=4;*/
  goto loop_back; }

// 7B : TDC i 1/2
I_TDC: T("I_TDC"); {
  rA = D;
  TESTW_2(rA);
  /*Cycles+=2;*/
  goto loop_back; }

// 80 : BRA r 2/2
I_BRA: T("I_BRA"); {
  do_branch();	
  goto loop_back; }

// 82 : BRL rl 3/3
I_BRL: T("I_BRL"); {
  PC = REAL_PC + (short)FETCHW+2;
  UPDATE_PC;
  /*Cycles += 3;*/
  goto loop_back; }

// 8A : TXA i 1/2
I_TXA: T("I_TXA"); {
  if (P&P_M) {
    rA = (rA&0xff00)+(X&0xff); TESTB_2(rA);
  } else {
    rA = X; TESTW_2(rA);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// 8B : PHB s 1/3
I_PHB: T("I_PHB"); {
  pushb(DB);
  /*Cycles+=3;*/
  goto loop_back; }

// 90 : BCC r 2/2
I_BCC: T("I_BCC"); {
  if (!F_C) { do_branch(); goto loop_back; }
  PCptr++; /*Cycles += 2;*/
  goto loop_back; }

// B0 : BCS r 2/2
I_BCS: T("I_BCS"); {
  if (F_C) { do_branch(); goto loop_back; }
  PCptr++; /*Cycles += 2;*/
  goto loop_back; }

// 98 : TYA i 1/2
I_TYA: T("I_TYA"); {
  if (P&P_M) {
    rA = (rA&0xff00)|(Y&0xff); TESTB_2(rA);
  } else {
    rA = Y; TESTW_2(rA);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// 9A : TXS i 1/2
I_TXS: T("I_TXS"); {
  if (P&P_E) 
     S = (S&0xFF00) + (X&0xff);  // OPT S= 100+ ??? 
  else 
    S = X;
  /*Cycles+=2;*/
  goto loop_back; }

// BA : TSX i 1/2
I_TSX: T("I_TSX"); {
  if (P&P_X) {
    X = (X&0xff00)|(S&0xff); TESTB_2(X);
  } else {
    X = S; TESTW_2(X);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// A8 : TAY i 1/2
I_TAY: T("I_TAY"); {
  if (P&P_X) {
    Y = (Y&0xff00)|(rA&0xff); TESTB_2(Y);
  } else {
    Y = rA; TESTW_2(Y);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// 9B : TXY i 1/2
I_TXY: T("I_TXY"); {
  if (P&P_X) {
    Y = (Y&0xff00)|(X&0xff); TESTB_2(Y);
  } else {
    Y = X; TESTW_2(Y);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// BB : TYX i 1/2
I_TYX: T("I_TYX"); {
  if (P&P_X) {
    X = (X&0xff00)|(Y&0xff); TESTB_2(X);
  } else {
    X = Y; TESTW_2(X);
  }
  /*Cycles+=2;*/
  goto loop_back; }


// B8 : CLV i 1/2
I_CLV: T("I_CLV"); {
  F_V = 0;
  /*Cycles+=2;*/
  goto loop_back; }

// AA : TAX i 1/2
I_TAX: T("I_TAX"); {
  if (P&P_X) {
    X = (X&0xff00)|(rA&0xff); TESTB_2(X);
  } else {
    X = rA; TESTW_2(X);
  }
  /*Cycles+=2;*/
  goto loop_back; }

// AB : PLB s 1/4
I_PLB: T("I_PLB"); {
  DB = pullb();
  TESTB_2(DB);
  /*Cycles+=4;*/
  goto loop_back; }

// F4 : PEA s 3/5
I_PEA: T("I_PEA"); {
  pushw(FETCHW);
  PCptr+=2; /*Cycles+=5;*/
  goto loop_back; }

// F4 : PEA s 3/5
I_PEI: T("I_PEI"); {
  pushw(mem_getword(D+FETCHB, 0));
  PCptr++; /*Cycles+=3;*/
  goto loop_back; }

// 62 : PER s 3/6
I_PER: T("I_PER"); {
  pushw(REAL_PC+(short)FETCHW+2);
  PCptr+=2; /*Cycles+=6;*/
  goto loop_back; }

// 0B : PHD s 1/3
I_PHD: T("I_PHD"); {
  pushw(D);
  /*Cycles+=3;*/
  goto loop_back; }

// 2B : PLD s 1/5
I_PLD: T("I_PLD"); {
  D = pullw();
  TESTW_2(D);
  /*Cycles+=5;*/
  goto loop_back; }

// 58 : CLI i 1/2
I_CLI: T("I_CLI"); {
  P &= ~P_I;
  /*Cycles+=2;*/
  goto loop_back; }

// 5A : PHY i 1/3
I_PHY: T("I_PHY"); {
  if (P&P_X)
	pushb(Y&0xFF);
  else
    pushw(Y);
  /*Cycles+=3;*/
  goto loop_back; }

// 54 : MVN xya 3/7
I_MVN: T("I_MVN"); {
  uint8 SB;
  SB = FETCHB1;
  DB = FETCHB;
#if 1  
  MVN(SB);
#else
  Cycles+=7*rA;
  do {
     mem_setbyte(Y, DB, mem_getbyte(X, SB));
     X++; Y++; rA--;
  } while (rA != 0xffff);
#endif  
  PCptr+=2;
  goto loop_back; }

// ?? : MVP xya ?
I_MVP: T("I_MVP"); {
  uint8 SB;
  SB = FETCHB1;
  DB = FETCHB;
#if 1  
  MVP(SB);
#else
  Cycles+=7*rA;
  do {
     mem_setbyte(Y, DB, mem_getbyte(X, SB));
     X--; Y--; rA--;
  } while (rA != 0xffff);
#endif  	
  PCptr+=2;
  goto loop_back; }


// CB : WAI i 1/3
I_WAI: T("I_WAI"); {
  // FIXME: There is something to do about it
  CPU.WAI_state = 1;
  PCptr--;
  /*Cycles+=3;*/
  goto loop_back; }

}

void CPU_pack()
{
  CPU.A  = A;  CPU.X  = X;  CPU.Y  = Y;
  CPU.S  = S;  CPU.P  = P;  CPU.D  = D;
  CPU.PB = PB; CPU.DB = DB; CPU.PC = PC;	
}

void CPU_unpack()
{
	sint8 buf[200];
  A = CPU.A;  X = CPU.X;  Y = CPU.Y;
  S = CPU.S;  P = CPU.P;  D = CPU.D;
  PB = CPU.PB; DB = CPU.DB; PC = CPU.PC;	
  
  
     sprintf(buf,
          "A:%04X X:%04X Y:%04X S:%04X D:%02X/%04X VC:%03d ?:%02d %d%d%d%d%d%d%d%d %02X:%04X\n",
          CPU.A, CPU.X, CPU.Y, CPU.S, CPU.DB, CPU.D, SNES.V_Count, Cycles,
          (P>>7)&1,(P>>6)&1,(P>>5)&1,(P>>4)&1,(P/8)&1,(P/4)&1,(P/2)&1,P&1,
          CPU.PB, CPU.PC);
          printf("%s\n", buf);
}
#endif

/*
uint8 IORead8(uint32 addr)
{
	uint32 address = addr & 0xFFFFFF;
	addr &= 0xFF000000;
	return IO_getbyte((int)addr, address);
}

void IOWrite8(uint32 addr, uint8 byte)
{
	uint32 address = addr & 0xFFFFFF;
	addr &= 0xFF000000;
	IO_setbyte((int)addr, address, byte);
}

uint16 IORead16(uint32 addr)
{
	uint32 address = addr & 0xFFFFFF;
	addr &= 0xFF000000;
	return IO_getword((int)addr, address);
}

void IOWrite16(uint32 addr, uint16 word)
{
	uint32 address = addr & 0xFFFFFF;
	addr &= 0xFF000000;
	return IO_setword((int)addr, address, word);
}
*/

#ifdef ASM_OPCODES

#include "opc_asm.h"
void CPU_pack()
{
  if (CPU.packed)
  	return;
  CPU.PC = (uint32)((sint32)PCptr+(sint32)SnesPCOffset); 
  CPU.PB = S&0xFFFF;
  
  CPU.A = REAL_A;
  CPU.X = X;
  CPU.Y = Y;
  Cycles = -((sint32)SaveR8 >> 14);
  
  CPU.S = S >> 16;
  CPU.P = 0; 
  if (SaveR8 & 0x00000002) CPU.P |= P_C;
  if (SaveR8 & 0x00000001) CPU.P |= P_V;
  if (SaveR8 & 0x00000400) CPU.P |= P_E;    
  if (SaveR6 & 0x00018000) CPU.P |= P_N;
  if (!(SaveR6 << 16)) CPU.P |= P_Z;
  CPU.P |= ((SaveR8 << 22) & 0x3c000000) >> 24;
  CPU.D = D >> 16;
  CPU.DB = D & 0xFF;
  
  CPU.WAI_state = (SaveR8 & 0x00001000)?1:0;  

  CPU.packed = 1;
}

void CPU_unpack()
{
  if (CPU.unpacked)
  	return;	
	
  SnesPCOffset = -(sint32)mem_getbaseaddress(CPU.PC, CPU.PB);
  PCptr = map_memory(CPU.PC, CPU.PB);
	
  S = CPU.PB;
  S |= CPU.S << 16;
	
  // FIXME: "B" register
  if (CPU.P & P_M)
  {
	A = CPU.A << 24;
	SnesB = (CPU.A & 0xFF00) << 16;
  }
  else
	A = CPU.A << 16;
		   
  X = CPU.X;
  Y = CPU.Y;
  
  SaveR8 = SaveR6 = 0;
  if (CPU.P & P_C) SaveR8 |= 0x00000002;
  if (CPU.P & P_V) SaveR8 |= 0x00000001;
  if (CPU.P & P_N) SaveR6 |= 0x00018000;
  if (CPU.P & P_E) SaveR8 |= 0x00000400;   
  
  if (!(CPU.P & P_Z)) SaveR6 |= 0x00000001;
  SaveR8 |= ((CPU.P << 24) & 0x3c000000) >> 22; 
  
  D = CPU.DB;  
  D |= CPU.D << 16;
  
  CPU.unpacked = 1;
  CPU_update();
}

void	pushb(uint8 b)
{
	SNESC.RAM[CPU.S] = b;
	CPU.S--;
}

void pushw(uint16 w)
{
	CPU.S--;
	SET_WORD16(SNESC.RAM+CPU.S, w);
	CPU.S--;
}

uint8	pullb()
{
	CPU.S++;
	return SNESC.RAM[CPU.S];
}

uint16	pullw()
{
	uint16 w;
	
	CPU.S++;	
	w = GET_WORD16(SNESC.RAM+CPU.S);
	CPU.S++;
	return w;
}


void CPU_goto(int cycles)
{	
	if (CFG.CPU_speedhack & 1)
		cycles -= cycles / 4; // Speed hack: 25 % speed up
	CPU_LoopSpeedHacks = (CFG.CPU_speedhack >= 2);
	CPU.Cycles = cycles;
	
	CPU_unpack();
	
//ori removed:	*APU_ADDR_BLK = 0;
	CPU_goto2(cycles);
//ori removed:	*APU_ADDR_BLK = 1;
	CPU.packed = 0;

//	CPU_pack();
}


#endif
