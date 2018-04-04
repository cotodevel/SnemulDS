
#ifdef ARM9

//=============================================================================//
//  Copyright 2007 Rick "Lick" Wong											//
//  This library is licensed as described in the included readme (MIT License) //
//=============================================================================//

#include "ram.h"
#include "ram_m3.h"

#include <stdio.h>
#include "busTGDS.h"

//SuperCard
static vuint16 *_sc_unlock(){
	*(vuint16*)0x9FFFFFE = 0xA55A;
	*(vuint16*)0x9FFFFFE = 0xA55A;
	*(vuint16*)0x9FFFFFE = 0x5; // RAM_RW
	*(vuint16*)0x9FFFFFE = 0x5;

	return (vuint16*)0x8000000;
}

static void _sc_lock(){
	*(vuint16*)0x9FFFFFE = 0xA55A;
	*(vuint16*)0x9FFFFFE = 0xA55A;
	*(vuint16*)0x9FFFFFE = 0x3; // MEDIA
	*(vuint16*)0x9FFFFFE = 0x3;
}

//M3
static vuint16 *_m3_unlock(){
	uint32 mode = 0x00400006; // RAM_RW
	vuint16 tmp;
	tmp = *(vuint16*)0x08E00002;
	tmp = *(vuint16*)0x0800000E;
	tmp = *(vuint16*)0x08801FFC;
	tmp = *(vuint16*)0x0800104A;
	tmp = *(vuint16*)0x08800612;
	tmp = *(vuint16*)0x08000000;
	tmp = *(vuint16*)0x08801B66;
	tmp = *(vuint16*)(0x08000000 + (mode << 1));
	tmp = *(vuint16*)0x0800080E;
	tmp = *(vuint16*)0x08000000;
	tmp = *(vuint16*)0x080001E4;
	tmp = *(vuint16*)0x080001E4;
	tmp = *(vuint16*)0x08000188;
	tmp = *(vuint16*)0x08000188;

	if(tmp){}
	return (vuint16*)0x8000000;
}

static void _m3_lock(){
	uint32 mode = 0x00400003; // MEDIA
	vuint16 tmp;
	tmp = *(vuint16*)0x08E00002;
	tmp = *(vuint16*)0x0800000E;
	tmp = *(vuint16*)0x08801FFC;
	tmp = *(vuint16*)0x0800104A;
	tmp = *(vuint16*)0x08800612;
	tmp = *(vuint16*)0x08000000;
	tmp = *(vuint16*)0x08801B66;
	tmp = *(vuint16*)(0x08000000 + (mode << 1));
	tmp = *(vuint16*)0x0800080E;
	tmp = *(vuint16*)0x08000000;
	tmp = *(vuint16*)0x080001E4;
	tmp = *(vuint16*)0x080001E4;
	tmp = *(vuint16*)0x08000188;
	tmp = *(vuint16*)0x08000188;
	if(tmp){}
}

//Opera DSBM
static vuint16 *_opera_unlock (){
	*(vuint16*)0x8240000 = 1;
	return (vuint16*)0x9000000;
}

static void _opera_lock(){
	*(vuint16*)0x8240000 = 0;
}

//G6
static vuint16 *_g6_unlock(){
	uint32 mode = 6; // RAM_RW
	vuint16 tmp;
	tmp = *(vuint16*)0x09000000;
	tmp = *(vuint16*)0x09FFFFE0;
	tmp = *(vuint16*)0x09FFFFEC;
	tmp = *(vuint16*)0x09FFFFEC;
	tmp = *(vuint16*)0x09FFFFEC;
	tmp = *(vuint16*)0x09FFFFFC;
	tmp = *(vuint16*)0x09FFFFFC;
	tmp = *(vuint16*)0x09FFFFFC;
	tmp = *(vuint16*)0x09FFFF4A;
	tmp = *(vuint16*)0x09FFFF4A;
	tmp = *(vuint16*)0x09FFFF4A;
	tmp = *(vuint16*)(0x09200000 + (mode << 1));
	tmp = *(vuint16*)0x09FFFFF0;
	tmp = *(vuint16*)0x09FFFFE8;

	if(tmp){}
	return (vuint16*)0x8000000;
}

