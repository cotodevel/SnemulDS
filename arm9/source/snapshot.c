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
#include <stdio.h>

#include "snapshot.h"
#include "common.h"
#include "opcodes.h"

#include "main.h"
#include "specific_shared.h"
#include "fs.h"

#include "apu.h"
#include "gfx.h"
#include "cfg.h"
#include "core.h"
#include "engine.h"

#include "posixHandleTGDS.h"
#include "fsfatlayerTGDS.h"
#include "utilsTGDS.h"


int		get_snapshot_name(sint8 *file, uchar nb, sint8 *name)
{
  FILE *f;
  //FIL fhandler;

  if (nb > 8) return 0;
  file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....
  FS_lock();
  if ((f = fopen(file, "r")) == NULL){
  //if(!(f_open(&fhandler,file,FA_READ) == FR_OK))
  
	  FS_unlock();
	  return 0;
  }

  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  fread(header, sizeof(TSnapShot_Header), 1, f);
	
	//unsigned int read_so_far;
	//f_read(&fhandler, header, sizeof(TSnapShot_Header), &read_so_far);
	
  sint8 header_name[17];
  memcpy(header_name, header->name, 16);
  header_name[16] = 0;
  sprintf(name, "Save #%d - %s", nb, header_name);

  free(header);
  fclose(f);
  //f_close(&fhandler);
  FS_unlock();
  return 1;
}

int	read_snapshot(sint8 *file, uchar nb)
{
  FILE *f;
  //FIL fhandler;
  int	i;

  if (nb > 8)
  	 return 0;
  file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....
  FS_lock();
  if ((f = fopen(file, "r")) == NULL){
	//if(!(f_open(&fhandler,file,FA_READ) == FR_OK))
	 FS_unlock();
  	 return 0;
  }

  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  
  
	
  fread(header, sizeof(TSnapShot_Header), 1, f);
	//unsigned int read_so_far;
	//f_read(&fhandler, header, sizeof(TSnapShot_Header), &read_so_far);
	
  fread(SNESC.RAM,  0x20000, 1, f);
	//f_read(&fhandler, SNESC.RAM, 0x20000, &read_so_far);
	
  fread(SNESC.VRAM, 0x10000, 1, f);
	//f_read(&fhandler, SNESC.VRAM, 0x10000, &read_so_far);
	
  fread(SNESC.SRAM, 0x8000, 1, f);
	//f_read(&fhandler, SNESC.SRAM, 0x8000, &read_so_far);
	
  for (i = 0; i < 256; i++)
  {
  	uint8	pal[3];
    fread(pal, 3, 1, f);
	//f_read(&fhandler, pal, 3, &read_so_far);
	
	GFX.SNESPal[i] = (pal[2]>>1)|((pal[1]>>1)<<5)|((pal[0]>>1)<<10);
  }

/*  fread(CPU.PPU_PORT, 2*0x100, 1, f);
  fread(CPU.DMA_PORT, 2*0x200, 1, f);*/

  fread(CPU.PPU_PORT, 2*0x90, 1, f);
  //f_read_fs(&fhandler, CPU.PPU_PORT, 2*0x90, &read_so_far);
	
  fread(CPU.DMA_PORT, 2*0x180, 1, f);
  //f_read_fs(&fhandler, CPU.DMA_PORT, 2*0x180, &read_so_far);
  
  TSnapShot *snapshot = (TSnapShot *)malloc(sizeof(TSnapShot));
  fread(snapshot,  sizeof(TSnapShot), 1, f);
  //f_read_fs(&fhandler, snapshot, sizeof(TSnapShot), &read_so_far);
  
  //printf("PC =  %02X:%04x\n", snapshot->PB, snapshot->PC);

  CPU.A  = snapshot->A;  CPU.X  = snapshot->X;  CPU.Y  = snapshot->Y;
  CPU.S  = snapshot->S;  CPU.P  = snapshot->P;  CPU.D  = snapshot->D;
  CPU.PB = snapshot->PB; CPU.DB = snapshot->DB; CPU.PC = snapshot->PC;
  GFX.BG_scroll_reg = snapshot->BG_scroll_reg;
  GFX.SC_incr = snapshot->SC_incr; GFX.FS_incr = snapshot->FS_incr;
  GFX.OAM_upper_byte = snapshot->OAM_upper_byte;
  SNES.PPU_NeedMultiply = snapshot->PPU_NeedMultiply;
  
  // FIXME: we should also save sprite infos
  
  if (snapshot->options[0] >= 1)
  {
  	unpackOptions(snapshot->options[0], &snapshot->options[1]); 	
  }

   if (CFG.Sound_output) {
	APU_stop(); // Make sure that the APU is *completely* stopped
   	// Read SPC file format
	fread(APU_RAM_ADDRESS, 1, 0x10200, f);
	//f_read(&fhandler, APU_RAM_ADDRESS, 0x10200, &read_so_far);
  
	APU_loadSpc(); 
  }

  free(snapshot);
  free(header);
  fclose(f);
  //f_close(&fhandler);
  FS_unlock();

  GFX.tiles_dirty = 1;
  GFX.Sprites_table_dirty = 1;
  CPU.unpacked = 0; // Update ASM  
  return 0;
}

