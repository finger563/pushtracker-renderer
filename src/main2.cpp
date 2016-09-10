#include <stdint.h>
#include "MAPPERS.H"
#include "TEXTURE.H"

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "SSP.h"
#include "ADXL345.h"
#include "OLED.h"
#include "sonic.h"
//#include "sonicr.h"
//#include "sonicr1.h"
//#include "sonicr2.h"
//#include "sonicrunr3.h"
//#include "sonicrunr4.h"
//#include "sonicrunr5.h"
//#include "sonicrunr6.h"
//#include "sonicrunr7.h"
//#include "sonicrunr8.h"
//#include "sonicdashr1.h"
//#include "sonicdashr2.h"
//#include "sonicdashr3.h"
//#include "sonicdashr4.h"
//#include "tiles.h"
//#include "tiles_scrap.h"
#include "renderTiles.h"

extern unsigned char *disp_buff;

}

#define BG_CLR  0xFF

void bootup(void);
void PllFeed(void);

extern "C" int    main( void );

extern "C" void abort(void)
{
  while (1 == 1)
  {}
} // abort

extern "C" void Draw32x32(int x,int y,int index)
{

   const u8 *dsp = tiles32x32;
   int size32x32 = 4*4;
   dsp += index*size32x32;
   int size8x8 = 8*8;
   const u8 *ptr;
   for (int j = 0;j<size32x32;j++)
   {
        ptr = &tiles8x8[(*dsp)*(size8x8)];
        blit(((j%4)*8)+x,(j/4)*8+y,ptr,8,8,0x7C);
        dsp++;
   }
}

extern "C" void draw_bg(int x,int y)
{
   FillScreen(BG_CLR);
   int index = tilesmap[x/256 + (y/256)*32];
   bool flagx = (x%256)>128;   
   bool flagy = (y%256)>128;   
   if (flagx);                 // need to show another tile to the right
   if (flagy);                 // need to show another tile on the bottom
   if (flagx&&flagy);          // need to show another tile to the bottom right
   const u8 *dsp = tiles256x256;
   int size256x256 = 8*8;
   dsp += index*size256x256 + (x%256)/32 + ((y%256)/32)*8;
   for (int j = 0;j<4;j++)
   {
      for (int i=0;i<4;i++)
      {
          Draw32x32(i*32-x,j*32-y,*dsp);
          dsp++;
      }
      dsp += 4;
   }
}


extern "C" void tile_bg(int x, int y)
{
   u8 set[32*4];
   u8 *ptr = set;
   for (int i=0;i<128;i++)
   {
//       GetLine(x,y+i,ptr);
       ScanLine(i,ptr);
   }
}

extern "C" void draw_sprite(const u8 *sprt)
{
//        blit((j*8)%128,(j/16)*8,sprt,8,8,0x2E);
//        dsp += 8*8;
}

int main( void )
{
   bootup();

   OLED_init();
   
    InitTiles();

  while (1)
  {
//      for (int i=0;i<64;i++)
//      {
//        for (int j=0;j<10;j++)
//        {
            draw_bg(512+128,256);  
            TileMap(50,50,sonic+(1%7)*30,30,47,210,0x2E);
            BufferToScreen(0,0,127,127);
            delay_ms(5);
//        }
//      }
  }
   
   return 0;
} // main

void bootup(void)
{
    SCS = 3;
  
   // Setting Multiplier and Divider values
	PLLCFG=0x25; // M = 6
   PllFeed();

   // Enabling the PLL */
   PLLCON=0x1;
   PllFeed();

#define PLOCK 0x400
   // Wait for the PLL to lock to set frequency
   while(!(PLLSTAT & PLOCK)) ;

   // Connect the PLL as the clock source
   PLLCON=0x3;
   PllFeed();

   // Enabling MAM and setting number of clocks used for Flash memory fetch (4 cclks in this case)
   //MAMTIM=0x3; //VCOM?
   MAMCR=0x2;
   MAMTIM=0x4; //Original

   // Setting peripheral Clock (pclk) to System Clock (cclk)
   VPBDIV=0x1;

	FIO0DIR |= (1<<STAT0)|(1<<STAT1);
	FIO0SET = (1<<STAT0)|(1<<STAT1);

	S0SPCR = 0x08;  // SPI clk to be pclk/8
	S0SPCR = 0x30;  // master, msb, first clk edge, active high, no ints

} // bootup

void PllFeed(void)
{
   PLLFEED=0xAA;
   PLLFEED=0x55;
} // PllFeed