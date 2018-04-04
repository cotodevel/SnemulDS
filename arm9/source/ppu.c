///***********************************************************/
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

#include "ppu.h"

#include <string.h>
#include "common.h"
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "consoleTGDS.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "opcodes.h"
#include "core.h"
#include "guiTGDS.h"
#include "dmaTGDS.h"

//snemulds
__attribute__((section(".dtcm")))
uint32 bittab[256]={0};
__attribute__((section(".dtcm")))
uint32 bittab8[16]={0};

void DRAW_PLANE(uint8 BG,uint8  BG_MODE){
	switch(CPU.PPU_PORT[0x07+BG]&3) {
		case 0: { draw_plane_32_30(BG, BG_MODE); } break; 
		case 1: { draw_plane_64_30(BG, BG_MODE); } break; 
		case 2: { draw_plane_32_60(BG, BG_MODE); } break; 
		case 3: { draw_plane_64_60(BG, BG_MODE); } break;
	}
}
 
void    init_render()
{

	int  PixelOdd = 1; //extrapolar pixel
	int   h;
	int   i;

// 2/4 bits in planar mode to 4 bits converter 
    for (i = 0; i < 256; i++)
    {
      h = 0;
      if (i & 128) h |= PixelOdd;
      if (i & 64) h |= PixelOdd << 4;
      if (i & 32) h |= PixelOdd << 8;
      if (i & 16) h |= PixelOdd << 12; 
      
      if (i & 8) h |= PixelOdd << 16;
      if (i & 4) h |= PixelOdd << 20;
      if (i & 2) h |= PixelOdd << 24;
      if (i & 1) h |= PixelOdd << 28;
	  bittab[i] = h;
    }
    
// 8 bits in planar mode to 8 bits converter 
  for (i = 0;i < 16; i++)
    {
      h = 0;
      if (i & 8) h |= PixelOdd;
      if (i & 4) h |= PixelOdd << 8;
      if (i & 2) h |= PixelOdd << 16;
      if (i & 1) h |= PixelOdd << 24;
      bittab8[i] = h;
    }    

}

void check_sprite_addr()
{ 	
	if (GFX.spr_addr_base+GFX.spr_addr_select == GFX.spr_addr[0])
	{
		GFX.spr_bank = 0;
		GFX.spr_addr_vcount[GFX.spr_bank] = SNES.V_Count;
	} else
		if (GFX.spr_addr_base+GFX.spr_addr_select == GFX.spr_addr[1])
		{
			GFX.spr_bank = 1;
			GFX.spr_addr_vcount[GFX.spr_bank] = SNES.V_Count;
		} 
		else
		{
			GFX.spr_bank ^= 1;
			GFX.spr_addr[GFX.spr_bank] = GFX.spr_addr_base+GFX.spr_addr_select;
			GFX.spr_addr_vcount[GFX.spr_bank] = SNES.V_Count;		
		}
	//LOG("(%03d)%04x+%04x -> %d\n", SNES.V_Count, GFX.spr_addr_base, GFX.spr_addr_select, GFX.spr_bank);	
}

void check_tile_addr()
{
//	GFX.tiles_dirty = 1;	
}

int			NeedUpdate2b;
int			NeedUpdate4b;
int			NeedFlush2b;
int			NeedFlush4b;

/*
 * First 64 ko are for MAP
 * 192 ko are allocatable (192 / 32ko = 6)
 */
__attribute__((section(".dtcm")))
t_TileZone	TileZones[8];
__attribute__((section(".dtcm")))
uint32		Mode7TileZone;

// Each tile block of SNES RAM (64 / 8ko = 8) 
// points to three DS Tile Zone, one for each depth
 
t_TileZone	*SNESToDS_TileAddress[8*4];

__attribute__((section(".dtcm")))
uint16		ToUpdate2b[100];
__attribute__((section(".dtcm")))
uint16		ToUpdate4b[100];

int	PPU_get_bgmode(int mode, int bg)
{
  if (bg == 0)
  {
  	if (mode == 1 || mode == 2 || mode == 5 || mode == 6)
  		return 4;
  	else if (mode == 0)
  		return 2;
  	else
  		return 8;
  }
  else if (bg == 1)
  {
  	if (mode == 1 || mode == 2 || mode == 3)
  		return 4;
  	else if (mode == 2 || mode == 4  || mode == 5)
  		return 2;
  	else
  		return 0;
  }
  else if (bg == 2)
  {
  	if (mode <= 1)
  		return 2;
  	else
  		return 0;
  }	
  return 0;
}

int	PPU_get_tile_address(int tile_address, int bg_mode)
{
	int	i;
	for (i = 2; i < 8; i++)
	{
		if (TileZones[i].base == tile_address &&
			TileZones[i].depth == bg_mode)
		{
			return i;
		}
	} 
	return -1;
}

/*
int	PPU_find_tile_address(int tile_address, int bg_mode)
{
	int	i;
	for (i = 2; i < 8; i++)
	{
		if (TileZones[i].depth == bg_mode &&
		    tile_address >= (TileZones[i].base<<13) && 
			tile_address <  (TileZones[i].base<<13)+bg_mode*8192)
		{
			return i;
		}
	} 
	return -1;
}
*/

int	PPU_allocate_tilezone()
{
	int	i;
	int	less_used = 2;
	
	for (i = 2; i < 8; i++)
	{
		if (TileZones[i].used < TileZones[less_used].used)
		{
			less_used = i;
		}
	}	
	return less_used; 	
}

int	PPU_get_most_used()
{
	int	i;
	int	most_used = 0;
	
	for (i = 2; i < 8; i++)
	{
		if (TileZones[i].used > most_used)
		{
			most_used = TileZones[i].used;
		}
	}	
	return most_used; 	
}


int	PPU_allocate_tilezone2()
{
	int	i;
	int	less_used = 2;
	
	for (i = 2; i < 7; i++)
	{
		if (TileZones[i].used+TileZones[i+1].used < 
			TileZones[less_used].used+TileZones[less_used+1].used)
		{
			less_used = i;
		}
	}	
	return less_used; 	
}

void	PPU_add_tile_address(int bg)
{
  int 		tile_zone = (GFX.tile_address[bg]>>13);
  int		ds_tile_zone;
  int		mode = CPU.PPU_PORT[0x05] & 7;
  int		bg_mode;
  int		i;
   	
  bg_mode = PPU_get_bgmode(mode, bg);
  
  if (!bg_mode || !(CFG.BG_Layer & (1 << bg)))
  	return;  
  ds_tile_zone = PPU_get_tile_address(GFX.tile_address[bg], bg_mode);
  if (ds_tile_zone == -1)
  { 
	  //LOG("PPU_add_tile_address : %d %d %d\n", tile_zone, bg, bg_mode);
	 
	  // FIXME : 256 colors should allocate two tile zones
	  if (bg_mode == 8 && mode != 7)
	  	ds_tile_zone = PPU_allocate_tilezone2();
	  else
	  	ds_tile_zone = PPU_allocate_tilezone();
	  //LOG("Allocated tile zone : %d\n", ds_tile_zone);
	  
	  // Clear previous linked zone
	  for (i = 0; i < 4*8; i++)
	  	if (SNESToDS_TileAddress[i] == &TileZones[ds_tile_zone]) 
	  	{
	  		SNESToDS_TileAddress[i] = NULL;
	  	}
	  	
	  int most_used = PPU_get_most_used();
	 
	  TileZones[ds_tile_zone].base = GFX.tile_address[bg];
	  TileZones[ds_tile_zone].depth = bg_mode;
	  TileZones[ds_tile_zone].used = most_used;
	  TileZones[ds_tile_zone].DSVRAMAddress = (uint16 *)(0x06020000+0x8000*ds_tile_zone);
	  if (bg_mode == 8 && mode != 7)
	  {
	  	
	  for (i = 0; i < 4*8; i++)
	  	if (SNESToDS_TileAddress[i] == &TileZones[ds_tile_zone+1]) 
	  	{
	  		SNESToDS_TileAddress[i] = NULL;
	  	}
	 
	  TileZones[ds_tile_zone+1].base = GFX.tile_address[bg];
	  TileZones[ds_tile_zone+1].depth = bg_mode;
	  TileZones[ds_tile_zone+1].used = most_used;
	  TileZones[ds_tile_zone+1].DSVRAMAddress = (uint16 *)(0x06020000+0x8000*ds_tile_zone);
	  	  	
	  }
  }
  
  TileZones[ds_tile_zone].used++;
  if (bg_mode == 8 && mode != 7)
  	TileZones[ds_tile_zone+1].used++;
  GFX.tile_slot[bg] = ds_tile_zone*2;  
//  LOG("TileSlot[%d] = %d (l=%d)\n", bg, ds_tile_zone, SNES.V_Count);

  if (mode == 7)
  	return;
  
  switch (bg_mode) {
  	case 2 :
  		SNESToDS_TileAddress[(tile_zone*4)+bg] = &TileZones[ds_tile_zone];
  		if (tile_zone+1 < 8)
			SNESToDS_TileAddress[((tile_zone+1)*4)+bg] = &TileZones[ds_tile_zone];
  		break;
  	case 4 :
  		SNESToDS_TileAddress[(tile_zone*4)+bg] = &TileZones[ds_tile_zone];
  		if (tile_zone+1 < 8)
			SNESToDS_TileAddress[((tile_zone+1)*4)+bg] = &TileZones[ds_tile_zone];
  		if (tile_zone+2 < 8)
			SNESToDS_TileAddress[((tile_zone+2)*4)+bg] = &TileZones[ds_tile_zone];
  		if (tile_zone+3 < 8)
			SNESToDS_TileAddress[((tile_zone+3)*4)+bg] = &TileZones[ds_tile_zone];
  		break;
  	case 8 :
			SNESToDS_TileAddress[(tile_zone*4)+bg] = &TileZones[ds_tile_zone];
  		if (tile_zone+1 < 8)
			SNESToDS_TileAddress[((tile_zone+1)*4)+bg] = &TileZones[ds_tile_zone];
  		if (tile_zone+2 < 8)
			SNESToDS_TileAddress[((tile_zone+2)*4)+bg] = &TileZones[ds_tile_zone];
  		if (tile_zone+3 < 8)
			SNESToDS_TileAddress[((tile_zone+3)*4)+bg] = &TileZones[ds_tile_zone];
  		if (tile_zone+4 < 8 && mode != 7)
			SNESToDS_TileAddress[((tile_zone+4)*4)+bg] = &TileZones[ds_tile_zone];
  		if (tile_zone+5 < 8 && mode != 7)
			SNESToDS_TileAddress[((tile_zone+5)*4)+bg] = &TileZones[ds_tile_zone];
  		if (tile_zone+6 < 8 && mode != 7)
			SNESToDS_TileAddress[((tile_zone+6)*4)+bg] = &TileZones[ds_tile_zone];
  		if (tile_zone+7 < 8 && mode != 7)
			SNESToDS_TileAddress[((tile_zone+7)*4)+bg] = &TileZones[ds_tile_zone];
  		break;
  }
}

