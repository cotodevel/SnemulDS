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

#ifndef __snapshot_h__
#define __snapshot_h__

#include "guiTGDS.h"

typedef struct {
	sint8 name[16];
}TSnapShot_Header;

typedef struct {
	int            A, X, Y, S, P, D, PB, DB, PC;
	
	uint8  BG_scroll_reg;
	uint8  PPU_NeedMultiply;
	uint8	 options[5];
	//uint8 HI_1C, HI_1D, HI_1E, HI_1F, HI_20;
	uint8  SC_incr, FS_incr, OAM_upper_byte;
} TSnapShot;

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern int	get_snapshot_name(sint8 *file, uchar nb, sint8 *name);
extern int	read_snapshot(sint8 *file, uchar nb);
extern int	write_snapshot(sint8 *file, uint8 nb, const sint8 *name);

#ifdef __cplusplus
}
#endif
