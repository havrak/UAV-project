/*
Author£ºKevin
Web£ºhttp://RobotControl.taobao.com
Compilation environment£ºMDK-Lite  Version: 5.17
Initial release time: 2016-1-31
Test£º This program has been tested on the STM32Core platform successfully.
Features£º
Use STM32Core platform serial port 2 to read the data of JY901, and then directly connect to the host computer through serial port 1. The baud rate of 9600 is selected on the host computer.
What you see with serial debugging software is a hexadecimal number.
Wiring Connection
USB-TTL serial converter      STM32Core              JY901
VCC          -----           VCC        ----        VCC
TX           -----           RX1  £¨PIN10£©   
RX           -----           TX1   (PIN9)
GND          -----           GND    ----        GND
                             RX2     £¨PIN 3£©  ----        TX
							 TX2     £¨PIN 2£© ----        RX
------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include "Main.h"
#include "REG.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "UART1.h"
#include "UART2.h"
#include "delay.h"
#include "IOI2C.h"
#include "hw_config.h"
#include "DIO.h"
void CopeSerial1Data(unsigned char ucData)
{	
	UART2_Put_Char(ucData);
}
void CopeSerial2Data(unsigned char ucData)
{
	LED_REVERSE();
	UART1_Put_Char(ucData);
	USB_TxWrite(&ucData,1);
}
int main(void)
{  
	unsigned char str[100];
	unsigned char len,i;
		
	USB_Config();		
	SysTick_init(72,10);
	Initial_UART1(9600);//connect with PC serial port
	Initial_UART2(9600);//connect with WT901 serial port	
	
	LED_ON();
	while (1)
	{
		delay_ms(1);
		len = USB_RxRead(str, sizeof(str));
		for (i=0;i<len;i++)
		{
				UART2_Put_Char(str[i]);
		}
	}
}