static void _g6_lock(){
	uint32 mode = 3; // MEDIA
	vuint16 tmp;
	tmp = *(vuint16*)0x09000000;
	tmp = *(vuint16*)0x09FFFFE0;
	tmp = *(vuint16*)0x09FFFFEC;
	tmp = *(vuint16*)0x09FFFFEC;
	tmp = *(vuint16*)0x09FFFFEC;
	tmp = *(vuint16*)0x09FFFFFC;
	tmp = *(vuint16*)0x09FFFFFC;
	tmp = *(vuint16*)0x09FFFFFC;
	tmp = *(vuint16*)0x09FFFF4A;
	tmp = *(vuint16*)0x09FFFF4A;
	tmp = *(vuint16*)0x09FFFF4A;
	tmp = *(vuint16*)(0x09200000 + (mode << 1));
	tmp = *(vuint16*)0x09FFFFF0;
	tmp = *(vuint16*)0x09FFFFE8;
	if(tmp){}
}

//EZ
static vuint16 *_ez4_unlock(){
	*(vuint16*)0x9FE0000 = 0xD200; // SetRompage (OS mode)
	*(vuint16*)0x8000000 = 0x1500;
	*(vuint16*)0x8020000 = 0xD200;
	*(vuint16*)0x8040000 = 0x1500;
	*(vuint16*)0x9880000 = 0x8000;
	*(vuint16*)0x9FC0000 = 0x1500;

	*(vuint16*)0x9FE0000 = 0xD200; // OpenNorWrite
	*(vuint16*)0x8000000 = 0x1500;
	*(vuint16*)0x8020000 = 0xD200;
	*(vuint16*)0x8040000 = 0x1500;
	*(vuint16*)0x9C40000 = 0x1500;
	*(vuint16*)0x9FC0000 = 0x1500;

	return (vuint16*)0x8400000;
}

static void _ez_lock(){
	*(vuint16*)0x9FE0000 = 0xD200; // CloseNorWrite
	*(vuint16*)0x8000000 = 0x1500;
	*(vuint16*)0x8020000 = 0xD200;
	*(vuint16*)0x8040000 = 0x1500;
	*(vuint16*)0x9C40000 = 0xD200;
	*(vuint16*)0x9FC0000 = 0x1500;

	*(vuint16*)0x9FE0000 = 0xD200; // SetRompage(0)
	*(vuint16*)0x8000000 = 0x1500;
	*(vuint16*)0x8020000 = 0xD200;
	*(vuint16*)0x8040000 = 0x1500;
	*(vuint16*)0x9880000 = 0;
	*(vuint16*)0x9FC0000 = 0x1500;
}

static vuint16 *_ez3in1_unlock(){
	*(vuint16*)0x9FE0000 = 0xD200; // SetRompage(352)
	*(vuint16*)0x8000000 = 0x1500;
	*(vuint16*)0x8020000 = 0xD200;
	*(vuint16*)0x8040000 = 0x1500;
	*(vuint16*)0x9880000 = 352;
	*(vuint16*)0x9FC0000 = 0x1500;

	*(vuint16*)0x9FE0000 = 0xD200; // OpenNorWrite
	*(vuint16*)0x8000000 = 0x1500;
	*(vuint16*)0x8020000 = 0xD200;
	*(vuint16*)0x8040000 = 0x1500;
	*(vuint16*)0x9C40000 = 0x1500;
	*(vuint16*)0x9FC0000 = 0x1500;

	return (vuint16*)0x8000000;
}
#if 0
static vuint16 *_eznew_unlock(){
	*(vuint16*)0x9FE0000 = 0xD200; // SetRompage (384) //352?
	*(vuint16*)0x8000000 = 0x1500;
	*(vuint16*)0x8020000 = 0xD200;
	*(vuint16*)0x8040000 = 0x1500;
	*(vuint16*)0x9880000 = 384;
	*(vuint16*)0x9FC0000 = 0x1500;

	*(vuint16*)0x9FE0000 = 0xD200; // OpenNorWrite
	*(vuint16*)0x8000000 = 0x1500;
	*(vuint16*)0x8020000 = 0xD200;
	*(vuint16*)0x8040000 = 0x1500;
	*(vuint16*)0x9C40000 = 0x1500;
	*(vuint16*)0x9FC0000 = 0x1500;

	return (vuint16*)0x8000000;
}