void     add_tile_2(int tile_addr_base, uint16 *vram_addr, int tilenb)
{
  int		k;
  uint8		*tile_ptr;
  uint32	tile_addr;
  uint32	*VRAM_ptr;

  tile_addr = tile_addr_base+tilenb*16;
  //VRAM_ptr = ((uint16 *)0x06010000)+tile_addr_base+tilenb*16;
  VRAM_ptr = (uint32 *)(vram_addr+(tilenb*16));
  tile_ptr = SNESC.VRAM+tile_addr;    
 
for (k=0;k<8;k++,tile_ptr+=2)
{
	//*VRAM_ptr++=(uint32)((bittab[tile_ptr[0x00]]) | (bittab[tile_ptr[0x01]]<<1));
	dmaFillWord(3,
		(uint32)((bittab[tile_ptr[0x00]]) | (bittab[tile_ptr[0x01]]<<1)),
		(uint32)  	VRAM_ptr,
		4
	);
	VRAM_ptr++;
}

GFX.tiles2b_def[tile_addr/16] = 2;
}

void     add_tile_4(int tile_addr_base, uint16 *vram_addr, int tilenb)
{
  int		k;
  uint8		*tile_ptr;
  uint32	tile_addr;  
  uint32	*VRAM_ptr;

  tile_addr = tile_addr_base+tilenb*32;
  tile_ptr = SNESC.VRAM+tile_addr;    
  VRAM_ptr = (uint32 *)(vram_addr+tilenb*16);
  
	//works
	for (k=0;k<8;k++,tile_ptr+=2)
    {
		dmaFillWord(3,
			(uint32)( (bittab[tile_ptr[0x00]]) | (bittab[tile_ptr[0x01]]<<1) | (bittab[tile_ptr[0x10]]<<2) | (bittab[tile_ptr[0x11]]<<3) ),
			(uint32)  	VRAM_ptr,
			4
		);
		VRAM_ptr++;
	}
    GFX.tiles4b_def[tile_addr/32] = 4;
}

void     add_tile_8(int tile_addr_base, uint16 *vram_addr, int tilenb)
{
  int		a;
  int		k;
  uint32	c1,c2;
  uint8		*tile_ptr;
  uint32	tile_addr;  
  uint32	*VRAM_ptr;

  tile_addr = tile_addr_base+tilenb*64;
  tile_ptr = SNESC.VRAM+tile_addr;    
  //VRAM_ptr = ((uint16 *)0x06010000)+tile_addr_base+tilenb*32;
  VRAM_ptr = (uint32 *)(vram_addr+tilenb*32);    

  for (k=0;k<8;k++,tile_ptr+=2)
    {
      c1 = c2 = 0;
      if ((a = tile_ptr[0x00]))
        {
          c1 |= bittab8[(a>>4)];
          c2 |= bittab8[(a&0xf)];
        }
      if ((a = tile_ptr[0x01]))
        {
          c1 |= bittab8[(a>>4)]<<1;
          c2 |= bittab8[(a&0xf)]<<1;
        }
      if ((a = tile_ptr[0x10]))
        {
          c1 |= bittab8[(a>>4)]<<2;
          c2 |= bittab8[(a&0xf)]<<2;
        }
      if ((a = tile_ptr[0x11]))
        {
          c1 |= bittab8[(a>>4)]<<3;
          c2 |= bittab8[(a&0xf)]<<3;
        }
      if ((a = tile_ptr[0x20]))
        {
          c1 |= bittab8[(a>>4)]<<4;
          c2 |= bittab8[(a&0xf)]<<4;
        }
      if ((a = tile_ptr[0x21]))
        {
          c1 |= bittab8[(a>>4)]<<5;
          c2 |= bittab8[(a&0xf)]<<5;
        }
      if ((a = tile_ptr[0x30]))
        {
          c1 |= bittab8[(a>>4)]<<6;
          c2 |= bittab8[(a&0xf)]<<6;
        }
      if ((a = tile_ptr[0x31]))
        {
          c1 |= bittab8[(a>>4)]<<7;
          c2 |= bittab8[(a&0xf)]<<7;
        }
		
		/*
		*VRAM_ptr++ = c1;      
		*VRAM_ptr++ = c2;
		*/
		
		dmaFillWord(3,
			(uint32)c1,
			(uint32)  	VRAM_ptr,
			4
		);
		VRAM_ptr++;
		dmaFillWord(3,
			(uint32)c2,
			(uint32)  	VRAM_ptr,
			4
		);
		VRAM_ptr++;
	}
  //GFX.tiles8b_def[tile_addr/64] = (tile_addr_base>>13)+1;
	GFX.tiles8b_def[tile_addr/64] = 8;
}


int		PPU_AddTile2InCache(t_TileZone *tilezone, int addr)
{	
  	if (!NeedFlush2b)
  	{  	
//  		LOG("*2:%p:%p:%d>\n", addr, tilezone->base, (addr-tilezone->base)/16);	
  	  	if (!(GFX.tiles2b_def[addr/16] & 0x80))
  	  	{
//  			LOG("<2:%p:%p:%d>\n", addr, tilezone->base, (addr-tilezone->base)/16);  	  		
  	  		ToUpdate2b[NeedUpdate2b] = addr/16;
  	  		GFX.tiles2b_def[addr/16] |= 0x80;
	  	  	if (++NeedUpdate2b >= 100)
	  	  	{
  		  		GFX.tiles_def[addr/8192] = 0; // Flush all
//  		  		LOG(">>>FLUSH\n");
  		  		NeedFlush2b = 1;
	  	  	}	
  	  	}
  	  	return 1;
  	}
  	return 0;
}

int		PPU_AddTile4InCache(t_TileZone *tilezone, int addr)
{	
  	if (!NeedFlush4b)
  	{
//  		LOG("<4:%p:%p:%d>\n", addr, TileZones[tilezone].base, (addr-TileZones[tilezone].base)/32);  		
  	  	//add_tile_4(TileZones[tilezone].base, (addr-TileZones[tilezone].base)/32);
  	  	if (!(GFX.tiles4b_def[addr/32] & 0x80))
  	  	{
//  			LOG("<4:%p:%p:%d>\n", addr, tilezone->base, (addr-tilezone->base)/32);  	  		
  	  		ToUpdate4b[NeedUpdate4b] = addr/32;
  	  		GFX.tiles4b_def[addr/32] |= 0x80;
	  	  	if (++NeedUpdate4b >= 100)
	  	  	{
  		  		GFX.tiles_def[addr/8192] = 0; // Flush all
//  		  		LOG(">>>FLUSH\n");
  		  		NeedFlush4b = 1;
	  	  	}
  	  	}
  	  	return 1; 	  	
  	}
  	return 0; 	
}


void check_tile()
{	
    int		addr = (CPU.PPU_PORT[0x16]<<1)&0xFFFF;

  GFX.map_def[addr/2048] = 0;
  // Check tile zone

#if 1
  if ((GFX.tiles_def[addr/8192]>>4) != 0)
  {
  	//int tilezone = (GFX.tiles_def[addr/8192]>>4)-1;
  	t_TileZone	*tilezone1;
  	t_TileZone	*tilezone2;
  	t_TileZone	*tilezone3;
  	int			leave = 0;
  	  	
  	tilezone1 = SNESToDS_TileAddress[((addr>>13)<<2)+0];  	
  	if (tilezone1)
  	{
  		if (tilezone1->depth == 4)
  			leave = PPU_AddTile4InCache(tilezone1, addr);
  			
  	}
  	tilezone2 = SNESToDS_TileAddress[((addr>>13)<<2)+1];
  	if (tilezone2 && tilezone2 != tilezone1)
  	{
  		if (tilezone2->depth == 2)
  			leave = PPU_AddTile2InCache(tilezone2, addr);
  		else
  		if (tilezone2->depth == 4)
  			leave = PPU_AddTile4InCache(tilezone2, addr);
  	}
  	tilezone3 = SNESToDS_TileAddress[((addr>>13)<<2)+2];
  	if (tilezone3 && tilezone3 != tilezone2)
  	{
  		if (tilezone3->depth == 2)
  			leave = PPU_AddTile2InCache(tilezone3, addr);  		
  	}
//  	LOG("%04x %p %p %p\n", addr, tilezone1, tilezone2, tilezone3);  	

  	// FIXME: 256 colors not handled here yet
  	if (leave)
  		return;  	  	
  }
#endif
  GFX.tiles2b_def[addr/16] = 0;
  GFX.tiles4b_def[addr/32] = 0;
  GFX.tiles8b_def[addr/64] = 0;
  GFX.tiles_def[addr/8192] = 0;
}

void	PPU_updateCache()
{
	int i;
	
	if (NeedFlush4b)
	{
		for (i = 0; i < NeedUpdate4b; i++)
		{
			GFX.tiles4b_def[ToUpdate4b[i]] = 0;
		}
		NeedFlush4b = NeedUpdate4b = 0;
	}
	
	if (NeedFlush2b)
	{
		for (i = 0; i < NeedUpdate2b; i++)
		{
			GFX.tiles2b_def[ToUpdate2b[i]] = 0;
		}
		NeedFlush2b = NeedUpdate2b = 0;
	}

  	t_TileZone	*tilezone1;
  	t_TileZone	*tilezone2;

	for (i = 0; i < NeedUpdate4b; i++)
	{
		int addr = ToUpdate4b[i]*32;
		tilezone1 = SNESToDS_TileAddress[((addr>>13)<<2)+0];
		if (tilezone1 && tilezone1->depth == 4 && (addr-tilezone1->base)/32 < 1024)
			add_tile_4(tilezone1->base, tilezone1->DSVRAMAddress, (addr-tilezone1->base)/32);
		tilezone2 = SNESToDS_TileAddress[((addr>>13)<<2)+1];
		if (tilezone1 != tilezone2 && 
		    tilezone2 && tilezone2->depth == 4 && (addr-tilezone2->base)/32 < 1024)
			add_tile_4(tilezone2->base, tilezone2->DSVRAMAddress, (addr-tilezone2->base)/32);
			
	}
	for (i = 0; i < NeedUpdate2b; i++)
	{
		int addr = ToUpdate2b[i]*16;
		tilezone1 = SNESToDS_TileAddress[((addr>>13)<<2)+1];
		if (tilezone1 && tilezone1->depth == 2 && (addr-tilezone1->base)/16 < 1024)
			add_tile_2(tilezone1->base, tilezone1->DSVRAMAddress, (addr-tilezone1->base)/16);
		tilezone2 = SNESToDS_TileAddress[((addr>>13)<<2)+2];
		if (tilezone1 != tilezone2 && 
		    tilezone2 && tilezone2->depth == 2 && (addr-tilezone2->base)/16 < 1024)
			add_tile_2(tilezone2->base, tilezone2->DSVRAMAddress, (addr-tilezone2->base)/16);
	}
	
	NeedUpdate4b = 0;
	NeedUpdate2b = 0;	
	
//	memset(&TileZones, 0, sizeof(TileZones));
	PPU_add_tile_address(0);
	PPU_add_tile_address(1);
	PPU_add_tile_address(2);
}

