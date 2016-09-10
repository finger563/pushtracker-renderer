
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "OLED.h"
#include "ADXL345.h"
#include "board.h"
#include "main.h"

/* OLED Commands */
static const enum
{
	SSD1339ColAddr = 0x15,	// 2 data bytes: start, end addr (range=0-131; def=0,131)
	
	SSD1339WriteRAM = 0x5c,	// enable MCU write to RAM
	SSD1339ReadRAM = 0x5d,	// enable MCU read from RAM
	
	SSD1339RowAddr = 0x75,	// 2 data bytes: start, end addr (range=0-131; def=0,131)
	
	SSD1339DrawLine = 0x83,	// 6: colStart, rowStart, colEnd, rowEnd, color1, color2 (start < end < 132)
	SSD1339DrawRect = 0x84,	// 8: colStart, rowStart, colEnd, rowEnd, line1, line2, fill1, fill2 (start < end < 132)
	SSD1339DrawCirc = 0x86,	// 7: colCenter, rowCenter, radius, line1, line2, fill1, fill2
	SSD1339Copy = 0x8a,		// 6: colStart, rowStart, colEnd, rowEnd, colNew, rowNew (start < end < 132)
	SSD1339DimWin = 0x8c,	// 4: colStart, rowStart, colEnd, rowEnd (start < end < 132)
	SSD1339ClearWin = 0x8e,	// 4: colStart, rowStart, colEnd, rowEnd (start < end < 132)
	
	SSD1339DrawSettings = 0x92,
	SSD1339HScroll = 0x96,
	SSD1339StartScroll = 0x9f,
	SSD1339StopScroll = 0x9e,
	
	SSD1339Settings = 0xa0,	// 1 data byte (see doc) [bit3=1 for 9-bit bus] [bit7:6=01 for 65k color (def't)]
	SSD1339StartLine = 0xa1,	// 1 data byte: vertical scroll by RAM (range=0-131; def=0)
	SSD1339Offset = 0xa2,	// 1 data byte: vertical scroll by Row (range=0-131; def=0)
	SSD1339DispAllOff = 0xa4,
	SSD1339DispAllOn = 0xa5,		// all pixels have GS15
	SSD1339DispNormal = 0xa6,	// default
	SSD1339DispInverse = 0xa7,	// GS0->GS63, GS1->GS62, ...
	SSD1339Config = 0xad,	// 1 data byte (see doc) [1000 1_1_ => 1000 1110 = 0x8e]

	SSD1339SleepOn = 0xae,		// (display off)
	SSD1339SleepOff = 0xaf,		// (display on)

	SSD1339PowerSaving = 0xb0,	// 1 data byte (see doc) [00: internal VSL, normal (def't); 0x12: power saving internal VSL]
	SSD1339SetPeriod = 0xb1,	// 1 data byte: low nibble: phase 1 (reset) period [4]; high nibble: phase 2 [7](pre-charge) period; 1-16DCLK
	SSD1339Clock = 0xb3,		// 1 data byte: low nibble: divide by DIVSET+1 [0]; high nibble: osc freq [9? d?]
	SSD1339GrayPulseWidth = 0xb8,	// 32 bytes (see doc)
	SSD1339LinearLUT = 0xb9,		// reset to default (linear grayscale) Look Up Table (PW1=1, PW2=3, PW3=GRAPH_LEFT ... PW63=125)
	SSD1339PrechargeV = 0xbb,	// 3 bytes: colors A, B, C; def't=0x1c; 0: 0.51*Vref, 1f: 0.84*Vref, 0x80: Vcomh
	SSD1339SetVcomh = 0xbe,		// 1 byte: 0: 0.51*Vref, 1f: 0.84*Vref (def't)

	SSD1339ColorContrast = 0xc1,	// 3 bytes: colors A, B, C; def't=0x80 (sets current)
	SSD1339Contrast = 0xc7,		// 1 byte: reduce output current (all colors) to (low nibble+1)/16 [def't=0f]
	SSD1339MUXRatio = 0xca,	// 1 byte: 16MUX-132MUX (range=15-131; def=131)

	SSD1339NOP = 0xe3,

	SSD1339Lock = 0xfd,		// 1 byte: 0x12=unlock (def't); 0x16=lock
	//---------------------------------------------------
	// For the updated screen with SSD1351U driver
	
	SSD1351ColAddr = 0x15,	// 2 data bytes: start, end addr (range=0-131; def=0,131)
	
	SSD1351WriteRAM = 0x5c,	// enable MCU write to RAM
	SSD1351ReadRAM = 0x5d,	// enable MCU read from RAM
	
	SSD1351RowAddr = 0x75,	// 2 data bytes: start, end addr (range=0-131; def=0,131)

	SSD1351HScroll = 0x96,
	SSD1351StartScroll = 0x9f,
	SSD1351StopScroll = 0x9e,

	SSD1351Settings = 0xa0,	// 1 data byte (see doc) [bit3=reserved]
        //  [bit7:6=01 for 65k color, bit7:6=00 for 256 color, bit7:6=10 for 256k color (8/18), bit7:6=11 for 256k color (16)]	
        //  [bit2=0 A-B-C color sequence, bit2=1 C-B-A color sequence]
	SSD1351StartLine = 0xa1,	// 1 data byte: vertical scroll by RAM (range=0-131; def=0)

	SSD1351Offset = 0xa2,	// 1 data byte: vertical scroll by Row (range=0-131; def=0)
	SSD1351DispAllOff = 0xa4,
	SSD1351DispAllOn = 0xa5,		// all pixels have GS15
	SSD1351DispNormal = 0xa6,	// default
	SSD1351DispInverse = 0xa7,	// GS0->GS63, GS1->GS62, ...
	SSD1351Config = 0xAB,	
	
	SSD1351NOP = 0xAD,
	SSD1351SleepOn = 0xae,		// (display off)
	SSD1351SleepOff = 0xaf,		// (display on)

	SSD1351SetPeriod = 0xb1,	// 1 data byte: low nibble: phase 1 (reset) period [4]; high nibble: phase 2 [7](pre-charge) period; 1-16DCLK

	SSD1351SetPerformance = 0xb2,	// 3 data bytes 0,0,0 = normal (def't), A4h,0,0=enhance performance
	SSD1351Clock = 0xb3,		// 1 data byte: low nibble: divide by DIVSET+1 [0]; high nibble: osc freq [9? d?]

	SSD1351VSL = 0xB4,			// 3 data bytes:  A = 0b101000XX,  A1:A0 => 00: External VSL, 10: Internal VSL; B = 0b10110101; C = 0b01010101
	
	SSD1351GPIO = 0xB5, 		// A1:0 => GPIO0, A3:2 => GPIO1
	SSD1351SecondPeriod = 0xB6,	// A3:A0 => 0:invalid, 1: 1 DCLKS, 2: 2 DCLKS, 3: 3 Dclks,...15: 15 DCLKS
	SSD1351GrayPulseWidth = 0xb8,	// 63 bytes (see doc)
	SSD1351LinearLUT = 0xb9,		// reset to default (linear grayscale) Look Up Table (PW1=1, PW2=3, PW3=GRAPH_LEFT ... PW63=125)
	SSD1351PrechargeV = 0xbb,	// 1 byte: 0: 0.2*Vref, 1f: 0.6*Vcc, high 3 bits = 0  

	SSD1351SetVcomh = 0xbe,		// 1 byte: 0: 0.72*Vref, 1f: 0.84*Vref (def't)  A2:0,   rest are 0
	
	SSD1351ColorContrast = 0xc1,	// 3 bytes: colors A, B, C; def't=0x80 (sets current)
	SSD1351Contrast = 0xc7,		// 1 byte: reduce output current (all colors) to (low nibble+1)/16 [def't=0f]
	SSD1351MUXRatio = 0xca,	// 1 byte: 16MUX-128MUX (range=15-OLED_END; def=OLED_END)
	
	SSD1351Lock = 0xfd,		// 1 byte: 0x12=unlock (def't); 0x16=lock
} Commands;