static vuint16 *_ezplus_unlock(){
	*(vuint16*)0x9FE0000 = 0xD200; // SetRompage (192)
	*(vuint16*)0x8000000 = 0x1500;
	*(vuint16*)0x8020000 = 0xD200;
	*(vuint16*)0x8040000 = 0x1500;
	*(vuint16*)0x9880000 = 192;
	*(vuint16*)0x9FC0000 = 0x1500;

	*(vuint16*)0x9FE0000 = 0xD200; // OpenNorWrite
	*(vuint16*)0x8000000 = 0x1500;
	*(vuint16*)0x8020000 = 0xD200;
	*(vuint16*)0x8040000 = 0x1500;
	*(vuint16*)0x9C40000 = 0x1500;
	*(vuint16*)0x9FC0000 = 0x1500;

	return (vuint16*)0x8000000;
}
#endif
//RawMem
static vuint16 *_raw_unlock(){return (vuint16*)0x08000000;}
static void _raw_lock(){}

//DSi
//static vuint16 *_dsi_unlock(){return (vuint16*)0x02380000;}
//static void _dsi_lock(){}

//===================================//
//			  Ram API !			//
//===================================//
typedef enum                {DETECT_RAM=0, RAW_RAM, DSi_RAM, SC_RAM,      M3_RAM, OPERA_RAM, G6_RAM, EZ4_RAM, EZ3in1_RAM, EZnew_RAM,     EZplus_RAM,     M3Ext_RAM} RAM_TYPE;
static const sint8 *_types[]={"Unknown",    "Raw",   "DSi",   "Supercard", "M3",   "DSBM",    "G6",   "EZ3/4", "EZ 3in1",  "EZ 3in1 new", "EZ 3in1 plus", "M3Ext"};

static vuint16* (*_unlock)() = 0;
static void (*_lock)() = 0;
static uint32 _size = 0;
static uint32 _type = DETECT_RAM;
vuint16 *extmem;

static bool _ram_test(){
	extmem = _unlock();

	extmem[0] = 0x1234;
	if(extmem[0] == 0x1234)		// test writability
	{
		if(_type==RAW_RAM)return true; //don't need to test non-write
		_lock();

		extmem[0] = 0x4321;
		if(extmem[0] != 0x4321)	// test non-writability
		{
			return true;
		}
		return false;
	}

	if(_lock==_ez_lock)_lock(); //make sure CloseNorWrite() if attempting EZ
	return false;
}

static void _ram_precalc_size(){
	if(_unlock == 0 || _lock == 0)
		return;
		
	extmem = _unlock();

#if 0
	_size = 0;
	extmem[0] = 0x2468;
	while(extmem[_size] == 0x2468)
	{
		extmem[_size] = 0;
		_size += 512;
		extmem[_size] = 0x2468;
	}
#endif

	extmem[0]=1;
	for(_size=512;;_size+=512){
		extmem[_size]=(_size>>9)+1;
		if(extmem[_size]!=(_size>>9)+1 || extmem[0]!=1)break; //overwriting extmem[0] means that you reached mirrored area.
		extmem[_size]=0;
	}
	extmem[0]=0;
	_size<<=1;

	//if(_type!=RAW_RAM)_lock();
}

