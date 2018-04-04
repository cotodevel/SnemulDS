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

#ifndef __ppu_h__
#define __ppu_h__

#ifdef ARM9

#include "dsregs.h"
#include "typedefsTGDS.h"
#include "gfx.h"

//Map base for each VRAM allocated region
#define BG_MAP_RAM_0x06020000(base) (uint16*)((uint16*)(((base)*0x800) + 0x06020000))
#define BG_MAP_RAM_0x06040000(base) (uint16*)((uint16*)(((base)*0x800) + 0x06040000))
#define BG_MAP_RAM_0x06060000(base) (uint16*)((uint16*)(((base)*0x800) + 0x06060000))

#define BG_TILE_RAM_0x06020000(base)   (uint16*)((uint16*)(((base)*0x4000) + 0x06020000))    
#define BG_TILE_RAM_0x06040000(base)   (uint16*)((uint16*)(((base)*0x4000) + 0x06040000))    
#define BG_TILE_RAM_0x06060000(base)   (uint16*)((uint16*)(((base)*0x4000) + 0x06060000))

#endif

/* should be 64 bytes long */
typedef struct s_OAM_entry
{
	uint8 Y;
	uint8 rot_data:2;
	uint8 mode:2;
	uint8 mosaic:1;
	uint8 color_depth:1;
	uint8 shape:2;
	
	uint16 X:9;
	uint8 rot_data2:3;
	uint8 flip:2;
	uint8 size:2;
	
	uint16 tile_index:10;
	uint8 pr:2;
	uint8 palette:4;
	
	uint16 rot_data3;
} t_OAM_entry;


/* Testing stuff... */

typedef struct
{
	int 	base;	// SNES base address
	int		depth;  // Bpp depth: 2 4 8
	uint16	*DSVRAMAddress;
	int		used;	
} t_TileZone;

#define CONVERT_SPR_TILE(tn) (((tn)&0xF)|(((tn)>>4)<<5))
//#define CONVERT_SPR_TILE(tn) (tn)
#define SNES_VRAM_OFFSET ((SNES_Port[0x01]&0x03) << 14)
#define DRAW_TILE(I, J, TILENB, BG, P, F) PPU_setMap(I, J, (TILENB)&1023, BG, P, F) 
#define ADD_TILE(TILE_BASE, TILENB, BG_MODE) \
  switch (BG_MODE) { \
    case 2 : if (!(GFX.tiles2b_def[TILE_BASE/16+(TILENB)] & 2)) \
  			    add_tile_2(TILE_BASE, TILENB); break; \
    case 4 : if (!GFX.tiles4b_def[TILE_BASE/32+(TILENB)]) \
    			add_tile_4(TILE_BASE, TILENB); break; \
    case 8 : if (!GFX.tiles8b_def[TILE_BASE/64+(TILENB)]) \
    			add_tile_8(TILE_BASE, TILENB); break; \
  }

#define SPRITE_ADD_X(INDEX) -(((GFX.spr_info_ext[INDEX>>2]&(1<<((INDEX&0x3)<<1))) != 0)<<8)
#define SPRITE_POS_Y(INDEX) (GFX.spr_info[INDEX].pos_y > 239 ? (sint8)GFX.spr_info[INDEX].pos_y : GFX.spr_info[INDEX].pos_y)


#endif


#ifdef __cplusplus
extern "C" {
#endif

extern uint32 bittab[256];
extern uint32 bittab8[16];
extern void DRAW_PLANE(uint8 BG,uint8  BG_MODE);
extern void    init_render();
extern void check_sprite_addr();
extern void check_tile_addr();

extern int			NeedUpdate2b;
extern int			NeedUpdate4b;
extern int			NeedFlush2b;
extern int			NeedFlush4b;

extern t_TileZone	TileZones[8];
extern uint32		Mode7TileZone;
extern t_TileZone	*SNESToDS_TileAddress[8*4];
extern uint16		ToUpdate2b[100];
extern uint16		ToUpdate4b[100];

extern int	PPU_get_bgmode(int mode, int bg);
extern int	PPU_get_tile_address(int tile_address, int bg_mode);
extern int	PPU_allocate_tilezone();
extern int	PPU_get_most_used();
extern int	PPU_allocate_tilezone2();
extern void	PPU_add_tile_address(int bg);
extern void     add_tile_2(int tile_addr_base, uint16 *vram_addr, int tilenb);
extern void     add_tile_4(int tile_addr_base, uint16 *vram_addr, int tilenb);
extern void     add_tile_8(int tile_addr_base, uint16 *vram_addr, int tilenb);
extern int		PPU_AddTile2InCache(t_TileZone *tilezone, int addr);
extern int		PPU_AddTile4InCache(t_TileZone *tilezone, int addr);
extern void check_tile();
extern void	PPU_updateCache();
extern void     add_sprite_tile_4(uint16 tilenb, int pos);
extern void	PPU_setMap(int i, int j, int tilenb, int bg, int p, int f);
extern void update_scroll();
extern void	draw_plane(int bg, int bg_mode, int nb_tilex, int nb_tiley, int tile_size);
extern void	draw_plane_withpriority(int bg, int bg_mode, int nb_tilex, int nb_tiley, int tile_size);
extern int	map_duplicate(int snes_block);
extern int		map_duplicate2(int snes_block);
extern int		map_duplicate4(int snes_block);
extern void draw_plane_32_30(uint8 bg, uint8 bg_mode);
extern void draw_plane_64_30(uint8 bg, uint8 bg_mode);
extern void draw_plane_32_60(uint8 bg, uint8 bg_mode);
extern void draw_plane_64_60(uint8 bg, uint8 bg_mode);
extern void draw_tile_sprite(int TILENB, int X, int Y, int SIZEX);
extern void PPU_set_sprites_bank(int bank);
extern void draw_sprites();
extern void renderMode1(int NB_BG, int MODE_1, int MODE_2, int MODE_3, int MODE_4);
extern void renderMode3(int MODE_1, int MODE_2);
extern void PPU_RenderLineMode1(uint32 NB_BG, uint32 MODE_1, uint32 MODE_2, uint32 MODE_3, uint32 MODE_4, t_GFX_lineInfo *l);
extern void PPU_RenderLineMode3(uint32 MODE_1, uint32 MODE_2, t_GFX_lineInfo *l);
extern void PPU_RenderLineMode7(t_GFX_lineInfo *l);
extern void renderMode7();
extern void PPU_reset();
extern void	PPU_setBackColor(uint32 rgb);
extern void	PPU_updateGFX(int line);
extern void	PPU_line_handle_BG3();
extern void	PPU_line_render();
extern void	PPU_line_render_scaled();
extern void draw_screen();
extern void	PPU_setPalette(int c, uint16 rgb);
extern void	PPU_setScreen(int value);
extern void PPU_update();

#ifdef __cplusplus
}
#endif