int write_snapshot(sint8 *file, uint8 nb, const sint8 *name)
{
  FILE *f;
  //FIL fhandler;
  int	i;

  if (nb > 8) return 0;
  file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....
  
  FS_lock();
  
  if ((f = fopen(file, "w")) == NULL)
	//if(!(f_open(&fhandler,file,FA_WRITE | FA_OPEN_ALWAYS) == FR_OK))
  
	/*
	--both removed originally
	if ((f = fopen(file, "w")) == NULL)
	  	if ((f = fopen(file, "w")) == NULL)
	*/
		
  {
	  FS_unlock();
  	  return 0;
  }
  
  TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
  strcpy(header->name, name);  
  
  fwrite(header, sizeof(TSnapShot_Header), 1, f);
	//unsigned int written;
	//f_write(&fhandler, header, sizeof(TSnapShot_Header), &written);
	
  fwrite(SNESC.RAM,  0x20000, 1, f);
	//f_write(&fhandler, SNESC.RAM, 0x20000, &written);
	
  fwrite(SNESC.VRAM, 0x10000, 1, f);
	//f_write(&fhandler, SNESC.VRAM, 0x10000, &written);
	
  fwrite(SNESC.SRAM, 0x8000, 1, f);
	//f_write(&fhandler, SNESC.SRAM, 0x8000, &written);
	
  for (i = 0; i < 256; i++)
  {
  	uint8	pal[3];
  	pal[2] = GFX.SNESPal[i]<<1;
  	pal[1] = (GFX.SNESPal[i]>>5)<<1;
  	pal[0] = (GFX.SNESPal[i]>>10)<<1;  	
    fwrite(pal, 3, 1, f);
	//f_write(&fhandler, pal, 3, &written);
	
  }
    
  fwrite(CPU.PPU_PORT,  2*0x90, 1, f);
  //f_write(&fhandler, CPU.PPU_PORT, 2*0x90, &written);
	
  fwrite(CPU.DMA_PORT,  2*0x180, 1, f);
  //f_write(&fhandler, CPU.DMA_PORT, 2*0x180, &written);
  
  TSnapShot *snapshot = (TSnapShot *)malloc(sizeof(TSnapShot));

  snapshot->A = CPU.A; snapshot->X = CPU.X; snapshot->Y = CPU.Y;
  snapshot->S = CPU.S; snapshot->P = CPU.P; snapshot->D = CPU.D;
  snapshot->PB = CPU.PB; snapshot->DB = CPU.DB; snapshot->PC = CPU.PC;

  snapshot->BG_scroll_reg = GFX.BG_scroll_reg;
  snapshot->SC_incr = GFX.SC_incr; snapshot->FS_incr = GFX.FS_incr;
  snapshot->OAM_upper_byte = GFX.OAM_upper_byte;
  snapshot->PPU_NeedMultiply = SNES.PPU_NeedMultiply;
  
  snapshot->options[0] = 2;
//	printf("\nUpdate Options\n");
  packOptions(&snapshot->options[1]);

  //GUI_console_printf(0, 23, "State written");
  
  fwrite(snapshot,  sizeof(TSnapShot), 1, f);
  //f_write(&fhandler, snapshot, sizeof(TSnapShot), &written);
  
  if (CFG.Sound_output) {
  	APU_stop(); // Make sure that the APU is *completely* stopped
	APU_saveSpc(); 
   	
	fwrite(APU_RAM_ADDRESS, 1, 0x10200, f);
	//f_write(&fhandler, APU_RAM_ADDRESS, 0x10200, &written);
  }
  
  free(snapshot);
  free(header);
  fclose(f);
  //f_close(&fhandler);
  FS_unlock();
  return (1);
}