vuint16* ram_init(){
#if 0
	if(IPCZ->NDSType>=NDSi){
		if(*(vuint32*)0x04004000){
			_unlock = _dsi_unlock;
			_lock   = _dsi_lock;
			_type   = DSi_RAM;
			_size   = 0x00c00000; //Use 0x02380000-0x02f80000 as ExtRam in DSi Mode.
			return (extmem=_unlock());
		}
		return NULL; //DS Mode, but we need to guard access to 0x08000000.
	}
#endif

	setCpuNdsBusAccessPrio(NDSSLOT_ARM9BUS);
	setCpuGbaBusAccessPrio(GBASLOT_ARM9BUS);
	
	
	uint32 type=DETECT_RAM;
	switch(type)
	{
#if 0
		case SC_RAM:
		{
			_unlock = _sc_unlock;
			_lock   = _sc_lock;
			_type   = SC_RAM;
		}
		break;

		case M3_RAM:
		{
			_unlock = _m3_unlock;
			_lock   = _m3_lock;
			_type   = M3_RAM;
		}
		break;

		case OPERA_RAM:
		{
			_unlock = _opera_unlock;
			_lock   = _opera_lock;
			_type   = OPERA_RAM;
		}
		break;

		case G6_RAM:
		{
			_unlock = _g6_unlock;
			_lock   = _g6_lock;
			_type   = G6_RAM;
		}
		break;

		case EZ_RAM:
		{
			_unlock = _ez_unlock;
			_lock   = _ez_lock;
			_type   = EZ_RAM;
		}
		break;
#endif

		case DETECT_RAM:
		default:
		{
			_unlock = _raw_unlock;
			_lock   = _raw_lock;
			_type   = RAW_RAM;

			if(_ram_test())
			{
				break;
			}

			// try ez
			_unlock = _ez4_unlock;
			_lock   = _ez_lock;
			_type   = EZ4_RAM;
			
			if(_ram_test())
			{
				break;
			}

			_unlock = _ez3in1_unlock;
			_lock   = _ez_lock;
			_type   = EZ3in1_RAM;

			if(_ram_test())
			{
				break;
			}
#if 0
			_unlock = _eznew_unlock;
			_lock   = _ez_lock;
			_type   = EZnew_RAM;
			
			if(_ram_test())
			{
				break;
			}

			_unlock = _ezplus_unlock;
			_lock   = _ez_lock;
			_type   = EZplus_RAM;
			
			if(_ram_test())
			{
				break;
			}
#endif
			// try supercard
			_unlock = _sc_unlock;
			_lock   = _sc_lock;
			_type   = SC_RAM;

			if(_ram_test())
			{
				break;
			}

			// try m3
			_unlock = _m3_unlock;
			_lock   = _m3_lock;
			_type   = M3_RAM;

			if(_ram_test())
			{
				break;
			}

			// try opera
			_unlock = _opera_unlock;
			_lock   = _opera_lock;
			_type   = OPERA_RAM;

			if(_ram_test())
			{
				break;
			}

			// try g6
			_unlock = _g6_unlock;
			_lock   = _g6_lock;
			_type   = G6_RAM;
			
			if(_ram_test())
			{
				break;
			}

			// try m3ext
			_unlock = M3ExtPack_Start;
			_lock   = M3ExtPack_InitReadOnly;
			_type   = M3Ext_RAM;

			if(_ram_test())
			{
				break;
			}

			// fail
			_unlock = 0;
			_lock   = 0;
			_type   = DETECT_RAM;
			_size   = 0;
			extmem  = NULL;

			return NULL;
		}
		break;
	}
	
	_ram_precalc_size();
	
	return extmem; //from here it is safe to refer to extmem variable
}


uint32 ram_type(){return _type;}
const sint8* ram_type_string(){return _types[_type];}
uint32 ram_size(){return _size;}

vuint16* ram_unlock(){
	
	setCpuNdsBusAccessPrio(NDSSLOT_ARM9BUS);
	setCpuGbaBusAccessPrio(GBASLOT_ARM9BUS);
	
	if(_unlock)return (extmem=_unlock());
	return 0;
}

void ram_lock(){
	setCpuNdsBusAccessPrio(NDSSLOT_ARM9BUS);
	setCpuGbaBusAccessPrio(GBASLOT_ARM9BUS);

	if(_lock)_lock();
	extmem=NULL;
}

#if 0
/// MoonShell extmem wrapper API ///
typedef struct {
  uint32 StartAdr,EndAdr,CurAdr;
  uint32 *pSlot;
  uint32 SlotCount;
} TBODY;