void     add_sprite_tile_4(uint16 tilenb, int pos)
{
  int		k;
  uint8		*tile_ptr; //nice settings, see below.
  
  uint32	tile_addr;
  uint32	*VRAM_ptr;

  if (tilenb&0x100)
    tile_addr = (tilenb+pos)*32+GFX.spr_addr_base+GFX.spr_addr_select;
  else
    tile_addr = (tilenb+pos)*32+GFX.spr_addr_base;

  VRAM_ptr = (uint32 *)(SPRITE_GFX + (CONVERT_SPR_TILE(tilenb+pos)+(GFX.spr_bank<<4))*16);    
  
	tile_ptr = SNESC.VRAM+tile_addr;  //nice settings, see below.
	
	for (k=0;k<8;k++,tile_ptr+=2){
		 	//*VRAM_ptr++=(uint32)((bittab[tile_ptr[0x00]]) | (bittab[tile_ptr[0x01]]<<1) 
			//			|(bittab[tile_ptr[0x10]]<<2) | (bittab[tile_ptr[0x11]]<<3));
		dmaFillWord(3,
			(uint32)((bittab[tile_ptr[0x00]]) | (bittab[tile_ptr[0x01]]<<1)|(bittab[tile_ptr[0x10]]<<2) | (bittab[tile_ptr[0x11]]<<3)),
			(uint32)  	VRAM_ptr,
			4
		);
		VRAM_ptr++;
	}	
	
}



void	PPU_setMap(int i, int j, int tilenb, int bg, int p, int f)
{
  uint16 *map_addr = GFX.DSMapAddress;
  
  if (bg == 2)
  {
  	// FIXME : 2 bits palettes here
    p += 8; 
    int mapblock = ((i >> 5) << 10) + ((j >> 5) << 11);  	
    *(map_addr + (i&31) + (j&31)*32 + mapblock) = (tilenb) | (f << 10) | (p << 12);
  }
  else
  {
    if (GFX.map_size[bg] == BG_32x32)
    {
    	*(map_addr + i + j*32) = (tilenb) | (f << 10) | (p << 12);
    }
    else if (GFX.map_size[bg] == BG_32x64)
    {
    	int mapblock = ((j >> 5) << 10);
    	*(map_addr + (i&31) + (j&31)*32 + mapblock) = (tilenb) | (f << 10) | (p << 12);
    }
    else
    {
    	int mapblock = ((i >> 5) << 10) + ((j >> 5) << 11);
    	*(map_addr + (i&31) + (j&31)*32 + mapblock) = (tilenb) | (f << 10) | (p << 12);
    }
  }
}


void update_scroll()
{
   BG0HOFS = CPU.PPU_PORT[(0x0D)+(0<<1)];
   BG0VOFS = CPU.PPU_PORT[(0x0E)+(0<<1)]+GFX.YScroll;
   BG1HOFS = CPU.PPU_PORT[(0x0D)+(1<<1)];
   BG1VOFS = CPU.PPU_PORT[(0x0E)+(1<<1)]+GFX.YScroll;
   BG2HOFS = CPU.PPU_PORT[(0x0D)+(2<<1)];
   BG2VOFS = CPU.PPU_PORT[(0x0E)+(2<<1)]+GFX.BG3YScroll;
   BG3HOFS = CPU.PPU_PORT[(0x0D)+(3<<1)];
   BG3VOFS = CPU.PPU_PORT[(0x0E)+(3<<1)]+GFX.BG3YScroll;
}

void	draw_plane(int bg, int bg_mode, int nb_tilex, int nb_tiley, int tile_size)
{
  int		i, j, map_address;
  int		tilenb, f, p;  	
  uint16	*map_ptr;
  uint8		*tiles_def = NULL;
  void	 	(*add_tile)(int tilebase, uint16 *vram_addr, int tilenb) = NULL;
  int 		tile_zone = (GFX.tile_address[bg]>>13);
	
  map_address  = GFX.map_slot[bg]<<11;
  GFX.DSMapAddress = (uint16 *)BG_MAP_RAM_0x06020000(GFX.map_slot_ds[bg]);
  
  switch (bg_mode) {
  	case 2 : 
  		tiles_def = GFX.tiles2b_def+GFX.tile_address[bg]/16;
  		add_tile = add_tile_2;

	    GFX.tiles_def[tile_zone+0] &= 0xF;
  		GFX.tiles_def[tile_zone+0] |= ((tile_zone+1)<<4)|(1 << bg);
  		GFX.tiles_def[tile_zone+1] &= 0xF;
  		GFX.tiles_def[tile_zone+1] |= ((tile_zone+1)<<4)|(1 << bg);  
  		break;
  	case 4 : 
  		tiles_def = GFX.tiles4b_def+GFX.tile_address[bg]/32;
  		add_tile = add_tile_4;

	    GFX.tiles_def[tile_zone+0] &= 0xF;
  		GFX.tiles_def[tile_zone+0] |= ((tile_zone+1)<<4)|(1 << bg);
  		GFX.tiles_def[tile_zone+1] &= 0xF;
  		GFX.tiles_def[tile_zone+1] |= ((tile_zone+1)<<4)|(1 << bg);  
	    GFX.tiles_def[tile_zone+2] &= 0xF;
  		GFX.tiles_def[tile_zone+2] |= ((tile_zone+1)<<4)|(1 << bg);
  		GFX.tiles_def[tile_zone+3] &= 0xF;
  		GFX.tiles_def[tile_zone+3] |= ((tile_zone+1)<<4)|(1 << bg);  
  		break;
  	case 8 : 
  		tiles_def = GFX.tiles8b_def+GFX.tile_address[bg]/64;
  		add_tile = add_tile_8;		
  		break;
  }

  uint16 *vram_addr = SNESToDS_TileAddress[(GFX.tile_address[bg]>>11)+bg]->DSVRAMAddress;
  		
  if (tile_size == 3)
  {
  	/* 8 px tile mode */
  	map_ptr = (uint16 *)(SNESC.VRAM+map_address);
  	// This look a stupid optimization, but it really speed up this loop

	if (CFG.BG3TilePriority && bg == 2)
	{
		//memset(GFX.BG3TilePriority, 0, 32); //byte copy
		dmaFillWord(3,0, (uint32)&GFX.BG3TilePriority, 32);
	}

  	if (nb_tilex == 32 && nb_tiley == 32)
  	{
  	for (i = 0; i < 32*32; i++)
  	{
  		//tilenb = *map_ptr++;
  		tilenb = map_ptr[i];
  		p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14;
  		if (bg == 2)
  		{
  			if ((tilenb&0x3ff) != 0)
  			{
  			if (tilenb & 0x2000) // priority tile
  				GFX.BG3TilePriority[i/32]++;
  			else
  				GFX.BG3TilePriority[i/32]--;
  			}
  			p += 8; // FIXME: this should be made for all 2 colors bg
  		}
  		tilenb &= 0x3ff;

  		if (!tiles_def[tilenb])
  			add_tile(GFX.tile_address[bg], vram_addr, tilenb);
  			
  		//*ds_map_ptr++ = (p << 12)|(f << 10)|tilenb;
  		GFX.DSMapAddress[i] = (p << 12)|(f << 10)|tilenb;
   	}  	
  	} 
	else if 
	
	(nb_tilex == 64 && nb_tiley == 64)
  	
  	{  		
  	for (i = 0; i < 64*64; i++)
  	{
  		tilenb = map_ptr[i];
  		p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14; tilenb &= 0x3ff;
  		if (!tiles_def[tilenb])
  		{ 		
  			add_tile(GFX.tile_address[bg], vram_addr, tilenb);
  		}
  		if (bg == 2)
  			p += 8; // FIXME: 2 colors palette
  		GFX.DSMapAddress[i] = (p << 12)|(f << 10)|tilenb;
  	}
  	}
	
	else if (nb_tilex == 64 && nb_tiley == 32)
  	{
  	for (i = 0; i < 64*32; i++)
  	{
  		tilenb = map_ptr[i];
  		p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14;
  		if (bg == 2)
  		{
  			if ((tilenb&0x3ff) != 0)
  			{  			
  			if (tilenb & 0x2000) // priority tile
  				GFX.BG3TilePriority[i/32]++;
  			else
  				GFX.BG3TilePriority[i/32]--;
  			}
  			p += 8; // FIXME: this should be made for all 2 colors bg
  		}
        tilenb &= 0x3ff;  		
  		if (!tiles_def[tilenb])
  			add_tile(GFX.tile_address[bg], vram_addr, tilenb);
  		GFX.DSMapAddress[i] = (p << 12)|(f << 10)|tilenb;
  	}

  	}
	
  	else
  	{
  	for (i = 0; i < 32*64; i++)
  	{
  		tilenb = map_ptr[i];
  		p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14;
  		if (bg == 2)
  		{
  			if ((tilenb&0x3ff) != 0)
  			{  			
  			if (tilenb & 0x2000) // priority tile
  				GFX.BG3TilePriority[i/32]++;
  			else
  				GFX.BG3TilePriority[i/32]--;
  			}
  			p += 8; // FIXME: this should be made for all 2 colors bg
  		}  			
        tilenb &= 0x3ff;   		
  		if (!tiles_def[tilenb])
  			add_tile(GFX.tile_address[bg], vram_addr, tilenb);
  		GFX.DSMapAddress[i] = (p << 12)|(f << 10)|tilenb;
  	}

  	}
  }
  else
  {
    /* infamous 16 px tile mode */  	
    for (j=0; j<nb_tiley; j++) {
      int mapblock;
      if (nb_tilex == 32 && nb_tiley == 64)
      	mapblock = ((j >> 5) << 11);
      else
        mapblock = ((j >> 5) << 12);
  	  map_ptr = (uint16 *)(SNESC.VRAM+map_address+(j&31)*64+mapblock);
      for (i=0; i<nb_tilex; i++) {
        tilenb = map_ptr[(i&31)+((i >> 5) << 10)];
        
        p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14; tilenb &= 0x3ff;
        
        if (!tiles_def[tilenb]) //Little trick ;)     
        {
        	add_tile(GFX.tile_address[bg], vram_addr, tilenb);
        	add_tile(GFX.tile_address[bg], vram_addr, (tilenb+1)&1023);
        	add_tile(GFX.tile_address[bg], vram_addr, (tilenb+16)&1023);
        	add_tile(GFX.tile_address[bg], vram_addr, (tilenb+17)&1023);
        }
        
        if (f == 0) 
        {
        	DRAW_TILE(i*2,   j*2,   tilenb,    bg, p, f);
        	DRAW_TILE(i*2+1, j*2,   tilenb+1,  bg, p, f);
        	DRAW_TILE(i*2,   j*2+1, tilenb+16, bg, p, f);
        	DRAW_TILE(i*2+1, j*2+1, tilenb+17, bg, p, f);
        }
        else if (f == 1) // Horizontal 
        {
        	DRAW_TILE(i*2,   j*2,   tilenb+1,    bg, p, f);
        	DRAW_TILE(i*2+1, j*2,   tilenb,  bg, p, f);
        	DRAW_TILE(i*2,   j*2+1, tilenb+17, bg, p, f);
        	DRAW_TILE(i*2+1, j*2+1, tilenb+16, bg, p, f);
        }
        else if (f == 2) // Vertical
        {
        	DRAW_TILE(i*2,   j*2,   tilenb+16,    bg, p, f);
        	DRAW_TILE(i*2+1, j*2,   tilenb+17,  bg, p, f);
        	DRAW_TILE(i*2,   j*2+1, tilenb, bg, p, f);
        	DRAW_TILE(i*2+1, j*2+1, tilenb+1, bg, p, f);
        }
        else // H/V
        {
        	DRAW_TILE(i*2,   j*2,   tilenb+17,    bg, p, f);
        	DRAW_TILE(i*2+1, j*2,   tilenb+16,  bg, p, f);
        	DRAW_TILE(i*2,   j*2+1, tilenb+1, bg, p, f);
        	DRAW_TILE(i*2+1, j*2+1, tilenb, bg, p, f);
        }  
      }
    }
  }
}

