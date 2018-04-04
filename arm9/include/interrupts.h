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

#ifndef __interrupts9_h__
#define __interrupts9_h__

#include "InterruptsARMCores_h.h"
#include "specific_shared.h"
#include "guiTGDS.h"
#include "dsregs_asm.h"
#include "fs.h"
#include "gfx.h"
#include "cfg.h"
#include "apu.h"
#include "ram.h"
#include "core.h"
#include "conf.h"
#include "frontend.h"
#include "main.h"
#include "font_8x8_uv.h"
#include "ppu.h"
#include "keypadTGDS.h"
#include "interrupts.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern void Vcounter();
extern void Vblank();
extern void Hblank();

#ifdef __cplusplus
}
#endif