static TBODY tBody;
static int init=0;
void extmem_Init(){
	if(!init){memset(&tBody,0,sizeof(tBody));ram_init();init=1;}
	if(_type==DETECT_RAM)return;

	tBody.StartAdr=(uint32)extmem;
	tBody.EndAdr=tBody.StartAdr+_size;
	tBody.CurAdr=tBody.StartAdr;
	tBody.pSlot=NULL;
	tBody.SlotCount=0;
}

void extmem_Free(){
	if(_type==DETECT_RAM)return;

	if(tBody.pSlot!=NULL){
		safefree(tBody.pSlot);
		tBody.pSlot=NULL;
	}
	tBody.EndAdr=tBody.StartAdr+_size;
	tBody.CurAdr=tBody.StartAdr;
	tBody.SlotCount=0;
}

bool extmem_ExistMemory(){
	if(_type==DETECT_RAM)return false;
	return true;
}

uint32 extmem_GetMemSize(){
	return ram_size();
}

void extmem_SetCount(uint32 Count){
	if(_type==DETECT_RAM)return;

	if(tBody.pSlot!=NULL){
		_consolePrint("extmem_SetSlotCount: Already allocated pSlot.\n");
		die();
	}
	  
	tBody.pSlot=(uint32*)safemalloc(sizeof(uint32)*Count);
	MemSet32DMA3(0,tBody.pSlot,sizeof(uint32)*Count);
	tBody.SlotCount=Count;
}

bool extmem_Exists(uint32 SlotIndex){
	if(_type==DETECT_RAM)return false;

	if(tBody.SlotCount<=SlotIndex){
		_consolePrintf("extmem_Exists(uint32 SlotIndex=%d); SlotCount==%d limit error.\n",SlotIndex,tBody.SlotCount);
		return false;
	}
	if(tBody.pSlot[SlotIndex]==0)return false;
	return(true);
}

bool extmem_Alloc(uint32 SlotIndex,uint32 Size){
	if(_type==DETECT_RAM)return false;

	if(tBody.SlotCount<=SlotIndex){
		_consolePrintf("extmem_Alloc(uint32 SlotIndex=%d,uint32 Size=%d); SlotCount==%d limit error.\n",SlotIndex,Size,tBody.SlotCount);
		return false;
	}
	  
	Size=align2(Size);
	  
	uint32 TermAdr=tBody.CurAdr+Size;
	  
	if(tBody.EndAdr<TermAdr){
// 		_consolePrintf("extmem_Alloc(uint32 SlotIndex=%d,uint32 Size=%d); MemAlloc limit error.\n",SlotIndex,Size);
		return false;
	}
	  
	tBody.pSlot[SlotIndex]=tBody.CurAdr;
	tBody.CurAdr=TermAdr;

	return true;
}

bool extmem_Write(uint32 SlotIndex,void *pData,uint32 DataSize){
	if(_type==DETECT_RAM)return false;

	if(tBody.SlotCount<=SlotIndex){
		_consolePrintf("extmem_Write(uint32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,tBody.SlotCount);
		return false;
	}
	  
	uint16 *pSrcData=(uint16*)pData;
	uint32 SrcSize=DataSize;
	uint16 *pDstData=(uint16*)tBody.pSlot[SlotIndex];
	uint32 DstSize=align2(SrcSize);
	  
	if(pDstData==NULL){
		_consolePrintf("extmem_Write(uint32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
		return false;
	}

	MemCopy16DMA3(pSrcData,pDstData,DstSize);
	return true;
}

bool extmem_Read(uint32 SlotIndex,void *pData,uint32 DataSize){
	if(_type==DETECT_RAM)return false;

	if(tBody.SlotCount<=SlotIndex){
		_consolePrintf("extmem_Read(uint32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,tBody.SlotCount);
		return false;
	}

	uint16 *pSrcData=(uint16*)tBody.pSlot[SlotIndex];
//	uint32 SrcSize=align2(Size);
	uint16 *pDstData=(uint16*)pData;
	uint32 DstSize=DataSize;

	if(pSrcData==NULL){
		_consolePrintf("extmem_Read(uint32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
		return(false);
	}

	MemCopy16DMA3(pSrcData,pDstData,DstSize);
	return true;
}
#endif

#endif