void	draw_plane_withpriority(int bg, int bg_mode, int nb_tilex, int nb_tiley, int tile_size)
{
	int		i, map_address;
	int		tilenb, f, p, pr;  	
	uint16	*map_ptr;
	uint16	*ds_map_ptr1, *ds_map_ptr2;
	uint8		*tiles_def = NULL;
	void	 	(*add_tile)(int tilebase, uint16 *vram_addr, int tilenb) = NULL;
	int 		tile_zone = (GFX.tile_address[bg]>>13);

	map_address  = GFX.map_slot[bg]<<11;
	ds_map_ptr1 = (uint16 *)BG_MAP_RAM_0x06020000(GFX.map_slot_ds[bg]); // main layer for high priority tile
	ds_map_ptr2 = (uint16 *)BG_MAP_RAM_0x06020000(GFX.map_slot_ds[3]); // back layer for low priority tile

	switch (bg_mode) {
	case 2 : 
		tiles_def = GFX.tiles2b_def+GFX.tile_address[bg]/16;
		add_tile = add_tile_2;

		GFX.tiles_def[tile_zone+0] &= 0xF;
		GFX.tiles_def[tile_zone+0] |= ((tile_zone+1)<<4)|(1 << bg);
		GFX.tiles_def[tile_zone+1] &= 0xF;
		GFX.tiles_def[tile_zone+1] |= ((tile_zone+1)<<4)|(1 << bg);  
		break;
	case 4 : 
		tiles_def = GFX.tiles4b_def+GFX.tile_address[bg]/32;
		add_tile = add_tile_4;

		GFX.tiles_def[tile_zone+0] &= 0xF;
		GFX.tiles_def[tile_zone+0] |= ((tile_zone+1)<<4)|(1 << bg);
		GFX.tiles_def[tile_zone+1] &= 0xF;
		GFX.tiles_def[tile_zone+1] |= ((tile_zone+1)<<4)|(1 << bg);  
		GFX.tiles_def[tile_zone+2] &= 0xF;
		GFX.tiles_def[tile_zone+2] |= ((tile_zone+1)<<4)|(1 << bg);
		GFX.tiles_def[tile_zone+3] &= 0xF;
		GFX.tiles_def[tile_zone+3] |= ((tile_zone+1)<<4)|(1 << bg);  
		break;
	case 8 : 
		tiles_def = GFX.tiles8b_def+GFX.tile_address[bg]/64;
		add_tile = add_tile_8;		
		break;
	}

	uint16 *vram_addr = SNESToDS_TileAddress[(GFX.tile_address[bg]>>11)+bg]->DSVRAMAddress;
  		
	if (tile_size != 3){
		return;
	}
  	/* 8 px tile mode */
  	map_ptr = (uint16 *)(SNESC.VRAM+map_address);

  	for (i = nb_tilex*nb_tiley; i >= 0; i--)
  	{
  		tilenb = map_ptr[i];
  		p = (tilenb&0x1c00) >> 10; f = (tilenb&0xc000) >> 14;
  		pr = (tilenb&0x2000);
  	    tilenb &= 0x3ff;
  		if (!tiles_def[tilenb]){
  			add_tile(GFX.tile_address[bg], vram_addr, tilenb);
  		}
  		if (pr) // High priority
  		{
  			// Put in the regular layer
  			ds_map_ptr1[i] = (p << 12)|(f << 10)|tilenb;
  			ds_map_ptr2[i] = CFG.Debug2; // FIXME: replace by the empty tile
  		} else // Low priority
  		{
  			// Put in the back layer
  			ds_map_ptr2[i] = (p << 12)|(f << 10)|tilenb;
  			ds_map_ptr1[i] = CFG.Debug2; // FIXME: replace by the empty tile
  		}
   	}  	
}



int	map_duplicate(int snes_block)
{
	int	i;
	for (i = 8; i < 32; i++)
	{
		if (GFX.map_def[i] == ((snes_block<<3) | 7))
			return i;
	}
	for (i = 8; i < 32; i++)
	{
		if (GFX.map_def[i] == 0)
		{
			GFX.map_def[i] = ((snes_block<<3) | 7);  
			return i;		
		}
	}	  
	return 0;
}


int		map_duplicate2(int snes_block)
{
	int	i;
	for (i = 8; i < 32; i++)
	{
		if (GFX.map_def[i+0] == ((snes_block<<3) | 7))
			return i;
	}
	for (i = 8; i < 32; i++)
	{
		if (GFX.map_def[i+0] == 0 && GFX.map_def[i+1] == 0)
		{
//			LOG("Allocated: %d\n", i);
			GFX.map_def[i+0] = GFX.map_def[i+1] = ((snes_block<<3) | 7);  
			return i;		
		}
	}	    
	return 0;
}

/*
 * Get 4 empty blocks in MAP VRAM for this infamous 16 pixels tile  mode
 */
 
int		map_duplicate4(int snes_block)
{
	int	i;
	for (i = 8; i < 32; i++)
	{
		if (GFX.map_def[i+0] == ((snes_block<<3) | 7))
			return i;
	}
	for (i = 8; i < 32; i++)
	{
		if (GFX.map_def[i+0] == 0 && GFX.map_def[i+1] == 0 &&
		    GFX.map_def[i+2] == 0 && GFX.map_def[i+3] == 0)
		{
//			LOG("Allocated: %d\n", i);
			GFX.map_def[i+0] = GFX.map_def[i+1] = GFX.map_def[i+2] = GFX.map_def[i+3] =
				((snes_block<<3) | 7);  
			return i;		
		}
	}	 
	return 0;
}


void draw_plane_32_30(uint8 bg, uint8 bg_mode)
{
  int nb_tilex, nb_tiley;
  int tile_size;	

  //LOG("> draw 32x30 %d %d %08x\n", bg, GFX.map_slot[bg], CPU.PPU_PORT[0x05]&(0x10 << bg));
  if ((GFX.map_def[GFX.map_slot[bg]] & (1<<bg)) && 
  	  (GFX.tiles_def[GFX.tile_address[bg]>>13] & (1<<bg)))
  {
  	//LOG("> no draw 32x30 %d\n", GFX.tile_address[bg]>>13);
  	if (!(CPU.PPU_PORT[0x05]&(0x10 << bg)))
  	{
#if 0  		
  	if (bg == 2 /*&& 
  		(GFX.map_slot[2] == GFX.map_slot[0] || GFX.map_slot[1] == GFX.map_slot[0])*/)
  	{
  		GFX.map_slot_ds[bg] = map_duplicate(GFX.map_slot[bg]);
  		LOG("< allocated = %d\n", GFX.map_slot_ds[bg]);
  	}
  	else
#endif  	
  		GFX.map_slot_ds[bg] = GFX.map_slot[bg];
  		GFX.map_size[bg] = BG_32x32;
  	}   		
  	return;
  }
  GFX.map_def[GFX.map_slot[bg]] |= (1 << bg);
  
/*  int tile_zone = (GFX.tile_address[bg]>>13);
  GFX.tiles_def[tile_zone+0] &= 0xF;
  GFX.tiles_def[tile_zone+0] |= (GFX.tile_address[bg]>>9)|(1 << bg);
  GFX.tiles_def[tile_zone+1] &= 0xF;
  GFX.tiles_def[tile_zone+1] |= (GFX.tile_address[bg]>>9)|(1 << bg);*/
  
  //LOG("< draw 32x32 %d %d %d %02x l=%d\n", bg, GFX.map_slot[bg], GFX.tile_address[bg]>>13, CPU.PPU_PORT[0x05]&(0x10 << bg), SNES.V_Count);

  
  if (CPU.PPU_PORT[0x05]&(0x10 << bg)) {
  	GFX.map_slot_ds[bg] = map_duplicate4(GFX.map_slot[bg]);
  	GFX.map_size[bg] = BG_64x64;
  	nb_tilex = 32; nb_tiley = 32;
    tile_size = 4;
  } else {
#if 0  	
  	if (bg == 2 /*&& 
  		(GFX.map_slot[2] == GFX.map_slot[0] || GFX.map_slot[1] == GFX.map_slot[0])*/)
  	{
  		GFX.map_slot_ds[bg] = map_duplicate(GFX.map_slot[bg]);
  		LOG("< allocated = %d\n", GFX.map_slot_ds[bg]);
  	}
  	else	
#endif
	GFX.map_slot_ds[bg] = GFX.map_slot[bg];
  	if (CFG.TilePriorityBG == bg)
  		GFX.map_slot_ds[3] = map_duplicate2(GFX.map_slot[bg]);  		
  	GFX.map_size[bg] = BG_32x32;
  	nb_tilex = 32; nb_tiley = 32;
    tile_size = 3;
  }

  if (CFG.TilePriorityBG == bg)
  	draw_plane_withpriority(bg, bg_mode, nb_tilex, nb_tiley, tile_size);
  else
  draw_plane(bg, bg_mode, nb_tilex, nb_tiley, tile_size); 
}

void draw_plane_64_30(uint8 bg, uint8 bg_mode)
{
  int 	nb_tilex, nb_tiley;
  int	tile_size;  

  //LOG("> draw 64x30 %d %d %08x\n", bg, GFX.map_slot[bg], CPU.PPU_PORT[0x05]&(0x10 << bg));
  if ((GFX.map_def[GFX.map_slot[bg]]   & (1<<bg)) &&
  	  (GFX.map_def[GFX.map_slot[bg]+1] & (1<<bg)) &&
  	  (GFX.tiles_def[GFX.tile_address[bg]>>13] & (1<<bg)))
  {
  	if (!(CPU.PPU_PORT[0x05]&(0x10 << bg)))
  	{
  		GFX.map_slot_ds[bg] = GFX.map_slot[bg];  		
  		GFX.map_size[bg] = BG_64x32;
  	}
  	return;
  }
  GFX.map_def[GFX.map_slot[bg]] |= (1 << bg);
  GFX.map_def[GFX.map_slot[bg]+1] |= (1 << bg);
    
  //LOG("< draw 64x32 %d %d %d %02x\n", bg, GFX.map_slot[bg], GFX.tile_address[bg]>>13, CPU.PPU_PORT[0x05]&(0x10 << bg));
  if (CPU.PPU_PORT[0x05]&(0x10 << bg)) {
  	GFX.map_slot_ds[bg] = map_duplicate4(GFX.map_slot[bg]);
  	GFX.map_size[bg] = BG_64x64;
  	nb_tilex = 32; nb_tiley = 32;
    tile_size = 4; 
  } else {
  	GFX.map_slot_ds[bg] = GFX.map_slot[bg];
  	if (CFG.TilePriorityBG == bg)
  		GFX.map_slot_ds[3] = map_duplicate2(GFX.map_slot[bg]);
  	GFX.map_size[bg] = BG_64x32;
  	nb_tilex = 64; nb_tiley = 32;
    tile_size = 3;
  }  
  
  
  if (CFG.TilePriorityBG == bg)
  	draw_plane_withpriority(bg, bg_mode, nb_tilex, nb_tiley, tile_size);
  else
  	draw_plane(bg, bg_mode, nb_tilex, nb_tiley, tile_size); 
}

