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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include "specific_shared.h"
#include "devoptab_devices.h"
#include "posixHandleTGDS.h"
#include "utilsTGDS.h"

#include "fs.h"
#include "guiTGDS.h"
#include "nds_cp15_misc.h"

#include <unistd.h>
#include <sys/dir.h>
#include <fcntl.h>
#include "fsfatlayerTGDSLegacy.h"
#include "fileHandleTGDS.h"

#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "ram.h"
#include "conf.h"
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
#include "about.h"
#include "fileHandleTGDS.h"
#include "xenofunzip.h"



/* *********************** FAT ************************ */

sint8 logbuf[32000];
static int logcnt = 0;

void	FS_printlogBufOnly(sint8 *buf)
{
	if (logcnt == 0)
	{
		// first call
		strcpy(logbuf, buf);
		logcnt = 1;
		return;
	}	
	if( strlen(buf)+strlen(logbuf) >= 32000)
	{
		strcpy(logbuf, buf);
		logcnt++;
	}
	else
		strcat(logbuf, buf);
}

void	FS_printlog(sint8 *buf)
{
 
	static FILE *f_log = NULL;
	if (logcnt == 0)
	{
		// first call
		strcpy(logbuf, buf);
		logcnt = 1;
		return;
	}	
	if( strlen(buf)+strlen(logbuf) >= 32000)
	{
		// Flush buffer
		sint8 name[30];
		sprintf(name,"snemul%d.log", logcnt%100);
		
		FS_lock();
		
		f_log = fopen(name, "w");
		if(f_log){
			//FILE handle ok
		}
		else{
			//FILE handle error
		}
		fwrite(logbuf, 1, strlen(logbuf), f_log);
		fclose(f_log);
		FS_unlock();
		
		strcpy(logbuf, buf);
		logcnt++;
	}
	else{
		strcat(logbuf, buf);
	}
	
}

//printf that instead stores on Filesystem
void	FS_flog(sint8 *fmt, ...)
{
		
	va_list ap;	

    va_start(ap, fmt);
    vsnprintf((sint8*)g_printfbuf, 100, fmt, ap);
    va_end(ap);

	FS_printlog((sint8*)g_printfbuf);
}

int	FS_loadROM(sint8 *ROM, sint8 *filename)
{
	FILE	*f;
	FS_lock();
	f = fopen(filename, "r");
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);

	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)ROM, (int)size);
	
	fread(ROM, 1, size, f);
	GUI_printf("Read done\n");
	fclose(f);
	FS_unlock();

	return 1;
}


FILE * fPaging ;

int	FS_loadROMForPaging(sint8 *ROM, sint8 *filename, int size)
{
	FS_lock();
	
	if (fPaging){
		fclose(fPaging);
	}
	fPaging = fopen(filename, "r");
	sint32 fd = fileno(fPaging);
	if(fd < 0){
		FS_unlock();
		return -1;
	}
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)ROM, (int)size);
	
	int ret = fread(ROM, 1, size, fPaging);
	FS_unlock();
	
	if(ret != size){
		return -1;
	}
	
	return 0;
}


int	FS_loadROMPage(sint8 *buf, unsigned int pos, int size){
	
	int ret;	
	FS_lock();
	
	if(!fPaging){
		return -1;
	}
	
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)buf, (int)size);
	
	ret = fseek(fPaging, pos, SEEK_SET);
	
	if (ret < 0)
	{
		FS_unlock();
		return -1;
	}
	
	
	return fread(buf, 1, size, fPaging);
}

int	FS_shouldFreeROM()
{
	return 1;
}