char disp_buff[128];
float z_buff[128];


/*****************************************************************************
 *
 * Description:
 *    Initializes the OLED screen
 *
 * Returns:
 *    
 *
 ****************************************************************************/
void OLED_init(void)
{
	
	unsigned long _dummy;
	_dummy = PINSEL2;
	_dummy &= ~((1<<2)|(1<<3));
	PINSEL2 = _dummy;

#ifdef SSD1339
	FIO1DIR |= LCD_DATA|LCD_DC|LCD_RW|LCD_CS|LCD_RD|LCD_RSTB|BS1|BS2;
	
	#ifdef USE_SLOW_GPIO
	IODIR0 |= OLED_PWR;
	#else
	FIO0DIR |= OLED_PWR;
	#endif
	
	FIO1CLR = BS1;
	FIO1SET = BS2;
	
	OLED_ON;
	
	delay_ms(500);
	
	FIO1CLR = LCD_RD|LCD_CS|LCD_RW;

	delay_ms(20);
	FIO1CLR = LCD_RSTB;
	delay_ms(200);
	FIO1SET = LCD_RSTB;
	delay_ms(20);

	write_c(SSD1339Settings); // Set Re-map / Color Depth
        #ifdef INVERT
	write_d(0x34);//0xb4); // 262K 8bit R->G->B       // 4 to swap 0 to not swap
        #else
        write_d(0x30);//0xb4); // 262K 8bit R->G->B       // 4 to swap 0 to not swap
        #endif
	write_c(SSD1339StartLine); // Set display start line
	write_d(0x00); // 00h start
	//write_c(0xa2); // Set display offset
	//write_d(0x80); // 80h start
	write_c(SSD1339DispNormal); // Normal display
	write_c(SSD1339Config); // Setz Master Configuration
	write_d(0x8A); // DC-DC off & external VcomH voltage & external pre-charge voltage  // was 8e
	write_c(SSD1339PowerSaving); // Power saving mode
	write_d(0x05);
	write_c(SSD1339SetPeriod); // Set pre & dis_charge
	write_d(0x11); // pre=1h dis=1h
	write_c(SSD1339Clock); // clock & frequency
	write_d(0xf0); // clock=Divser+1 frequency=fh
	//write_c(SSD1339LinearLUT);
	write_c(SSD1339PrechargeV); // Set pre-charge voltage of color A B C
	write_d(0x1c); // color A
	write_d(0x1c); // color B
	write_d(0x1c); // color C
	write_c(SSD1339SetVcomh); // Set VcomH
	write_d(0x1f); //
	write_c(SSD1339ColorContrast); // Set contrast current for A B C
	write_d(0xC0); // Color A	
	write_d(0xC0); // Color B
	write_d(0xC0); // Color C
	write_c(SSD1339Contrast); // Set master contrast
	write_d(0x0f); // no change	
	write_c(SSD1339MUXRatio); // Duty
	write_d(127); // OLED_END+1
	write_c(SSD1339SleepOff); // Display on
	write_c(SSD1339DrawSettings);    // fill enable command
	write_d(0x01); 
#else

	FIO1DIR |= LCD_DATA|LCD_DC|LCD_RW|LCD_CS|LCD_RD|LCD_RSTB|BS0|BS1;
	#ifdef USE_SLOW_GPIO
	IODIR0 |= OLED_PWR;
	#else
	FIO0DIR |= OLED_PWR;
	#endif
	FIO1SET = BS1|BS0;
	
	OLED_OFF;
	delay_ms(500);
	
	FIO1CLR = LCD_RD|LCD_CS|LCD_RW;

	delay_ms(20);
	FIO1CLR = LCD_RSTB;
	delay_ms(200);
	FIO1SET = LCD_RSTB;
	delay_ms(20);
	write_c(SSD1351Lock);
	write_d(0x12);
	write_c(SSD1351Lock);
	write_d(0xB1);
	write_c(SSD1351SleepOn);
	write_c(SSD1351Clock); // clock & frequency
	write_d(0xF1); // clock=Divser+1 frequency=fh
	//ClearScreen();
	//BufferToScreen(0,0,127,127);
	write_c(SSD1351MUXRatio); // Duty
	write_d(127); // OLED_END+1
	write_c(SSD1351Settings); // Set Re-map / Color Depth
        //  [bit7:6=01 for 65k color, bit7:6=00 for 256 color, bit7:6=10 for 256k color (8/18), bit7:6=11 for 256k color (16)]	
        //  [bit2=0 A-B-C color sequence, bit2=1 C-B-A color sequence, c is red, a is blue]
	write_d(0xb4);//0xb4); 34		// 26 turns it upside down
	write_c(SSD1351ColAddr);
	write_d(0);
	write_d(127);
	write_c(SSD1351RowAddr);
	write_d(0);
	write_d(127);
	write_c(SSD1351StartLine); // Set display start line
	write_d(0x00); // 00h start
	write_c(SSD1351Offset); // Set display offset
	write_d(0x00); // 80h start
	write_c(SSD1351GPIO);
	write_d(0x00);
	write_c(SSD1351Config); // Set Master Configuration
	write_d(0x01); // internal VDD
	write_c(SSD1351SetPeriod); // Set pre & dis_charge
	write_d(0x32); 		// 32
	write_c(SSD1351VSL);
	write_d(0xA0);		// A1:A0 => 00: External VSL, 10: Internal VSL
	write_d(0xB5);
	write_d(0x55);
	write_c(SSD1351PrechargeV); // Set pre-charge voltage of color A B C
	write_d(0x17);		//	-- reset = 0x17
	write_c(SSD1351SetVcomh); // Set VcomH
	write_d(0x05); // reset value 0x05
	write_c(SSD1351ColorContrast); // Set contrast current for A B C
	write_d(0xC8); // Color A	-- default
	write_d(0x80); // Color B
	write_d(0xC8); // Color C
	write_c(SSD1351SetPerformance);
	write_d(0xA4);
	write_d(0x00);
	write_d(0x00);
	write_c(SSD1351Contrast); // Set master contrast
	write_d(0x0f); // no change		-- was 0x0f
	write_c(SSD1351SecondPeriod);
	write_d(0x01);
	write_c(SSD1351DispNormal); // Normal display
	OLED_ON;
	write_c(SSD1351SleepOff); // Display on
#endif
}

