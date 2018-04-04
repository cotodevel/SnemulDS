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

#ifndef __api_wrapper_h__
#define __api_wrapper_h__

#include "dsregs.h"
#include "typedefsTGDS.h"
#include "consoleTGDS.h"
#include "api_wrapper.h"
#include "apu_jukebox.h"
#include "gui_console_connector.h"


#endif


#ifdef __cplusplus
extern "C" {
#endif

extern uint8 LayersConf[10][4];

extern void readOptionsFromConfig(sint8 *section);
extern void saveOptionsToConfig(sint8 *section);
extern void	PPU_ChangeLayerConf(int i);
extern void	applyOptions();

extern void packOptions(uint8 *ptr);
extern void unpackOptions(int version, uint8 *ptr);
extern int 	checkConfiguration(sint8 *name, int crc);

extern int	loadROM(sint8 *name, int confirm);
extern int	changeROM(sint8 *ROM, int size);

#ifdef __cplusplus
}
#endif
