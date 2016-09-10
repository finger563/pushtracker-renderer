#ifndef BOARD_H
#define BOARD_H

//#include <stdlib.h>
#include "LPC214x.h"

//#define INVERT
//#define DEBUG	1		// used for printing to the serial terminal out of the programming pins
#ifdef INVERT
#define TRANS_CHAR 0x2E
#else
#define TRANS_CHAR 0x8C
#endif

//================================
//		Boolean Defines
//================================
#define ON	0
#define OFF	1
#define TRUE 1
#define FALSE 0

//================================
//			I/O Defines
//================================
//#define USE_SLOW_GPIO
#define USE_FAST_GPIO


#define CHIP_SELECT	(1<<20)
#define SPI_SLAVE_CS (1<<20)  //pin P0.20

#ifdef USE_SLOW_GPIO
	#define SelectAccel()	IOCLR0 = CHIP_SELECT
	#define UnselectAccel()	IOSET0 = CHIP_SELECT
#else
	#define SelectAccel()	FIO0CLR = CHIP_SELECT
	#define UnselectAccel()	FIO0SET = CHIP_SELECT
#endif



#define OLED_PWR	(1<<30)			// P0.30

#ifdef USE_SLOW_GPIO
	#define OLED_ON		IOSET0 = OLED_PWR
	#define OLED_OFF	IOCLR0 = OLED_PWR
#else
	#define OLED_ON		FIO0SET = OLED_PWR
	#define OLED_OFF	FIO0CLR = OLED_PWR
#endif

#define BLUE			21		// blue LED is on P0.21 (active low)
#define USB_LED			31		// usb (green) LED is on P0.31 (active low)

#define STAT0                   21		// blue LED is on P0.21 (active low)
#define STAT1			31		// usb (green) LED is on P0.31 (active low)

#define SWITCH_UP		10
#define SWITCH_ENTER            11
#define SWITCH_DOWN		12

#define ODOMETER_PIN	25		// D+ is on P0.26, and is used for odometer, when it goes low, the switch is actuated by a magnet

//================================
//		PushTracker Defines
//================================
#define LOG_INTERVAL		30
#define ODO_INTERVAL		5
#define WRITE_INTERVAL		2000
#define SYNC_INTERVAL		WRITE_INTERVAL

#define RAF_BUFFER_SIZE		5
#define VAL_BUFFER_SIZE		8
#define RAW_BUFFER_SIZE		25

typedef struct 
{
	float x,y,z;
} handle;

typedef struct cube_typ
{
	handle center;		// center
	int v[12][3];			//vertexes 
	int poly[10][10];		// polygons
	int num_vert;
	int num_polys;
} cube, *cube_ptr;

typedef struct accel_struct
{
	short x,y,z;
	float gx,gy,gz;
} accel_obj, *accel_ptr;


static void delay_ms(unsigned int count)
{
  long i        = 0;
  long countMs  = count * 8000;

  for(i = 0; i < countMs; i++);
}
//void reset_system(void);
//void Cube(void);

//void cube_init(struct cube* cb,float a, float x,float y,float z);

#endif
