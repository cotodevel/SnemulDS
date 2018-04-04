//============================================================================//
//                                                                            //
//  Copyright 2007 Rick "Lick" Wong                                           //
//                                                                            //
//  This library is licensed as described in the included readme.             //
//                                                                            //
//============================================================================//

#ifdef ARM9

#ifndef __RAM
#define __RAM

#include "typedefsTGDS.h"
#include "dsregs.h"


#endif


#ifdef __cplusplus
extern "C" {
#endif


//typedef enum { DETECT_RAM=0, SC_RAM, M3_RAM, OPERA_RAM, G6_RAM, EZ_RAM } RAM_TYPE;

//  Call this before the others
vuint16*  ram_init ();

//  Returns the type of the RAM device
uint32   ram_type ();

//  Returns the type of the RAM device in a string
const sint8*   ram_type_string ();

//  Returns the total amount of RAM in bytes
uint32   ram_size ();


//  Unlocks the RAM and returns a pointer to the begin
vuint16* ram_unlock ();

//  Locks the RAM
void  ram_lock ();


#ifdef __cplusplus
}
#endif
#endif
