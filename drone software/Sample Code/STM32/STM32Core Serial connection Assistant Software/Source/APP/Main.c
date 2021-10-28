/*
Author：Kevin
Web：http://RobotControl.taobao.com
Compilation environment：MDK-Lite  Version: 5.17
Initial release time: 2016-1-31
Test： This program has been tested on the STM32Core platform successfully.
Features：
Use STM32Core platform serial port 2 to read the data of JY901, and then directly connect to the host computer through serial port 1. The baud rate of 9600 is selected on the host computer.
What you see with serial debugging software is a hexadecimal number.
Wiring Connection
USB-TTL serial converter      STM32Core              JY901
VCC          -----           VCC        ----        VCC
TX           -----           RX1  （PIN10）   
RX           -----           TX1   (PIN9)
GND          -----           GND    ----        GND
                             RX2     （PIN 3）  ----        TX
							 TX2     （PIN 2） ----        RX
------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include "Main.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "UART1.h"
#include "UART2.h"
#include "delay.h"
#include "IOI2C.h"
#include "hw_config.h"
#include "JY901.h"
#include "DIO.h"

struct STime		stcTime;
struct SAcc 		stcAcc;
struct SGyro 		stcGyro;
struct SAngle 	stcAngle;
struct SMag 		stcMag;
struct SDStatus stcDStatus;
struct SPress 	stcPress;
struct SLonLat 	stcLonLat;
struct SGPSV 		stcGPSV;
struct SQ       stcQ;

//CopeSerialData calls the function for the serial port 2 interrupt. This function is called once each time the serial port receives data.
void CopeSerial2Data(unsigned char ucData)
{
	static unsigned char ucRxBuffer[250];
	static unsigned char ucRxCnt = 0;	
	
	LED_REVERSE();					//Once receiving data, LED flashes once
	USB_TxWrite(&ucData,1);			//Forward the received serial data to the USB-HID port, and you can directly see the data output by the module by connecting to the upper computer.
	ucRxBuffer[ucRxCnt++]=ucData;	//Store the received data in the buffer
	if (ucRxBuffer[0]!=0x55) //The data header is not correct, then restart to find the 0x55 data header
	{
		ucRxCnt=0;
		return;
	}
	if (ucRxCnt<11) {return;}//If the data is less than 11, return
	else
	{
		switch(ucRxBuffer[1])//Determine what kind of data the data is, and then copy it to the corresponding structure. Some data packets need to open the corresponding output through the upper computer before receiving the data of this data packet.
		{
			case 0x50:	memcpy(&stcTime,&ucRxBuffer[2],8);break;//memcpy is a memory copy function that comes with the compiler. You need to reference "string.h" to copy the characters of the receive buffer into the data structure to achieve data parsing.
			case 0x51:	memcpy(&stcAcc,&ucRxBuffer[2],8);break;
			case 0x52:	memcpy(&stcGyro,&ucRxBuffer[2],8);break;
			case 0x53:	memcpy(&stcAngle,&ucRxBuffer[2],8);break;
			case 0x54:	memcpy(&stcMag,&ucRxBuffer[2],8);break;
			case 0x55:	memcpy(&stcDStatus,&ucRxBuffer[2],8);break;
			case 0x56:	memcpy(&stcPress,&ucRxBuffer[2],8);break;
			case 0x57:	memcpy(&stcLonLat,&ucRxBuffer[2],8);break;
			case 0x58:	memcpy(&stcGPSV,&ucRxBuffer[2],8);break;
			case 0x59:	memcpy(&stcQ,&ucRxBuffer[2],8);break;
		}
		ucRxCnt=0;//Clear the cache area
	}
}

void CopeSerial1Data(unsigned char ucData)
{	
	UART2_Put_Char(ucData);//转发串口1收到的数据给串口2（JY模块）
}


int main(void)
{  
	char str[100];
	unsigned char len,i;
		
	USB_Config();		//Configure USB-HID
	SysTick_init(72,10);//Set the clock frequency
	Initial_UART1(9600);//connect with PC serial port
	Initial_UART2(9600);//Connect to the serial port of JY-901 module	
	
	LED_ON();
	delay_ms(1000);//Wait for JY-91 initialization to complete
	while(1)
	{			
			delay_ms(500);
		//Output time 
		sprintf(str,"Time:20%d-%d-%d %d:%d:%.3f\r\n",stcTime.ucYear,stcTime.ucMonth,stcTime.ucDay,stcTime.ucHour,stcTime.ucMinute,(float)stcTime.ucSecond+(float)stcTime.usMiliSecond/1000);
			UART1_Put_String(str);		
			delay_ms(10);
		//Output acceleration
		sprintf(str,"Acc:%.3f %.3f %.3f\r\n",(float)stcAcc.a[0]/32768*16,(float)stcAcc.a[1]/32768*16,(float)stcAcc.a[2]/32768*16);
			UART1_Put_String(str);
			delay_ms(10);
		//Output angular velocity
		sprintf(str,"Gyro:%.3f %.3f %.3f\r\n",(float)stcGyro.w[0]/32768*2000,(float)stcGyro.w[1]/32768*2000,(float)stcGyro.w[2]/32768*2000);
			UART1_Put_String(str);
			delay_ms(10);
		//Output angle
		sprintf(str,"Angle:%.3f %.3f %.3f\r\n",(float)stcAngle.Angle[0]/32768*180,(float)stcAngle.Angle[1]/32768*180,(float)stcAngle.Angle[2]/32768*180);
			UART1_Put_String(str);
			delay_ms(10);
		//Output magnetic field
		sprintf(str,"Mag:%d %d %d\r\n",stcMag.h[0],stcMag.h[1],stcMag.h[2]);
			UART1_Put_String(str);		
			delay_ms(10);
		//Output air pressure, altitude
		sprintf(str,"Pressure:%ld Height%.2f\r\n",stcPress.lPressure,(float)stcPress.lAltitude/100);
			UART1_Put_String(str); 
			delay_ms(10);
		//Output port status
		sprintf(str,"DStatus:%d %d %d %d\r\n",stcDStatus.sDStatus[0],stcDStatus.sDStatus[1],stcDStatus.sDStatus[2],stcDStatus.sDStatus[3]);
			UART1_Put_String(str);
			delay_ms(10);
		//latitude and longitude Output 
		sprintf(str,"Longitude:%ldDeg%.5fm Lattitude:%ldDeg%.5fm\r\n",stcLonLat.lLon/10000000,(double)(stcLonLat.lLon % 10000000)/1e5,stcLonLat.lLat/10000000,(double)(stcLonLat.lLat % 10000000)/1e5);
			UART1_Put_String(str);
			delay_ms(10);
		//Ground speed output
		sprintf(str,"GPSHeight:%.1fm GPSYaw:%.1fDeg GPSV:%.3fkm/h\r\n",(float)stcGPSV.sGPSHeight/10,(float)stcGPSV.sGPSYaw/10,(float)stcGPSV.lGPSVelocity/1000);
			UART1_Put_String(str);
			delay_ms(10);
		//Quaternion output
		sprintf(str,"Four elements:%.5f %.5f %.5f %.5f\r\n\r\n",(float)stcQ.q[0]/32768,(float)stcQ.q[1]/32768,(float)stcQ.q[2]/32768,(float)stcQ.q[3]/32768);
			UART1_Put_String(str);
		    delay_ms(10);//Wait for the transfer to complete
	}//主循环
}



