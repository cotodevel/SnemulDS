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

#ifndef __frontend_h_
#define __frontend_h_

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif


extern sint8* myfgets(sint8 *buf,int n,FILE *fp);
//sint8* myfgets(sint8 *buf,int n,FIL *fp);

extern void SplitItemFromFullPathAlias(const sint8 *pFullPathAlias,sint8 *pPathAlias,sint8 *pFilenameAlias);
extern bool _readFrontend(sint8 *target);
extern bool readFrontend(sint8 **_name, sint8 **_dir);

#ifdef __cplusplus
}
#endif
