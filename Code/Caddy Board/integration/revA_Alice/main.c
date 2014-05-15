/*
 FreeRTOS V6.1.0
 
 This is a basic RTOS program. It turns on the LEDs.
 Nothing too exciting.
 
 */

#include <stdint.h>
#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "FreeRTOS.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "task.h"
#include "usart.h"
#include "Components/Sonar/Sonar.h"
#include "queue.h"
#include "spi.h"
#include "ADC.h"
#include "Components/FNR/FNR.h"
#include "Components/Speed/Speed.h"

void vTaskFunction_1(void *pvParameters);
void vTaskFunction_2(void *pvParameters);
void vTaskFunction_3(void *pvParameters);
void vTaskPot(void *pvParameters);
void vIO_init(void);
void vApplicationTickHook();
void printNum(unsigned char i);
void printHex(int i);

int count = 0;

int sonarTime;

void vApplicationTickHook()
{
    count++;
}

void vApplicationStackOverflowHook(){
	return;
}
/*-----------------------------------------------------------*/

int main( void )
{
    
    //- wimpy variable defs
	uint8_t val[2] = {0x3F, 0xFE};
    uint8_t *val0, *val1;
    
	//- avoid pesky warnings...
	val0 = val + 0;
	val1 = val + 1;
    
	DDRA = 0xF7;
	PORTA = 0;
	DDRD = 0xFF;
	DDRE = 0xFF;
	DDRB = 0xFF; 
	DDRK = 0;

	PORTD = 0;
	PORTE = 0;
	PORTK = 2;

    //- init IO with goodness
	vIO_init();

	//- Create a 
	xTaskCreate( (pdTASK_CODE) vTaskFunction_1, (signed char *) "T0", configMINIMAL_STACK_SIZE+1000,
                (void *) val1, 1, NULL );
   xTaskCreate( (pdTASK_CODE) vTaskFunction_2, (signed char *) "T0", configMINIMAL_STACK_SIZE+1000,
                (void *) val1, 1, NULL );
   /*xTaskCreate( (pdTASK_CODE) vTaskFunction_3, (signed char *) "T0", configMINIMAL_STACK_SIZE+1000,
                (void *) val1, 1, NULL );
	xTaskCreate( (pdTASK_CODE) vTaskPot, (signed char *) "T0", configMINIMAL_STACK_SIZE+1000,
                (void *) val1, 1, NULL );*/

 
   /*xTaskCreate( (pdTASK_CODE) vTaskSonar, (signed char *) "T1", configMINIMAL_STACK_SIZE+1000,
                (void *) val1, 1, NULL );*/

   xTaskCreate( (pdTASK_CODE) vTaskUSARTWrite, (signed char *) "T2", configMINIMAL_STACK_SIZE+1000,
   				(void *) val1, 1, NULL);

   xTaskCreate( (pdTASK_CODE) vTaskUSARTRead, (signed char *) "T3", configMINIMAL_STACK_SIZE+1000,
   				(void *) val1, 1, NULL);

	
	xTaskCreate( (pdTASK_CODE) vTaskADC, (signed char *) "T3", configMINIMAL_STACK_SIZE+1000,
   				(void *) val1, 1, NULL);

//   xTaskCreate( (pdTASK_CODE) vTaskUSARTLog, (signed char *) "T4", configMINIMAL_STACK_SIZE+1000,
//   				(void *) val1, 1, NULL);
    
    //- kick off the scheduler
	vTaskStartScheduler();
    
	return 0;
}
/*-----------------------------------------------------------*/

void printHex(int i){
	char hex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	USART_Write(hex[(i&0xF000) >> 12]);
	USART_Write(hex[(i&0xF00) >> 8]);
	USART_Write(hex[(i&0xF0) >> 4]);
	USART_Write(hex[i&0xF]);

}

void printNum(unsigned char i){
    
    char str[10];
    memset(str,0,10);
    itoa(i,str,10);
    char * irr = str;
    while(*irr){
        USART_AddToQueue((unsigned char )*irr);
        irr++;
    }
    
}

ISR(TIMER5_OVF_vect){

	PORTB ^= 0xFF;
	TCNT3L = 0;
	TCNT3H = 0;

}

unsigned int getTimerCount2(){

	//volatile char timerLow = TCNT3L;

	return TCNT0;

}


int potValue(int sonarMax,int sonarMin,int potMax,int potMin,int x){
	int value;	
	if(x < sonarMin){
		return 0;
	} else if (x > sonarMax){
		return 0x7F;
	} else {
		value = x / 4;
		return value > potMax?potMax:value;
	}
} 

void vTaskFunction_1(void *pvParameters)
{	
	int sonarResult;
	int movingForward = 1;

	int sonarMax = 0x31C;
	int sonarMin = 0xE0;
	int potMin = 0x18;
	int potMax = 0x7F;
	int backwardSwitch = 0xE1;
	int forwardSwitch = 0x160;

	

   USART_Init(9600, 16000000);

	initializeSPI();
	initSpeedController();
	
	PORTA |= 4;
	
	//PORTC = 0x2;
	setFNR(1);
	
	for(;;){

		sonarResult = getSonarResult();
		if(movingForward == 1){
			if(sonarResult < backwardSwitch && sonarResult < 0x300){
				movingForward = -1;
				setFNR(-1);
			}
			setPot(potValue(sonarMax,sonarMin,potMax,potMin,sonarResult));
		} else {
			if(sonarResult > forwardSwitch && sonarResult < 0x300){
				movingForward = 1;
				setFNR(1);
			}
			setPot(potValue(sonarMax,sonarMin,potMax,potMin,sonarResult) + 0x10);
		}
		printHex(potValue(sonarMax,sonarMin,potMax,potMin,sonarResult));
		USART_Write('\n');
		USART_Write('\r');
		vTaskDelay(1);
	}

	for(;;){
		PORTL = 0x10;
		vTaskDelay(200);
		PORTL = 0x20;
		vTaskDelay(200);
		PORTL = 0x40;
		vTaskDelay(200);
		PORTL = 0x80;
		vTaskDelay(200);
	}
}

void vTaskFunction_2(void *pvParameters){
	DDRL = 0xC0;
	for(;;){
		PORTL ^= 0xC0;
		vTaskDelay(300);
	}	
}

void vTaskFunction_3(void *pvParameters){
	
	/*for(;;){
		if(PINA & 0x08){
			PORTA |= 0x84;
		} else {
			PORTA &= ~0x84;
		}
		vTaskDelay(5);
	}*/

}

void vTaskPot(void *pvParameters){
	initializeSPI();
	int a = 0;
	initSpeedController();
	/*for(;;){
		setPot(a);
		vTaskDelay(80);
		a++;
	}*/
	for(;;){
		setPot(1);
		vTaskDelay(500);
		setPot(0x40);
		vTaskDelay(500);
		setPot(0x7F);
		vTaskDelay(500);
		setPot(0x40);
		vTaskDelay(500);
	}
}


void vIO_init(void)
{
    //- set PortB as output
	DDRB = 0xFF; 
    PORTB = 0; 
}