void OLED_ShutDown(void)
{
	write_c(SSD1351SleepOn);
	delay_ms(100);
	OLED_OFF;
	delay_ms(500);
}

void OLED_TurnOn(void)
{
	OLED_ON;
	delay_ms(500);
	write_c(SSD1351SleepOff);
	delay_ms(100);
}

void ClearBuff(void* buff)
{
        memset(buff,0,128);
}

void BufferToScreen(int y)
{
	write_c(0x15);	// set column start and end addresses
	write_d(0);
	write_d(127);
	write_c(0x75);	// set row start and end adresses
	write_d(y);
	write_d(y+1);
	write_c(0x5C);	// write to RAM command
        int i;
        for (i=0;i<128;i++)
                //write_262k(disp_buff[i]);
                write_d(disp_buff[i]);
}

//==============================================================================================
//================================  I/O Functions  =============================================
//==============================================================================================


void write_c(unsigned char out_command) {	
	//FIO1DIR |= LCD_DATA;
	FIO1PIN2 = out_command;
	FIO1CLR = LCD_DC;	//	|LCD_CS
	FIO1SET = LCD_RD;
	FIO1CLR = LCD_RD;
}

void write_d(unsigned char out_data) {
	//FIO1DIR |= LCD_DATA;
	FIO1PIN2 = out_data;
	FIO1SET = LCD_DC | LCD_RD;
	FIO1CLR = LCD_RD;
}

void write_color(unsigned short out_color) {
	FIO1PIN2 = (char)(out_color>>8);
	FIO1SET = LCD_DC | LCD_RD;
	FIO1CLR = LCD_RD;
	FIO1PIN2 = (char)(out_color&0xFF);
	FIO1SET = LCD_RD;
	FIO1CLR = LCD_RD;	
}

void write_262k(unsigned short out_color) {
	FIO1PIN2 = RED_RGB(out_color);
	FIO1SET = LCD_DC | LCD_RD;
	FIO1CLR = LCD_RD;
	FIO1PIN2 = GRN_RGB(out_color)>>1;
	FIO1SET = LCD_DC | LCD_RD;
	FIO1CLR = LCD_RD;
	FIO1PIN2 = BLU_RGB(out_color);
	FIO1SET = LCD_RD;
	FIO1CLR = LCD_RD;
}

void Reset_SSD1339(void) {
  FIO1CLR = LCD_RSTB;
  delay_ms(100);
  FIO1SET = LCD_RSTB;
}
