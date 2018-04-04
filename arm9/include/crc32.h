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

#ifndef __crc32_h__
#define __crc32_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#define CRC16_POLYNOMIAL 0xa001
#define CRC32_POLYNOMIAL 0xedb88320


#endif


#ifdef __cplusplus
extern "C" {
#endif

extern void init_crc_table (void *table, unsigned int polynomial);
extern unsigned int *crc32_table;
extern void free_crc32_table (void);
extern unsigned int crc32 (unsigned int crc, const void *buffer, unsigned int size);

#ifdef __cplusplus
}
#endif