void draw_plane_32_60(uint8 bg, uint8 bg_mode)
{
  int 	nb_tilex, nb_tiley;
  int	tile_size;  

  if ((GFX.map_def[GFX.map_slot[bg]]   & (1<<bg)) &&
  	  (GFX.map_def[GFX.map_slot[bg]+1] & (1<<bg)) && 
  	  (GFX.tiles_def[GFX.tile_address[bg]>>13] & (1<<bg)))
  {
  	if (!(CPU.PPU_PORT[0x05]&(0x10 << bg)))
  	{
  		GFX.map_slot_ds[bg] = GFX.map_slot[bg];
  		GFX.map_size[bg] = BG_32x64;
  	}
  	return;
  }
  GFX.map_def[GFX.map_slot[bg]] |= (1 << bg);
  GFX.map_def[GFX.map_slot[bg]+1] |= (1 << bg);
  
  //LOG("< draw 32x64 %d %d %d %02x\n", bg, GFX.map_slot[bg], GFX.tile_address[bg]>>13, CPU.PPU_PORT[0x05]&(0x10 << bg));

  if (CPU.PPU_PORT[0x05]&(0x10 << bg)) {
  	GFX.map_slot_ds[bg] = map_duplicate4(GFX.map_slot[bg]);
  	GFX.map_size[bg] = BG_64x64;
  	nb_tilex = 32; nb_tiley = 32;
    tile_size = 4; 
  } else {
  	GFX.map_slot_ds[bg] = GFX.map_slot[bg];
  	GFX.map_size[bg] = BG_32x64;
  	nb_tilex = 32; nb_tiley = 64;
    tile_size = 3;
  }  
  
  draw_plane(bg, bg_mode, nb_tilex, nb_tiley, tile_size); 
}

void draw_plane_64_60(uint8 bg, uint8 bg_mode)
{
  int 	nb_tilex, nb_tiley;
  int	tile_size;  
  
  if ((GFX.map_def[GFX.map_slot[bg]]   & (1<<bg)) &&
  	  (GFX.map_def[GFX.map_slot[bg]+1] & (1<<bg)) && 
  	  (GFX.map_def[GFX.map_slot[bg]+2] & (1<<bg)) &&
  	  (GFX.map_def[GFX.map_slot[bg]+3] & (1<<bg)) &&
  	  (GFX.tiles_def[GFX.tile_address[bg]>>13] & (1<<bg)))
  {
  	if (!(CPU.PPU_PORT[0x05]&(0x10 << bg)))
  	{
  		GFX.map_slot_ds[bg] = GFX.map_slot[bg];
  		GFX.map_size[bg] = BG_64x64;
  	}
  	return;
  }
  
  GFX.map_def[GFX.map_slot[bg]] |= (1 << bg);
  GFX.map_def[GFX.map_slot[bg]+1] |= (1 << bg);
  GFX.map_def[GFX.map_slot[bg]+2] |= (1 << bg);
  GFX.map_def[GFX.map_slot[bg]+3] |= (1 << bg);

  //LOG("< draw 64x64 %d %d %d %02x l=%d\n", bg, GFX.map_slot[bg], GFX.tile_address[bg]>>13, CPU.PPU_PORT[0x05]&(0x10 << bg), SNES.V_Count);

  if (GFX.map_slot[bg] > 28)
  {
  	// BG overflow
  	return;	
  }

  if (CPU.PPU_PORT[0x05]&(0x10 << bg)) {
  	GFX.map_slot_ds[bg] = map_duplicate4(GFX.map_slot[bg]);
  	GFX.map_size[bg] = BG_64x64;
  	nb_tilex = 32; nb_tiley = 32;
    tile_size = 4; 
  } else {
  	GFX.map_slot_ds[bg] = GFX.map_slot[bg];
  	GFX.map_size[bg] = BG_64x64;
  	nb_tilex = 64; nb_tiley = 64;
    tile_size = 3;
  }  
  
  draw_plane(bg, bg_mode, nb_tilex, nb_tiley, tile_size);  
}



inline void draw_tile_sprite(int TILENB, int X, int Y, int SIZEX)
{
    add_sprite_tile_4(GFX.spr_info[TILENB].fst_tile, (Y*16+X));
}

void PPU_set_sprites_bank(int bank)
{
	int	i;
	if (bank == 1)
	{
		for (i = 0; i < 128; i++)
			OAM[i*4+2] |= (1<<4);
	} else
	{
		for (i = 0; i < 128; i++)
			OAM[i*4+2] &= ~(1<<4);
	}
}

void draw_sprites(/*uint8 pf*/)
{
	int i, x, y;
	int spr_size;
	t_OAM_entry sprite;

	//  memset(GFX.spr_cnt, 0, 240); 

	if (GFX.HighestSprite == 0)
		i = 127;
	else
		i = GFX.HighestSprite-1;

	((uint32 *)&sprite)[0] = 0;
	((uint32 *)&sprite)[1] = 0;
	
	while (1)
	{
		if ((GFX.spr_info[i].pos_y < 224 || GFX.spr_info[i].pos_y > 240) &&
			((uint32 *)&GFX.spr_info[i])
		 /*(GFX.spr_info[i].pos_x || GFX.spr_info[i].pos_y || GFX.spr_info[i].fst_tile
		 || GFX.spr_info[i].palette || GFX.spr_info[i].flip))*/
		)
		{
			sprite.X = GFX.spr_info[i].pos_x+SPRITE_ADD_X(i);
			if (CFG.Scaled)
			{
				if (CFG.Scaled == 2) // Full screen
				{
					sprite.rot_data = 1;
					sprite.rot_data2 = GFX.spr_info[i].flip;					
					sprite.Y = GFX.spr_info[i].pos_y*192/224; // doesn't seem okay
				}
				else // Half scaling : no sprite squishing
				{
					sprite.flip = GFX.spr_info[i].flip;
					sprite.Y = (GFX.spr_info[i].pos_y-8)*192/208; // doesn't seem okay
				}
			}
			else
			{
				sprite.flip = GFX.spr_info[i].flip;
				sprite.Y = GFX.spr_info[i].pos_y-GFX.YScroll+1; // +1 seems okay
			}
			sprite.tile_index = CONVERT_SPR_TILE(GFX.spr_info[i].fst_tile)+(GFX.spr_bank<<4);
			
			sprite.palette = GFX.spr_info[i].palette;
			sprite.pr = CFG.SpritePr[GFX.spr_info[i].pf_priority];

			spr_size = GFX.spr_info_ext[i>>2]&(1<<(((i&0x3)<<1)+1));

			switch (CPU.PPU_PORT[0x01]>>5)
			{
			case 0x00:
				if (spr_size)
				{
					sprite.size = 1;
					
					for (y = 0; y < 2; y++)
						for (x = 0; x < 2; x++){
							draw_tile_sprite(i, x, y, 8);
						}
				}
				else
				{
					sprite.size = 0;
					draw_tile_sprite(i, 0, 0, 0);
				}
				break;
			case 0x01:
				if (spr_size)
				{
					sprite.size = 2;
					for (y = 0; y < 4; y++)
						for (x = 0; x < 4; x++)
							draw_tile_sprite(i, x, y, 24);
				}
				else
				{
					sprite.size = 0;
					draw_tile_sprite(i, 0, 0, 0);
				}
				break;
			case 0x02:
				if (spr_size)
				{
					sprite.size = 3;
					for (y = 0; y < 8; y++)
						for (x = 0; x < 8; x++)
							draw_tile_sprite(i, x, y, 56);
				}
				else
				{
					sprite.size = 0;
					draw_tile_sprite(i, 0, 0, 0);
				}
				break;
			case 0x03:
				if (spr_size)
				{
					sprite.size = 2;
					for (y = 0; y < 4; y++)
						for (x = 0; x < 4; x++)
							draw_tile_sprite(i, x, y, 24);
				}
				else
				{
					sprite.size = 1;
					for (y = 0; y < 2; y++)
						for (x = 0; x < 2; x++)
							draw_tile_sprite(i, x, y, 8);
				}
				break;
			case 0x04:
				if (spr_size)
				{
					sprite.size = 3;
					for (y = 0; y < 8; y++)
						for (x = 0; x < 8; x++)
							draw_tile_sprite(i, x, y, 56);
				}
				else
				{
					sprite.size = 1;
					for (y = 0; y < 2; y++)
						for (x = 0; x < 2; x++)
							draw_tile_sprite(i, x, y, 8);
				}
				break;
			case 0x05:
				if (spr_size)
				{
					sprite.size = 3;
					for (y = 0; y < 8; y++)
						for (x = 0; x < 8; x++)
							draw_tile_sprite(i, x, y, 56);
				}
				else
				{
					sprite.size = 2;
					for (y = 0; y < 4; y++)
						for (x = 0; x < 4; x++)
							draw_tile_sprite(i, x, y, 24);
				}
				break;
			}
			((uint32 *)OAM)[i*2] = ((uint32 *)&sprite)[0];
			((uint32 *)OAM)[i*2+1] = ((uint32 *)&sprite)[1];

		}
		else
		{
			((uint32 *)OAM)[i*2] = 0x0200;
			((uint32 *)OAM)[i*2+1] = 0;
		}
	
		i--;
		if (i == GFX.HighestSprite-1)
			break;
		if (i == -1)
			i = 127;
	}
	
	// Set scaling matrix
	if (CFG.Scaled == 1) // Half scaling
	{
		// Pa (X scale)
		OAM[3] = 0x0100;
		// Pd (Y scale) : 192/224
		//OAM[15] = 0x012A;
		OAM[15] = 0x0110;
		// H FLIP
		OAM[16+3] = 0xFF01;
		//OAM[16+15] = 0x012A;
		OAM[16+15] = 0x0110;
		// V FLIP
		OAM[32+3] = 0x0100;
		//OAM[32+15] = 0xFED6;
		OAM[32+15] = 0xFEF1;
		// H + V FLIP
		OAM[48+3] = 0xFF01;
		//OAM[48+15] = 0xFED6;
		OAM[48+15] = 0xFEF1;
	}
	else // Full scaling
	{
		// Pa (X scale)
		OAM[3] = 0x0100;
		// Pd (Y scale) : 192/224
		//OAM[15] = 0x012A;
		OAM[15] = 0x0120;
		// H FLIP
		OAM[16+3] = 0xFF01;
		//OAM[16+15] = 0x012A;
		OAM[16+15] = 0x0120;
		// V FLIP
		OAM[32+3] = 0x0100;
		//OAM[32+15] = 0xFED6;
		OAM[32+15] = 0xFEE0;
		// H + V FLIP
		OAM[48+3] = 0xFF01;
		//OAM[48+15] = 0xFED6;
		OAM[48+15] = 0xFEE0;
	}
}

