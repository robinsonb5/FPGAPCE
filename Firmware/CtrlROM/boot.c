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


#define BIT_SCANLINES 1

#define DEFAULT_DIPSWITCH_SETTINGS 0

void SetVolumes(int v);


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

enum boot_settings {BOOT_IGNORESETTINGS,BOOT_LOADSETTINGS,BOOT_SAVESETTINGS};

static int Boot(enum boot_settings settings)
{
	int result=0;
	int opened;

	OSD_Puts("Initializing SD card\n");
	if(spi_init())
	{
		int dipsw=GetDIPSwitch();

		if(!FindDrive())
			return(0);

		HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_SDCARD;	// Release reset but steal SD card

		if((opened=FileOpen(&file,"FPGAPCE CFG")))	// Do we have a configuration file?
		{
			if(settings==BOOT_SAVESETTINGS)	// If so, are we saving to it, or loading from it?
			{
				int i;
				int *p=(int *)sector_buffer;
				*p++=dipsw;
				for(i=0;i<126;++i)	// Clear remainder of buffer
					*p++=0;
				FileWrite(&file,sector_buffer);
			}
			else if(settings==BOOT_LOADSETTINGS)
			{
				FileRead(&file,sector_buffer);
				dipsw=*(int *)(&sector_buffer[0]);
				HW_HOST(HW_HOST_SW)=dipsw;
				SetDIPSwitch(dipsw);
	//				printf("DIP %d, Vol %d\n",dipsw,GetVolumes());
			}
		}

		if((opened=FileOpen(&file,"MSX3BIOSSYS")))	// Try and load MSX3 BIOS first
		{
			OSD_Puts("Loading ROM\n");
			int filesize=file.size;
			unsigned int c=0;
			int bits;

			bits=0;
			c=filesize;
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

			while(filesize>0)
			{
				OSD_ProgressBar(c,bits);
				if(FileRead(&file,sector_buffer))
				{
					int i;
					int *p=(int *)&sector_buffer;
					for(i=0;i<(filesize<512 ? filesize : 512) ;i+=4)
					{
						int t=*p++;
						int t1=t&255;
						int t2=(t>>8)&255;
						int t3=(t>>16)&255;
						int t4=(t>>24)&255;
						HW_HOST(HW_HOST_BOOTDATA)=t4;
						HW_HOST(HW_HOST_BOOTDATA)=t3;
						HW_HOST(HW_HOST_BOOTDATA)=t2;
						HW_HOST(HW_HOST_BOOTDATA)=t1;
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
			HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_BOOTDONE;	// Release SD card and early-terminate any remaining requests for boot data
			return(1);
		}
	}
	return(0);
}


static void doreset(enum boot_settings s)
{
	Menu_Hide();
	OSD_Clear();
	OSD_Show(1);
	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_RESET;	// Put OCMS into Reset
	PS2Wait();
	PS2Wait();
	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_SDCARD;	// Release reset but steal SD card
	Boot(s);
	Menu_Set(topmenu);
	OSD_Show(0);
}

static void savereset()
{
	doreset(BOOT_SAVESETTINGS);
}

static void reset()
{
	doreset(BOOT_IGNORESETTINGS);
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
	{MENU_ENTRY_CALLBACK,"Save and Reset",MENU_ACTION(&savereset)},
	{MENU_ENTRY_CYCLE,(char *)video_labels,2},
	{MENU_ENTRY_TOGGLE,"Scanlines",BIT_SCANLINES},
	{MENU_ENTRY_CYCLE,(char *)cart_labels,2},
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
	m=&topmenu[3];
	 	if(MENU_CYCLE_VALUE(m))
			result|=4;	// Cartridge type
	return(result);
}


int main(int argc,char **argv)
{
	int i;
	SetDIPSwitch(DEFAULT_DIPSWITCH_SETTINGS);
	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_RESET;	// Put OCMS into Reset
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


	if(Boot(BOOT_LOADSETTINGS))
	{
		OSD_Show(0);
		Menu_Set(topmenu);
		while(1)
		{
			int visible;
			static int prevds;

			HandlePS2RawCodes();
			visible=Menu_Run();
			HW_HOST(HW_HOST_SW)=GetDIPSwitch();
			if(GetDIPSwitch()!=prevds)
			{
				int i;
				prevds=GetDIPSwitch();
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
	}
	else
	{
		OSD_Puts("Loading BIOS failed\n");
	}
	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_BOOTDONE;	// Release SD card and early-terminate any remaining requests for boot data

	return(0);
}

