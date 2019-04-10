#ifndef DEBUGMODE_H
#define DEBUGMODE_H

int debugmode(int row);

#define DEBUGBASE 0xFFFFFA00
#define HW_DEBUG(x) *(volatile unsigned int *)(DEBUGBASE+x)

#define REG_DEBUG1 0x0
#define REG_DEBUG2 0x4
#define REG_DEBUG3 0x8
#define REG_DEBUG4 0xc

#endif