void renderMode1(int NB_BG, int MODE_1, int MODE_2, int MODE_3, int MODE_4)
{
   uint32 SB;

   SB = (CPU.PPU_PORT[0x2D]|CPU.PPU_PORT[0x2C])&CFG.BG_Layer&((1<<NB_BG)-1);
   
   if ((SB&0x08)) {
    DRAW_PLANE(3, MODE_4);
  }
  if ((SB&0x04)) {
    DRAW_PLANE(2, MODE_3);
  }
  if ((SB&0x02)) {
    DRAW_PLANE(1, MODE_2);
  }
  if ((SB&0x01)) {
    DRAW_PLANE(0, MODE_1);
  }  	
}

void renderMode3(int MODE_1, int MODE_2)
{
  uint32 SB = (CPU.PPU_PORT[0x2D]|CPU.PPU_PORT[0x2C])&CFG.BG_Layer&((1<<2)-1);

  if ((SB&0x02)) {
    DRAW_PLANE(1, MODE_2);
  }
  if ((SB&0x01)) {
    DRAW_PLANE(0, MODE_1);
  }
}

// FIXME: mode 0 doesn't work anymore
//segfaults @ ITCM
void PPU_RenderLineMode1(uint32 NB_BG, uint32 MODE_1, uint32 MODE_2, uint32 MODE_3, uint32 MODE_4, t_GFX_lineInfo *l)
{
  uint32 	order[4] = { 3, 3, 3, 3 };
  uint32	SB;
  
  if (CFG.LayersConf == 0)
  {
  int BG3TilePriority = 1;  	 
  if (CFG.BG3TilePriority)
  {
  	if (GFX.map_size[2] == BG_32x32)
   		BG3TilePriority = GFX.BG3TilePriority[((SNES.V_Count+CPU.PPU_PORT[0x12])/8)&31];
   	else
  	if (GFX.map_size[2] == BG_64x32)
   		BG3TilePriority = GFX.BG3TilePriority[(((SNES.V_Count+CPU.PPU_PORT[0x12])/8)&31)
   											  /*+((CPU.PPU_PORT[0x11]/8)&32)*/];
   	else
  	if (GFX.map_size[2] == BG_32x64)
   		BG3TilePriority = GFX.BG3TilePriority[((SNES.V_Count+CPU.PPU_PORT[0x12])/8)&63];
  }

   
  SB = CPU.PPU_PORT[0x2C]&CFG.BG_Layer&0xF;

  /* SPRITE MAINSCREEN : 3 2 */
  /* 3=1 1=2 0=0 */ 
  if ((CPU.PPU_PORT[0x2D]&CFG.BG_Layer&0xF) && (SB&0x04)) order[2] = 2;
  if (CFG.TilePriorityBG == -1)
  {
  	if (SB&0x02) order[1]=2;
  	if (SB&0x01) order[0]=1;
  } else
  if (CFG.TilePriorityBG == 1)
  {
  	// BG2 is split in two layers, BG1 is between both
//  	if (SB&0x02) order[3] = 2; // BG2 low
  	if (SB&0x01) order[0] = 2; // BG1 
  	if (SB&0x02) order[1] = 1; // BG2 high
  } else
  if (CFG.TilePriorityBG == 0)
  {
  	// BG1 is split in two layers, BG2 is between both
//  	if (SB&0x01) order[3] = 3; // BG1 low
  	if (SB&0x02) order[1] = 2; // BG2 Z
  	if (SB&0x01) order[0] = 1; // BG1 high
  } 

  /* SPRITE MAINSCREEN : 1 0 */
  if ((SB&0x04) && (CPU.PPU_PORT[0x05]&8) && BG3TilePriority > 0) order[2] = 0;
  
/*  if (SNES.V_Count == CFG.Debug2)
  	LOG("%x / %x / %x / %x\n", order[0], order[1], order[2], order[3]);*/
  }
  else
  {
  order[0] = CFG.LayerPr[0];
  order[1] = CFG.LayerPr[1];
  order[2] = CFG.LayerPr[2];
  order[3] = CFG.LayerPr[3];
  }
  
  SB = (CPU.PPU_PORT[0x2D]|CPU.PPU_PORT[0x2C])&CFG.BG_Layer&0x17/*0x1f*/;
  
  if (CFG.TilePriorityBG != -1)
  {
  	// Use BG3 for duplicate of main layer
  	SB |= 0x08;
  	GFX.map_slot_ds[3] = map_duplicate2(GFX.map_slot[CFG.TilePriorityBG]);
  	l->lBG3_CR = BG_COLOR_16 | order[3] | (GFX.map_slot_ds[3]<<8) |
  				(GFX.tile_slot[CFG.TilePriorityBG]<<2) | GFX.map_size[CFG.TilePriorityBG];
  	
  } else
  	l->lBG3_CR = 0;
  
  // FIXME: should block interrupt here /moved vram to 0x06020000  
  l->lDISPLAY_CR = MODE_0_2D | DISPLAY_SPR_2D | (SB << 8) | DISPLAY_SCREEN_BASE(2) | DISPLAY_CHAR_BASE(2);
  
  l->lBG0_CR = BG_COLOR_16 | order[0] | (GFX.tile_slot[0]<<2)  | (GFX.map_slot_ds[0]<<8) | GFX.map_size[0];
  l->lBG1_CR = BG_COLOR_16 | order[1] | (GFX.tile_slot[1]<<2)  | (GFX.map_slot_ds[1]<<8) | GFX.map_size[1];
  if (CFG.Scaled != 0 || (GFX.YScroll == 16 && CFG.BG3Squish == 0))
  l->lBG2_CR = BG_COLOR_16 | order[2] | (GFX.tile_slot[2]<<2)  | (GFX.map_slot_ds[2]<<8) | GFX.map_size[2];

  /* Transparency */
  if (CFG.Transparency && (CPU.PPU_PORT[0x30]&0x02) && (CPU.PPU_PORT[0x31] != 0))
  {
  	int AB;
  	if (CPU.PPU_PORT[0x31]&0x40) // Half blending
  		AB = 0x0808;
  	else 
  		AB = 0x0F0F;

  	// Destination is sub screen
  	// Source is main screen
  	int source = CPU.PPU_PORT[0x2C]&CPU.PPU_PORT[0x31]&0x1F;
  	int destination = CPU.PPU_PORT[0x2D]&0x1F;
  	
  	if (CFG.TilePriorityBG != -1)
  	{
  		if (source & (CFG.TilePriorityBG+1)) source |= 0x08;
  		if (destination & (CFG.TilePriorityBG+1)) destination |= 0x08;
  	}
  		
  	l->lBLEND = BLEND_ALPHA | (destination << 8) | (source) | (AB << 16);
  }
  else
  {
  	l->lBLEND = BLEND_NONE;
  }

}

//segfaults @ ITCM
void PPU_RenderLineMode3(uint32 MODE_1, uint32 MODE_2, t_GFX_lineInfo *l)
{
   uint32 	order[2];
   uint32	SB;

  SB = CPU.PPU_PORT[0x2D]&CFG.BG_Layer&((1<<2)-1);
  if (SB&0x02) order[1]=3;
  if (SB&0x01) order[0]=3;
  SB = CPU.PPU_PORT[0x2C]&CFG.BG_Layer&((1<<2)-1);
  if (SB&0x02) order[1]=1;
  if (SB&0x01) order[0]=0;

  SB = (CPU.PPU_PORT[0x2D]|CPU.PPU_PORT[0x2C])&CFG.BG_Layer&0x13;

  // FIXME: should block interrupt here / moved vram to 0x06020000
  l->lDISPLAY_CR = MODE_0_2D | DISPLAY_SPR_2D | (SB << 8) | DISPLAY_SCREEN_BASE(2) | DISPLAY_CHAR_BASE(2);
  l->lBG0_CR = BG_COLOR_256 | order[0] | (GFX.tile_slot[0]<<2)  | (GFX.map_slot_ds[0]<<8) | GFX.map_size[0];
  l->lBG1_CR = BG_COLOR_16 | order[1] | (GFX.tile_slot[1]<<2)  | (GFX.map_slot_ds[1]<<8) | GFX.map_size[1];
  l->lBG2_CR = l->lBG3_CR = 0;
  // FIXME
}

//cant move to itcm because break games (mode7)
void PPU_RenderLineMode7(t_GFX_lineInfo *l)
{
	int SB = (CFG.BG_Layer&0x10)|((CFG.BG_Layer & 0x1) << 3);
	
	l->lDISPLAY_CR = MODE_2_2D | DISPLAY_SPR_2D | (SB << 8) | DISPLAY_SCREEN_BASE(2) | DISPLAY_CHAR_BASE(2);
  	l->lBG0_CR = 0; l->lBG1_CR = 0; l->lBG2_CR = 0;
  	if (!SNES.Mode7Repeat)
	  	l->lBG3_CR = BG_COLOR_256 | (GFX.tile_slot[0]<<2)| BG_RS_128x128 | BG_PRIORITY(3) | BG_WRAP_ON;
	else
		l->lBG3_CR = BG_COLOR_256 | (GFX.tile_slot[0]<<2) | BG_RS_128x128 | BG_PRIORITY(3) ;


	int X0 = (int)CPU.PPU_PORT[0x1F] << 19; X0 >>= 19;
	int Y0 = (int)CPU.PPU_PORT[0x20] << 19; Y0 >>= 19;
	int HOffset = (int)CPU.PPU_PORT[0x0D] << 19; HOffset >>= 19;
	int VOffset = (int)CPU.PPU_PORT[0x0E] << 19; VOffset >>= 19;
	 
	l->A = CPU.PPU_PORT[0x1B];
	l->B = CPU.PPU_PORT[0x1C];
	l->C = CPU.PPU_PORT[0x1D];
	l->D = CPU.PPU_PORT[0x1E];
	l->CX = l->A*(-X0+HOffset)+l->B*(SNES.V_Count-Y0+VOffset)+(X0<<8);
	l->CY = l->C*(-X0+HOffset)+l->D*(SNES.V_Count-Y0+VOffset)+(Y0<<8);
}

