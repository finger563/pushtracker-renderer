#include "ADXL345.h"
#include "SSP.h"
#include "board.h"

void initAccel(void)
{	
	unsigned char byte=0;
	
	SelectAccel();
	SSP_send(0x80);			// read Device ID from addr 0x00
	byte = SSP_recv();
	UnselectAccel();
	
	//rprintf("Device ID: %d\n",byte);
	
	while(byte != 0x28)
	{
		SelectAccel();
		SSP_send(0x2D);		
		SSP_send(0x28);
		UnselectAccel();
		
		delay_ms(5);
		
		SelectAccel();
		SSP_send(0xAD);		//read from power control register
		byte = SSP_recv();
		UnselectAccel();
		
		//rprintf("received %d\n",byte);
		
		delay_ms(500);
	}
	
	SelectAccel();
	SSP_send(0x31);
	SSP_send(0x08);
	UnselectAccel();
	
	//rprintf("sent 0x31 and 0x08.\n");
	
	delay_ms(5);
	
	SelectAccel();
	SSP_send(0xB1);
	byte = SSP_recv();
	UnselectAccel();
	
	//rprintf("received %d.\n",byte);
	
	SelectAccel();
	SSP_send(0x38);
	SSP_send(0x00);
	UnselectAccel();
	
	//rprintf("sent 0x38 and 0x00.\n");
	
	delay_ms(5);
	
	SelectAccel();
	SSP_send(0xB8);
	byte = SSP_recv();
	UnselectAccel();
}

short accelX(void)
{
	short val=0;

	SelectAccel();
	SSP_send(READ_2 | OutX);
	val = SSP_recv();
	val |= (SSP_recv()<<8);
	UnselectAccel();
	
	return val;
}	

short accelY(void)
{	
	short val=0;
	
	SelectAccel();
	SSP_send(READ_2 | OutY);
	val = SSP_recv();
	val |= (SSP_recv()<<8);
	UnselectAccel();
	
	return val;
}

short accelZ(void)
{	
	short val=0;

	SelectAccel();
	SSP_send(READ_2 | OutZ);
	val = SSP_recv();
	val |= (SSP_recv()<<8);
	UnselectAccel();
	
	return val;
}

void accel_all(accel_ptr aptr)
{
	short _dummy;
	SelectAccel();
	SSP_send(READ_2| OutX);
	_dummy = SSP_recv();
	_dummy |= (SSP_recv()<<8);
	aptr->x = _dummy;
	_dummy = SSP_recv();
	_dummy |= (SSP_recv()<<8);
	aptr->y = _dummy;
	_dummy = SSP_recv();
	_dummy |= (SSP_recv()<<8);
	aptr->z = _dummy;
	
	aptr->gx = (float)aptr->x*((float)9.81/(float)GRAVITY);
	aptr->gy = (float)aptr->y*((float)9.81/(float)GRAVITY);
	aptr->gz = (float)aptr->z*((float)9.81/(float)GRAVITY);
	
	UnselectAccel();
}

void powerdownAccel(void)
{
	SelectAccel();
	SSP_send(WRITE | POWER_CTL);
	SSP_send(PD);
	UnselectAccel();
}
