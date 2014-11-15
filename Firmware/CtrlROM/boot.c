/*	Firmware for loading files from SD card.
	Part of the ZPUTest project by Alastair M. Robinson.
	SPI and FAT code borrowed from the Minimig project.
*/


#include "stdarg.h"

#include "uart.h"
#include "spi.h"
#include "minfat.h"
#include "small_printf.h"
#include "host.h"
#include "ps2.h"
#include "keyboard.h"
#include "hexdump.h"
#include "osd.h"
#include "menu.h"

fileTYPE file;
static struct menu_entry topmenu[];
int dipswitch;

int multitap;
#define DEFAULT_DIPSWITCH_SETTINGS HW_HOST_SWF_MULTITAP

void SetVolumes(int v);
int GetDIPSwitch();


void OSD_Puts(char *str)
{
	int c;
	while((c=*str++))
		OSD_Putchar(c);
}


void WaitEnter()
{
	while(1)
	{
		HandlePS2RawCodes();
		if(TestKey(KEY_ENTER)&2)
			return;
	}
}


void LoadSettings()
{
	if(FileOpen(&file,"FPGAPCE CFG"))	// Do we have a configuration file?
	{
		FileRead(&file,sector_buffer);
		dipswitch=*(int *)(&sector_buffer[0]);
		HW_HOST(HW_HOST_SW)=dipswitch;
		SetDIPSwitch(dipswitch);
	}
}


void SaveSettings(int row)
{
	if(FileOpen(&file,"FPGAPCE CFG"))	// Do we have a configuration file?
	{
		int i;
		int *p=(int *)sector_buffer;
		*p++=dipswitch;
		for(i=0;i<126;++i)	// Clear remainder of buffer
			*p++=0;
		FileWrite(&file,sector_buffer);
	}
}


static int LoadROM(const char *filename)
{
	int result=0;
	int opened;

	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_RESET;	// Put core into Reset
	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_SDCARD;	// Release reset but steal SD card

	if((opened=FileOpen(&file,filename)))
	{
		int filesize=file.size;
		unsigned int c=0;
		int bits;

		bits=0;
		c=filesize-1;
		while(c)
		{
			++bits;
			c>>=1;
		}
		bits-=9;

		if((filesize&0xfff)) // Do we have a header?
		{
			filesize-=512;
			FileNextSector(&file);	// Skip the header		
		}

		dipswitch&=~HW_HOST_SWF_ROMSPLIT;
		if(filesize==(768*1024))
			dipswitch|=HW_HOST_SWF_ROMSPLIT;

		HW_HOST(HW_HOST_SW)=dipswitch;

		while(filesize>0)
		{
			OSD_ProgressBar(c,bits);
			if(FileRead(&file,sector_buffer))
			{
				int i;
				int *p=(int *)&sector_buffer;
				for(i=0;i<(filesize<512 ? filesize : 512) ;i+=4)
				{
					unsigned int t=*p++;  // AABBCCDD
					unsigned int t1=((t&0xff00)>>8)|((t&0xff)<<8); // DDCC
					unsigned int t2=((t&0xff000000)>>24)|((t&0xff0000)>>8);  // BBAA
					HW_HOST(HW_HOST_BOOTDATA)=t2; // BBAA
					HW_HOST(HW_HOST_BOOTDATA)=t1; // DDCC
				}
			}
			else
			{
				OSD_Puts("Read failed\n");
				return(0);
			}
			FileNextSector(&file);
			filesize-=512;
			++c;
		}
		return(1);
	}
	OSD_Puts("Failed\n");
	return(0);
}


static void reset(int row)
{
	Menu_Hide();
	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_RESET;	// Put core into Reset
	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_BOOTDONE;	// release keyboard
}



static int romindex=0;
static int romcount;


static void selectrom(int row);
static void scrollroms(int row);

static char romfilenames[13][16];

static struct menu_entry rommenu[]=
{
	{MENU_ENTRY_CALLBACK,romfilenames[0],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[1],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[2],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[3],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[4],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[5],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[6],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[7],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[8],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[9],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[10],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[11],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_CALLBACK,romfilenames[12],MENU_ACTION(&selectrom)},
	{MENU_ENTRY_SUBMENU,"Back",MENU_ACTION(topmenu)},
	{MENU_ENTRY_NULL,0,MENU_ACTION(scrollroms)}
};


static void selectrom(int row)
{
	int i;
	char *fn=rommenu[row].label;
	if(fn && fn[0])
	{
//		OSD_Puts("Loading ");
//		OSD_Puts(fn);
		LoadROM(fn);
	}
	Menu_Set(topmenu);
	Menu_Hide();
	OSD_Show(0);
}


static void copyname(char *dst,const unsigned char *src)
{
	int i;
	for(i=0;i<11;++i)
		*dst++=*src++;
	*dst++=0;
}


static void listroms();

static void scrollroms(int row)
{
	switch(row)
	{
		case ROW_LINEUP:
			if(romindex)
				--romindex;
			break;
		case ROW_PAGEUP:
			romindex-=16;
			if(romindex<0)
				romindex=0;
			break;
		case ROW_LINEDOWN:
			++romindex;
			break;
		case ROW_PAGEDOWN:
			romindex+=16;
			break;
	}
	listroms();
	Menu_Draw();
}