void renderMode7()
{
	static int Mode7FrameSkip = 0;

	if (!(CFG.BG_Layer & 0x1))
		return;
		
	// Update one frame on 4
	// FIXME : find a better method to speed up MODE 7
	if ((Mode7FrameSkip++ & 3) == 0)
		return;

    // Copy map
	uint16	*map_addr = (uint16*)BG_MAP_RAM_0x06020000(0);
	uint8	*BUFVRAM = SNESC.VRAM;
	uint8	*BUFVRAM1 = SNESC.VRAM+1;
	int		i, j;
	for (i = 0, j = 0; i < 128*128*2; i+=4, j++)
	{
		uint16	t;
		t = BUFVRAM[i]+(BUFVRAM[i+2]<<8); 
		map_addr[j] = t;
	}
	// Copy tile data
	uint16	*tile_addr = (uint16*)BG_TILE_RAM_0x06020000(GFX.tile_slot[0]);
	for (i = 0, j = 0; i < 128*128*2; i+=4, j++)
	{
		uint16	t;
		t = BUFVRAM1[i]+(BUFVRAM1[i+2]<<8); 
		tile_addr[j] = t;
	}
}

void PPU_reset()
{
  int i;
	
  DISPCNT &= 0xffff00ff;

  for (i = 0; i < 128; i++)
  {  
	((uint32 *)OAM)[i*2] = 0x0200;
	((uint32 *)OAM)[i*2+1] = 0;
  }
  GFX.brightness = 0;
  
  // Clear DS VRAM
  i = 0;
  dmaFillWord(3,i, (uint32)0x6020000,  128*1024); // FIX: clear only bank A!!
  dmaFillWord(3,i, (uint32)0x6040000,  128*1024); // FIX: clear only bank A!!
  dmaFillWord(3,i, (uint32)0x6400000,  64*1024); // FIX: clear only bank A!!
  
  GFX.DSFrame = 0;
  memset(GFX.tiles2b_def, 0, 4096);
  memset(GFX.tiles4b_def, 0, 2048);
  memset(GFX.tiles8b_def, 0, 1024);
  	
  memset(GFX.map_def, 0, 32);
  memset(GFX.tiles_def, 0, 8);
  
  // Testing stuff to move to GFX someday
  memset(TileZones, 0, sizeof(TileZones));
  memset(SNESToDS_TileAddress, 0, sizeof(SNESToDS_TileAddress));
  Mode7TileZone = 0;
  
  NeedUpdate2b = NeedUpdate4b = NeedFlush2b = NeedFlush4b = 0;

  memset(ToUpdate2b, 0, 100);
  memset(ToUpdate4b, 0, 100);
  
  GFX.spr_bank = 0;	
}

inline void	PPU_setBackColor(uint32 rgb)
{
    BG_PALETTE[0] = rgb;
}

//software way for offsetting tiles. DS uses the bg scroll regs
#if 0
void	update_scrolly(int bg)
{
  int delta;

  if (GFX.tiles_ry[bg] != 8 && CPU.PPU_PORT[(0x0E)+bg*2] != GFX.old_scrolly[bg]) {
    delta = GFX.tiles_ry[bg] + CPU.PPU_PORT[(0x0E)+bg*2]-GFX.old_scrolly[bg];
    if (delta >= 0 && delta < 8)
      GFX.tiles_ry[bg] = delta;
    else
      GFX.tiles_ry[bg] = 8;
  }
}

void	update_scrollx(int bg)
{
  int i, delta;

  if (GFX.tiles_ry[bg] != 8 && CPU.PPU_PORT[(0x0D)+bg*2] != GFX.old_scrollx[bg]) {
    delta = CPU.PPU_PORT[(0x0D)+bg*2]-GFX.old_scrollx[bg];

    if (delta < -7 || delta > 7)
      GFX.tiles_ry[bg] = 8;
    else {
      for (i = 0; i < GFX.tiles_cnt[bg*2]; i++)
        GFX.tiles_x[bg*2][i] -= delta;
      for (i = 0; i < GFX.tiles_cnt[bg*2+1]; i++)
        GFX.tiles_x[bg*2+1][i] -= delta;
    }
  }
}
#endif

//__attribute__((section(".itcm")))//segfaults @ ITCM
void	PPU_updateGFX(int line)
{
	t_GFX_lineInfo *l = &GFX.lineInfo[line];
	
	if (l->mode & 0xf8)
	{
	if (l->mode == -1)
	{
		// Blank line
		DISPCNT |= DISPLAY_SCREEN_OFF;
		return;
	}	
	if (l->mode & 0x10)
	{
		PPU_set_sprites_bank(0);
		return;
	}
	if (l->mode & 0x20)
	{
		PPU_set_sprites_bank(1);
		return;
	}
	if (l->mode & 0x40) // bi-linear filtering
	{
		l->lBG0_Y0 += GFX.DSFrame&1;
		l->lBG1_Y0 += GFX.DSFrame&1;
		l->lBG2_Y0 += GFX.DSFrame&1;
		l->lBG3_Y0 += GFX.DSFrame&1;
		l->mode &= 7;
	}
	}
		
	if (l->mode == 7)
	{	
		asm("ldr  	r2, [%1, #12];"
			"str  	r2, [%0, #12];"
			"ldr	r2, [%1, #8];"
			"str 	r2, [%0, #8];"
			"ldr	r2, [%1, #4];"
			"str 	r2, [%0, #4];"
			"ldr	r2, [%1, #0];"
			"str 	r2, [%0, #0]" : :
			"r"(0x04000030), "r"(&l->A) : "r2");	
/*	   	BG3_XDX = l->A; 
	   	BG3_XDY = l->B;
	  	BG3_YDX = l->C; 
	  	BG3_YDY = l->D;
		BG3_CX = l->CX;
		BG3_CY = l->CY;*/
		
		DISPCNT = l->lDISPLAY_CR;
		REG_BG3CNT = l->lBG3_CR;		
		return;		
	}

	DISPCNT = l->lDISPLAY_CR;
	BG_PALETTE[0] = l->lBACK_color;
	
	// Write scrollings regs 
	asm("ldr  	r2, [%1, #0];"
		"str  	r2, [%0, #0];"
		"ldr	r2, [%1, #4];"
		"str 	r2, [%0, #4];"
		"ldr	r2, [%1, #8];"
		"str 	r2, [%0, #8];"
		"ldr	r2, [%1, #12];"
		"str 	r2, [%0, #12];"		
		: :	"r"(0x04000010), "r"(&l->lBG0_X0) : "r2");	
	
	// Write CR regs
	asm("ldr  	r2, [%1, #0];"
		"str  	r2, [%0, #0];"
		"ldr	r2, [%1, #4];"
		"str 	r2, [%0, #4];"
		: :	"r"(0x04000008), "r"(&l->lBG0_CR) : "r2");	
	// BLEND
	(*(vuint32*)0x04000050) = l->lBLEND; 
		

//	BRIGHTNESS = l->lBRIGHTNESS;
}

//__attribute__((section(".itcm")))//segfaults @ ITCM
inline void	PPU_line_handle_BG3()
{ 
  int 			  y;
  t_GFX_lineInfo *l;
  
  // Update BG 3 only because it doesn't have the same scrolling than others		
  y = SNES.V_Count-GFX.BG3YScroll;
  if (CFG.BG3Squish != 0)
  {
  	if (SNES.V_Count < (16-CFG.BG3Squish)*8)
		y += CFG.BG3Squish*8;
	else
	if (SNES.V_Count >= (16+CFG.BG3Squish)*8)
		y -= CFG.BG3Squish*8;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
	else
		y = -1;			
  }	
  if (y >= 0 && y < 192)
  {	
	l = &GFX.lineInfo[y];
	//FIXME move me elsewhee... duplicate code here
	int order, SB;
	if (CFG.LayersConf == 0)
	{ 
  	  int BG3TilePriority = 1;  	 
  	  if (CFG.BG3TilePriority)	
      	BG3TilePriority = GFX.BG3TilePriority[((SNES.V_Count+CPU.PPU_PORT[0x12])/8)&31];	  	
	  order=3;
	  SB = CPU.PPU_PORT[0x2C]&CFG.BG_Layer&0xF;
	  if ((CPU.PPU_PORT[0x2D]&CFG.BG_Layer&0xF) && (SB&0x04)) order = 2;
	  if ((SB&0x04) && (CPU.PPU_PORT[0x05]&8) && BG3TilePriority > 0) order = 0;
	}
	else
	{
	  order = CFG.LayerPr[2];
	}
	
	l->lBG2_CR = BG_COLOR_16 | order | (GFX.tile_slot[2]<<2)  | (GFX.map_slot_ds[2]<<8) | GFX.map_size[2];
    l->lBG2_X0 = CPU.PPU_PORT[(0x0D)+(2<<1)];

	if (CFG.BG3Squish != 0)
	{
		if (y < 96)
			l->lBG2_Y0 = CPU.PPU_PORT[(0x0E)+(2<<1)]+GFX.BG3YScroll-CFG.BG3Squish*8;
		else
			l->lBG2_Y0 = CPU.PPU_PORT[(0x0E)+(2<<1)]+GFX.BG3YScroll+CFG.BG3Squish*8;
	} else
	l->lBG2_Y0 = CPU.PPU_PORT[(0x0E)+(2<<1)]+GFX.BG3YScroll;
  }
}

