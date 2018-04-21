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


#ifndef __gui_console_snemulds_h__
#define __gui_console_snemulds_h__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <malloc.h>
#include <ctype.h>
#include "common.h"
#include "specific_shared.h"

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "consoleTGDS.h"
#include "core.h"
#include "opcodes.h"
#include "snapshot.h"
#include "ppu.h"

#include "guiTGDS.h"
#include "fs.h"
#include "common.h"

#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "biosTGDS.h"

#include "conf.h"
#include "gui_widgets.h"
#include "console_str.h"
#include "InterruptsARMCores_h.h"
#include "about.h"
#include "dmaTGDS.h"

#include "posixHandleTGDS.h"
#include "fsfatlayerTGDSLegacy.h"
#include "keypadTGDS.h"
#include "videoTGDS.h"





#ifdef __cplusplus
extern "C" {
#endif

extern t_GUIScreen *buildGFXConfigMenu();
extern int GUI_getConfigInt(sint8 *objname, sint8 *field, int val);
extern sint8 *GUI_getConfigStr(sint8 *objname, sint8 *field, sint8 *str);

//simple hook for writing values to volatile config file loaded earlier from setting file. Does not update config file.
extern void GUI_setConfigStr(sint8 *objname, sint8 *field, sint8 *value);

extern void GUI_setConfigStrUpdateFile(sint8 *objname, sint8 *field, sint8 *value);
extern void GUI_setObjFromConfig(t_GUIScreen *scr, int nb, sint8 *objname);
extern t_GUIScreen *buildMainMenu();
extern int ROMSelectorHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int SPCSelectorHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int LoadStateHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int SaveStateHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int GFXConfigHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int AdvancedHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int LayersOptionsHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern t_GUIScreen *buildLayersMenu();
extern int ScreenOptionsHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern t_GUIScreen *buildScreenMenu();
extern int OptionsHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern t_GUIScreen *buildOptionsMenu();
extern int MainScreenHandler(t_GUIZone *zone, int msg, int param, void *arg);
extern int FirstROMSelectorHandler(t_GUIZone *zone, int msg, int param, void *arg);
//read rom from (path)touchscreen:output rom -> CFG.ROMFile
extern void GUI_getROM(sint8 *rompath);
extern void GUI_deleteROMSelector();
extern void GUI_createMainMenu();
extern void GUI_getConfig();
extern void	GUI_showrominfo(int size);

//Definition that overrides the weaksymbol expected from toolchain to init console video subsystem
extern vramSetup * getProjectSpecificVRAMSetup();

//Custom console VRAM layout setup

//1) VRAM Layout
extern bool InitProjectSpecificConsole();

//2) Uses subEngine: VRAM Layout -> Console Setup
extern vramSetup * SNEMULDS_2DVRAM_SETUP();

#ifdef __cplusplus
}
#endif


#endif