static void listroms()
{
	int i,j;
	j=0;
	for(i=0;(j<romindex) && (i<dir_entries);++i)
	{
		DIRENTRY *p=NextDirEntry(i);
		if(p)
			++j;
	}

	for(j=0;(j<12) && (i<dir_entries);++i)
	{
		DIRENTRY *p=NextDirEntry(i);
		if(p)
			copyname(romfilenames[j++],p->Name);
		else
			romfilenames[j][0]=0;
	}
}


void load_pcengine(int row)
{
	dipswitch&=~HW_HOST_SWF_BITFLIP;
	selectrom(row);
}


void load_tg16(int row)
{
	dipswitch|=HW_HOST_SWF_BITFLIP;
	selectrom(row);
}


static struct hotkey hotkeys[]=
{
	{KEY_P,load_pcengine},
	{KEY_T,load_tg16},
	{0,0}
};



static void showrommenu(int row)
{
	romindex=0;
	listroms();
	Menu_Set(rommenu);
	Menu_SetHotKeys(hotkeys);
}


static struct menu_entry topmenu[];

static char *video_labels[]=
{
	"VGA - 31KHz, 60Hz",
	"TV - 480i, 60Hz"
};

static char *cart_labels[]=
{
	"PC Engine mode",
	"Turbografx 16 mode"
};


static struct menu_entry topmenu[]=
{
	{MENU_ENTRY_CALLBACK,"Reset",MENU_ACTION(&reset)},
	{MENU_ENTRY_CALLBACK,"Save settings",MENU_ACTION(&SaveSettings)},
	{MENU_ENTRY_CYCLE,(char *)video_labels,2},
	{MENU_ENTRY_TOGGLE,"Scanlines",HW_HOST_SWF_SCANLINES},
	{MENU_ENTRY_CYCLE,(char *)cart_labels,2},
	{MENU_ENTRY_CALLBACK,"Load ROM \x10",MENU_ACTION(&showrommenu)},
	{MENU_ENTRY_CALLBACK,"Exit",MENU_ACTION(&Menu_Hide)},
	{MENU_ENTRY_NULL,0,0}
};


int SetDIPSwitch(int d)
{
	struct menu_entry *m;
	MENU_TOGGLE_VALUES=d&2; // Scanlines
	m=&topmenu[2]; MENU_CYCLE_VALUE(m)=d&1; // Video mode
	m=&topmenu[3]; MENU_CYCLE_VALUE(m)=(d&4 ? 1 : 0); // Cartridge type
}


int GetDIPSwitch()
{
	struct menu_entry *m;
	int result=MENU_TOGGLE_VALUES&0x2; // Scanline
	int t;
	m=&topmenu[2];
	 	if(MENU_CYCLE_VALUE(m))
			result|=1;	// Video mode
	m=&topmenu[4];
	 	if(MENU_CYCLE_VALUE(m))
			result|=4;	// Cartridge type
	if(multitap)
		result|=HW_HOST_SWF_MULTITAP;

	return(result);
}


int main(int argc,char **argv)
{
	int i;
	multitap=1;
	SetDIPSwitch(DEFAULT_DIPSWITCH_SETTINGS);
	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_RESET;	// Put core into Reset
	HW_HOST(HW_HOST_SW)=DEFAULT_DIPSWITCH_SETTINGS;
	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_SDCARD;	// Release reset but steal SD card
	HW_HOST(HW_HOST_MOUSEBUTTONS)=3;

	PS2Init();
	EnableInterrupts();
	PS2Wait();
	PS2Wait();
	OSD_Clear();
	OSD_Show(1);	// Figure out sync polarity
	PS2Wait();
	PS2Wait();
	OSD_Show(1);	// OSD should now show correctly.

	OSD_Puts("Initializing SD card\n");
	i=5;
	while(--i>0)
	{
		spi_init();
		if(FindDrive())
			i=-1;
	}
	if(!i)	// Did we escape the loop?
	{
		OSD_Puts("Card init failed\n");
		return(0);
	}


	LoadSettings();

	if(LoadROM("BOOT    PCE"))
	{
		Menu_Set(topmenu);
		OSD_Show(0);
	}
	else	// If we couldn't load boot.pce then we drop into the file selector...
	{
		showrommenu(0);
		Menu_Show();
	}

	while(1)
	{
		int visible;

		HandlePS2RawCodes();
		visible=Menu_Run();
		HW_HOST(HW_HOST_GAMEPAD)=joya<<8|joyb;
		if(GetDIPSwitch()!=dipswitch)
		{
			int i;
			dipswitch=GetDIPSwitch();
			HW_HOST(HW_HOST_SW)=dipswitch;
			for(i=0;i<5;++i)
			{
				OSD_Show(visible);	// Refresh OSD position
				PS2Wait();
				PS2Wait();
			}
		}
		if(visible)
			HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_BOOTDONE|HW_HOST_CTRLF_KEYBOARD;	// capture keyboard
		else					
			HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_BOOTDONE;	// release keyboard
	}

	return(0);
}

