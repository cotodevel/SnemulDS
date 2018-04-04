#include "apu_jukebox.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "cfg.h"
#include "gfx.h"
#include "core.h"
#include "engine.h"
#include "apu.h"
#include "ppu.h"
#include "main.h"
#include "conf.h"
#include "fs.h"
#include "memmap.h"
#include "crc32.h"
#include "guiTGDS.h"
#include "common.h"
#include "specific_shared.h"
#include "utilsTGDS.h"

int selectSong(sint8 *name)
{
	sint8 spcname[100];

	strcpy(spcname, CFG.ROMPath);
	if (CFG.ROMPath[strlen(CFG.ROMPath)-1] != '/')
		strcat(spcname, "/");
	strcat(spcname, "/");
	strcat(spcname, name);
	strcpy(CFG.Playlist, spcname);
	CFG.Jukebox = 1;
	CFG.Sound_output = 0;
	APU_stop();
	if (FS_loadFile(spcname, (sint8 *)APU_RAM_ADDRESS, 0x10200) < 0)
		return -1;
	APU_playSpc();
	// Wait APU init
	
	return 0;
}