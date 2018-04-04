#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "frontend.h"

#include "diskio.h"
#include "ff.h"

#include "typedefsTGDS.h"
#include "posixHandleTGDS.h"
#include "fsfatlayerTGDS.h"
#include "utilsTGDS.h"



static uint8 buf[768]; //lol

typedef uint16 UnicodeChar;
#define ExtLinkBody_MaxLength (256)
#define ExtLinkBody_ID (0x30545845) // EXT0
typedef struct {
  uint32 ID,dummy1,dummy2,dummy3; // dummy is ZERO.
  sint8 DataFullPathFilenameAlias[ExtLinkBody_MaxLength];
  sint8 DataPathAlias[ExtLinkBody_MaxLength];
  sint8 DataFilenameAlias[ExtLinkBody_MaxLength];
  sint8 NDSFullPathFilenameAlias[ExtLinkBody_MaxLength];
  sint8 NDSPathAlias[ExtLinkBody_MaxLength];
  sint8 NDSFilenameAlias[ExtLinkBody_MaxLength];
  UnicodeChar DataFullPathFilenameUnicode[ExtLinkBody_MaxLength];
  UnicodeChar DataPathUnicode[ExtLinkBody_MaxLength];
  UnicodeChar DataFilenameUnicode[ExtLinkBody_MaxLength];
  UnicodeChar NDSFullPathFilenameUnicode[ExtLinkBody_MaxLength];
  UnicodeChar NDSPathUnicode[ExtLinkBody_MaxLength];
  UnicodeChar NDSFilenameUnicode[ExtLinkBody_MaxLength];
} TExtLinkBody;


sint8* myfgets(sint8 *buf,int n,FILE *fp){ //accepts LF/CRLF
	sint8 *ret=fgets(buf,n,fp);
	if(!ret)return NULL;
	if(strlen(buf)&&buf[strlen(buf)-1]=='\n')buf[strlen(buf)-1]=0;
	if(strlen(buf)&&buf[strlen(buf)-1]=='\r')buf[strlen(buf)-1]=0;
	return ret;
}


void SplitItemFromFullPathAlias(const sint8 *pFullPathAlias,sint8 *pPathAlias,sint8 *pFilenameAlias){
	uint32 SplitPos=0;
	{
		uint32 idx=0;
		while(1){
			sint8 uc=pFullPathAlias[idx];
			if(uc==0) break;
			if(uc=='/') SplitPos=idx+1;
			idx++;
		}
	}

	if(pPathAlias){
		if(SplitPos<=1){
			pPathAlias[0]='/';
			pPathAlias[1]=0;
		}else{
			uint32 idx=0;
			for(;idx<SplitPos-1;idx++){
				pPathAlias[idx]=pFullPathAlias[idx];
			}
			pPathAlias[SplitPos-1]=0;
		}
	}
	if(pFilenameAlias)strcpy(pFilenameAlias,&pFullPathAlias[SplitPos]);
}

bool _readFrontend(sint8 *target){
	
	
	//FIL f;
	FILE *f=fopen(getfatfsPath("loadfile.dat"),"r");
	//if(f_open(&f,"/loadfile.dat",FA_READ) == FR_OK){
	if(f){
		int i=0;
		myfgets((sint8*)buf,768,f);
		//myfgets((sint8*)buf,768,&f);
		fclose(f);
		//f_close(&f);
		
		fatfs_unlink(getfatfsPath("loadfile.dat"));	//unlink("/loadfile.dat");
		//f_unlink("/loadfile.dat");
		
		//if(!memcmp((sint8*)buf,"fat:",4))i+=4;
		if(!memcmp((sint8*)buf+i,"//",2))i+=1;
		if(!memcmp((sint8*)buf+i,"/./",3))i+=2; //menudo dir handling is buggy?
		strcpy(target,(sint8*)buf+i);
		if(strlen(target)<4||(strcasecmp(target+strlen(target)-4,".smc")&&strcasecmp(target+strlen(target)-4,".sfc")))return false;
		return true;
	}
	f=fopen(getfatfsPath("plgargs.dat"),"r");
	//if(f_open(&f,"/plgargs.dat",FA_READ) == FR_OK){
	if(f){
		int i=0;
		myfgets((sint8*)buf,768,f);
		//myfgets((sint8*)buf,768,&f);
		myfgets((sint8*)buf,768,f); //second line
		//myfgets((sint8*)buf,768,&f); //second line
		
		fclose(f);
		//f_close(&f);
		
		fatfs_unlink(getfatfsPath("plgargs.dat"));	//unlink("/plgargs.dat");
		//f_unlink("/plgargs.dat");
		
		//if(!memcmp((sint8*)buf,"fat:",4))i+=4;
		//if(!memcmp((sint8*)buf+i,"//",2))i+=1;
		//if(!memcmp((sint8*)buf+i,"/./",3))i+=2;
		strcpy(target,(sint8*)buf+i);
		if(strlen(target)<4||(strcasecmp(target+strlen(target)-4,".smc")&&strcasecmp(target+strlen(target)-4,".sfc")))return false;
		return true;
	}
	f=fopen(getfatfsPath("moonshl2/extlink.dat"),"r+b");
	//if(f_open(&f,"/moonshl2/extlink.dat",FA_READ | FA_WRITE) == FR_OK){
	if(f){
		TExtLinkBody extlink;
		memset(&extlink,0,sizeof(TExtLinkBody));
		
		fread(&extlink,1,sizeof(TExtLinkBody),f);
		//unsigned int read_so_far;
		//f_read(&f, &extlink, sizeof(TExtLinkBody), &read_so_far);
		
		if(extlink.ID!=ExtLinkBody_ID){
			fclose(f);
			//f_close(&f);
			return false;
		}
		
		//strcpy(target,extlink.DataFullPathFilenameAlias);
		
		ucs2tombs((uint8*)target,extlink.DataFullPathFilenameUnicode,768);
		
		fseek(f,0,SEEK_SET);
		//f_lseek (&f, 0);
		
		fwrite((void*)"____",1,4,f);
		//unsigned int written;
		//f_write(&f, "____", 4, &written);
		//f_truncate(&f);
		
		fclose(f);
		//f_close(&f);
		
		if(strlen(target)<4||(strcasecmp(target+strlen(target)-4,".smc")&&strcasecmp(target+strlen(target)-4,".sfc")))return false;
		return true;
	}
	return false; //it is your choice to boot GUI or to halt.
}

extern int argc;
extern sint8 **argv;
static sint8 target[768],name[768],dir[768];
bool readFrontend(sint8 **_name, sint8 **_dir)
{
	*_name=NULL;
	*_dir=NULL;
	if(argc>1){
		strcpy(target,argv[1]);
	}else{
		if(!_readFrontend(target))return false;
	}
	SplitItemFromFullPathAlias(target,dir,name);
	//chdir(dir);
	*_name=name;
	*_dir=dir;
	return true;
}

