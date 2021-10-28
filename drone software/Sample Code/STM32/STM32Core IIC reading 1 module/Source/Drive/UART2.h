#ifndef __UARTS_H
#define __UARTS_H


void Initial_UART1(unsigned long baudrate);
void UART1_Put_Char(unsigned char DataToSend);
unsigned char UART1_Get_Char(void);
void UART1_Put_String(unsigned char *Str);

void Initial_UART2(unsigned long baudrate);
void UART2_Put_Char(unsigned char DataToSend);
unsigned char UART2_Get_Char(void);
void UART2_Put_String(unsigned char *Str);
#endif

//------------------End of File----------------------------

