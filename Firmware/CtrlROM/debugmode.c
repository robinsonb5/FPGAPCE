#include "osd.h"
#include "menu.h"
#include "keyboard.h"

#include "debugmode.h"

static void showhex(unsigned int t)
{
	int i;
	for(i=0;i<8;++i)
	{
		unsigned int t2=(t>>28);
		t2+='0';
		if(t2>'9')
			t2+='@'-'9';
		OSD_Putchar(t2);
		t<<=4;
	}
}

int debugmode(int row)
{
	int counter=1;
	int history1_low;
	int history1_high;
	int history2_low;
	int history2_high;
	int history3_low;
	int history3_high;
	int history4_low;
	int history4_high;

	OSD_Clear();
	while(!TestKey(KEY_ESC))
	{
		int v;
		--counter;
		if(!counter)
		{
			counter=1000;
			history1_low=0xffffffff;
			history2_low=0xffffffff;
			history3_low=0xffffffff;
			history4_low=0xffffffff;
			history1_high=0;
			history2_high=0;
			history3_high=0;
			history4_high=0;
		}
		OSD_SetX(0);
		OSD_SetY(0);

		v=HW_DEBUG(REG_DEBUG1);
		history1_low&=v;
		history1_high|=v;
		showhex(v);
		OSD_Putchar(' ');
		showhex(history1_low);
		OSD_Putchar(' ');
		showhex(history1_high);
		OSD_Putchar('\n');

		v=HW_DEBUG(REG_DEBUG2);
		history2_low&=v;
		history2_high|=v;
		showhex(v);
		OSD_Putchar(' ');
		showhex(history2_low);
		OSD_Putchar(' ');
		showhex(history2_high);
		OSD_Putchar('\n');

		v=HW_DEBUG(REG_DEBUG3);
		history3_low&=v;
		history3_high|=v;
		showhex(v);
		OSD_Putchar(' ');
		showhex(history3_low);
		OSD_Putchar(' ');
		showhex(history3_high);
		OSD_Putchar('\n');

		v=HW_DEBUG(REG_DEBUG4);
		history4_low&=v;
		history4_high|=v;
		showhex(v);
		OSD_Putchar(' ');
		showhex(history4_low);
		OSD_Putchar(' ');
		showhex(history4_high);
		OSD_Putchar('\n');
		HandlePS2RawCodes();
	}
	Menu_Draw();
}