//__attribute__((section(".itcm")))//segfaults @ ITCM
void	PPU_line_render()
{
	int y;
	t_GFX_lineInfo *l;
		
	if ((GFX.YScroll != GFX.BG3YScroll || CFG.BG3Squish != 0) && (CPU.PPU_PORT[0x05]&7) == 1)
	{
		if ((!(CPU.PPU_PORT[0x00]&0x80)) && (CPU.PPU_PORT[0x00]&0x0f)) 
			PPU_line_handle_BG3();
	}
	
	y = SNES.V_Count-GFX.YScroll;	
	if (y < 0 || y >= 192)
		return;
	l = &GFX.lineInfo[y];
	
	if (((CPU.PPU_PORT[0x00]&0x80)) || !(CPU.PPU_PORT[0x00]&0x0f)) // Line hidden
	{
		l->mode = -1;
		return;
	}
	
	
	// Sprite address change detection
	// If sprite address change is outside DS's screen, put the event at line #0
	/*
	if (y == 0)
	{
		if (GFX.spr_addr_vcount[0]-GFX.YScroll < 0)
		{
			l->mode = 0x10;
			GFX.spr_addr_vcount[0] = 0xFFFF;
			return;
		}			
		if (GFX.spr_addr_vcount[1]-GFX.YScroll < 0)
		{
			l->mode = 0x20;
			GFX.spr_addr_vcount[1] = 0xFFFF;
			return;
		}
		if (GFX.spr_addr_vcount[0] != 0xFFFF && GFX.spr_addr_vcount[0]-GFX.YScroll > 191)
		{
			l->mode = 0x10;
		
			GFX.spr_addr_vcount[0] = 0xFFFF;
			return;
		}			
		if (GFX.spr_addr_vcount[1] != 0xFFFF && GFX.spr_addr_vcount[1]-GFX.YScroll > 191)
		{
			l->mode = 0x20;		
			GFX.spr_addr_vcount[1] = 0xFFFF;
			return;
		}		
	}
	*/
	if (GFX.spr_addr_vcount[0] == SNES.V_Count)
	{
		l->mode = 0x10;
		GFX.spr_addr_vcount[0] = 0xFFFF;
		return;
	}
	if (GFX.spr_addr_vcount[1] == SNES.V_Count)
	{
		l->mode = 0x20;
		GFX.spr_addr_vcount[1] = 0xFFFF;
		return;
	}
		
	l->mode = (CPU.PPU_PORT[0x05]&7);	

	if ((CPU.PPU_PORT[0x31]&0x20))
		l->lBACK_color = GFX.SNESPal[0]+GFX.BACK;
	else
		l->lBACK_color = GFX.SNESPal[0];
		
	l->lBG0_X0 = CPU.PPU_PORT[(0x0D)+(0<<1)];
    l->lBG0_Y0 = CPU.PPU_PORT[(0x0E)+(0<<1)]+GFX.YScroll;
    l->lBG1_X0 = CPU.PPU_PORT[(0x0D)+(1<<1)];
    l->lBG1_Y0 = CPU.PPU_PORT[(0x0E)+(1<<1)]+GFX.YScroll;
    if (GFX.YScroll == GFX.BG3YScroll && CFG.BG3Squish == 0)
    {
    l->lBG2_X0 = CPU.PPU_PORT[(0x0D)+(2<<1)];
    l->lBG2_Y0 = CPU.PPU_PORT[(0x0E)+(2<<1)]+GFX.BG3YScroll;
    }
/*    l->lBG3_X0 = CPU.PPU_PORT[(0x0D)+(3<<1)];
    l->lBG3_Y0 = CPU.PPU_PORT[(0x0E)+(3<<1)]+16;*/

    if (CFG.TilePriorityBG != -1)
    {
	l->lBG3_X0 = CPU.PPU_PORT[(0x0D)+(CFG.TilePriorityBG<<1)];
    l->lBG3_Y0 = CPU.PPU_PORT[(0x0E)+(CFG.TilePriorityBG<<1)]+GFX.YScroll;
    }

   
    //l->lBRIGHTNESS = (2<<14) | ((0x0f - GFX.brightness)<<1);
    switch (CPU.PPU_PORT[0x05]&7) {
      case 0 : PPU_RenderLineMode1(4, 2, 2, 2, 2, l); break;
      case 1 : PPU_RenderLineMode1(3, 4, 4, 2, 0, l); break;
      case 2 : PPU_RenderLineMode1(2, 4, 4, 0, 0, l); break;
      case 3 : PPU_RenderLineMode3(   8, 4, l); break;
      case 4 : PPU_RenderLineMode3(   8, 2, l); break;
      case 5 : PPU_RenderLineMode1(2, 4, 2, 0, 0, l); break;
      case 6 : PPU_RenderLineMode1(1, 4, 0, 0, 0, l); break;
      case 7 : PPU_RenderLineMode7(l); break;
    }
}

void	PPU_line_render_scaled()
{
	int y;
	t_GFX_lineInfo *l;
	int offset = (CFG.Scaled == 1) ? 8 : 0;
	
	if (CFG.Scaled == 1) // Half scaling
		y = (SNES.V_Count-offset) * 192 / 208;
	else
		y = SNES.V_Count * 192 / 224;
	
	if (y < 0 || y >= 192)
		return;

	l = &GFX.lineInfo[y];
	
	if (((CPU.PPU_PORT[0x00]&0x80)) || !(CPU.PPU_PORT[0x00]&0x0f)) // Line hidden
	{
		l->mode = -1;
		return;
	}
	

	// Sprite address change detection
	// If sprite address change is outside DS's screen, put the event at line #0
	if (y == 0)
	{
		if (GFX.spr_addr_vcount[0]-offset < 0)
		{
			l->mode = 0x10;
			GFX.spr_addr_vcount[0] = 0xFFFF;
			return;
		}			
		if (GFX.spr_addr_vcount[1]-offset < 0)
		{
			l->mode = 0x20;
			GFX.spr_addr_vcount[1] = 0xFFFF;
			return;
		}
		if (GFX.spr_addr_vcount[0] != 0xFFFF && GFX.spr_addr_vcount[0]-offset > 191)
		{
			l->mode = 0x10;
			GFX.spr_addr_vcount[0] = 0xFFFF;
			return;
		}			
		if (GFX.spr_addr_vcount[1] != 0xFFFF && GFX.spr_addr_vcount[1]-offset > 191)
		{
			l->mode = 0x20;			
			GFX.spr_addr_vcount[1] = 0xFFFF;
			return;
		}		
	}
	
	if (GFX.spr_addr_vcount[0] == SNES.V_Count)
	{
		l->mode = 0x10;
		GFX.spr_addr_vcount[0] = 0xFFFF;
		return;
	}
	if (GFX.spr_addr_vcount[1] == SNES.V_Count)
	{
		l->mode = 0x20;
		GFX.spr_addr_vcount[1] = 0xFFFF;
		return;
	}
		
	l->mode = (CPU.PPU_PORT[0x05]&7);	

	if ((CPU.PPU_PORT[0x31]&0x20))
		l->lBACK_color = GFX.SNESPal[0]+GFX.BACK;
	else
		l->lBACK_color = GFX.SNESPal[0];
	
	int per_px = (CFG.Scaled == 1) ? 12 : 6;
	l->lBG0_X0 = CPU.PPU_PORT[(0x0D)+(0<<1)];
	l->lBG1_X0 = CPU.PPU_PORT[(0x0D)+(1<<1)];
	l->lBG2_X0 = CPU.PPU_PORT[(0x0D)+(2<<1)];
	l->lBG0_Y0 = CPU.PPU_PORT[(0x0E)+(0<<1)]+offset+y/per_px;
	l->lBG1_Y0 = CPU.PPU_PORT[(0x0E)+(1<<1)]+offset+y/per_px;
	l->lBG2_Y0 = CPU.PPU_PORT[(0x0E)+(2<<1)]+offset+y/per_px;

	if (CFG.TilePriorityBG != -1)
    {
	l->lBG3_X0 = CPU.PPU_PORT[(0x0D)+(CFG.TilePriorityBG<<1)];
    l->lBG3_Y0 = CPU.PPU_PORT[(0x0E)+(CFG.TilePriorityBG<<1)]+offset+y/per_px;
    }
	
	if (y % per_px == per_px-1)
		l->mode |= 0x40;
   
    //l->lBRIGHTNESS = (2<<14) | ((0x0f - GFX.brightness)<<1);
    switch (CPU.PPU_PORT[0x05]&7) {
      case 0 : PPU_RenderLineMode1(4, 2, 2, 2, 2, l); break;
      case 1 : PPU_RenderLineMode1(3, 4, 4, 2, 0, l); break;
      case 2 : PPU_RenderLineMode1(2, 4, 4, 0, 0, l); break;
      case 3 : PPU_RenderLineMode3(   8, 4, l); break;
      case 4 : PPU_RenderLineMode3(   8, 2, l); break;
      case 5 : PPU_RenderLineMode1(2, 4, 2, 0, 0, l); break;
      case 6 : PPU_RenderLineMode1(1, 4, 0, 0, 0, l); break;
      case 7 : PPU_RenderLineMode7(l); break;
    }
}

void draw_screen()
{
		
    GFX.Blank_Screen = 0;   
    
	if (!GFX.v_blank)
	{
		GFX.need_update = 1;
		return;
	}

#ifdef SNEMUL_LOGGING

#ifdef TIMER_Y      
    SNES.stat_before = REG_VCOUNT;
#else
	SNES.stat_before = TIMER3_DATA;
#endif

#endif
   
	//BRIGHTNESS = (2<<14) | ((0x0f - GFX.brightness));
	REG_MASTER_BRIGHT = (2<<14) | ((0x0f - GFX.brightness)<<1);
      
    if (GFX.tiles_dirty)
	{
		//LOG("Clear all tiles\n");
  		//ori: memset(GFX.tiles2b_def, 0, 4096);
		dmaFillWord(3,0,(uint32)&GFX.tiles2b_def,4096);
  		
		//ori: memset(GFX.tiles4b_def, 0, 2048);
  		dmaFillWord(3,0,(uint32)&GFX.tiles4b_def,2048);
		
		//ori: memset(GFX.tiles8b_def, 0, 1024);		
		dmaFillWord(3,0,(uint32)&GFX.tiles8b_def,1024);
		
		
		GFX.tiles_dirty = 0;
	}

    PPU_updateCache();
    
    if (GFX.Sprites_table_dirty)
    {
  	  draw_sprites();
      GFX.Sprites_table_dirty = 0;  	
    }
    
    switch (CPU.PPU_PORT[0x05]&7) {
      case 0 : renderMode1(4, 2, 2, 2, 2); break;
      case 1 : renderMode1(3, 4, 4, 2, 0); break;
      case 2 : renderMode1(2, 4, 4, 0, 0); break;
      case 3 : renderMode3(   8, 4); break;
      case 4 : renderMode3(   8, 2); break;
      case 5 : renderMode1(2, 4, 2, 0, 0); break;
      case 6 : renderMode1(1, 4, 0, 0, 0); break;
      case 7 : renderMode7(); break;
    }
	
#ifdef SNEMUL_LOGGING
#ifdef TIMER_Y	
	if (SNES.stat_before > REG_VCOUNT)
	   SNES.stat_GFX += 262+REG_VCOUNT-SNES.stat_before;
	else
	   SNES.stat_GFX += REG_VCOUNT-SNES.stat_before;
#else
	if (SNES.stat_before > TIMER3_DATA)
		SNES.stat_GFX += 65536+TIMER3_DATA-SNES.stat_before;
	else
		SNES.stat_GFX += TIMER3_DATA-SNES.stat_before;
#endif
SNES.stat_GFX = 0;
#endif 
    SNES.stat_CPU = 0;  
    SNES.stat_IOREGS = 0;
    SNES.stat_DMA = 0;

#if 0
  }
#endif 
}

void	PPU_setPalette(int c, uint16 rgb)
{
	if ((CPU.PPU_PORT[0x05]&7) > 1)
	{
		if (c > 0/* || !(CPU.PPU_PORT[0x31]&0x20)*/) // FIXME
			BG_PALETTE[c] = rgb; 
		if (c >= 128)
			SPRITE_PALETTE[c-128] = rgb;
		return;
	}
	
	if (c < 128) // TILE color
	{
		
		if (c > 0/* || !(CPU.PPU_PORT[0x31]&0x20)*/) // FIXME
			BG_PALETTE[c] = rgb;
		// Recopie pour les palettes 2 bits
		if (c < 32)
		  	BG_PALETTE[128+((c>>2)<<4)+(c&3)] = rgb;
	}
	else // SPRITE color
		SPRITE_PALETTE[c-128] = rgb;
}

void	PPU_setScreen(int value)
{     
     if ((value & 0xF) >  GFX.brightness)
     {
     	GFX.brightness = value & 0xF;
     }
          
  	int i;
    for (i = 0; i < 256; i++)
     {
       PPU_setPalette(i, GFX.SNESPal[i]);
	 }
}

void PPU_update()
{
	int i;
	
	PPU_reset();
	
	for (i = 0; i < 256; i++)
	{
	  PPU_setPalette(i, GFX.SNESPal[i]);
	}
}
