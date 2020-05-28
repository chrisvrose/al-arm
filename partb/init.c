
#include<lpc214x.h>
#include<stdio.h>
#include "init.h"
#define PRESCALEMULT 15000

//Toggle bits designed to maintain sanity
volatile unsigned int rtcToggler = 0;
volatile unsigned int timerToggler = 0;
volatile unsigned int alarmBit = 0;
volatile unsigned int ADCResult = 0;
unsigned int pastADCResult = 0;

volatile RTC_Time savedTime;
volatile RTC_Time savedAlarm;

volatile RTC_Time customTime;
volatile RTC_Time customAlarm;
volatile int saveRequested = 0;

volatile char currentTime[17];
volatile char currentDate[17];


// 0 def 1 change alarm 2 
int globalState = 0;

// 0 h 1 m 2 s 3day 4mon 5yyyy 6alarm/timeapply toggle
int globalCursorState = 0;
//int maxes[] = {24,60,60,31,12,4000,2};
char * dateTimes[] = {"","JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};

__irq void rtcInterruptHandler(void){
	rtcToggler = ~rtcToggler;
	
	if(ILR&1){
		savedTime = RTC_Get_Time();
		//The usual seconds.
		ILR = 1;
	}
	if(ILR&2){
		alarmBit = 1;
		//This is an alarm
		//SOUND THE ALARM
		
		IOSET0 = 1<<25;
		ILR = 2;
	}
	VICVectAddr = 0;
}

__irq void timerInterruptHandler(void){
	timerToggler = ~timerToggler;
	if(T0IR&1){
		//Mid second
		IOSET0 = 1<<31;
	}
	else if (T0IR&4){
		//Start of second
		IOCLR0 = 1u<<31;
	}
	
	
	if(T0IR&0xF){
		//Do this every interrupt
		int result = getJoyStickValue();
		//Do work here
		//switch()
		if(result){
			if(result&0b1111){
				//dirchange - directions pressed
				directionManip(result);
			}else{
				//statechange - directions
				prepareStateChange();
				
				changeState();
			}
		}
		
		//After dealing with everything
		displayState();
	}
	
	//clear all interrupts
	T0IR = 0xf;
	VICVectAddr = 0x0;
}



void directionManip(int input){
	switch(globalState){
		case 0:
			//Silence buzzer
			IOCLR0 = 1<<25;
		break;
		case 1:
			//DEAL WITH DIRECTION IN ALARM change- 1
			//DEAL WITH DIRECTION IN TIME change - 2
			if(input&0b1010){
				//Flip around selection
				if(input&0b1000){
					//right
					globalCursorState =  ((globalCursorState+1)%7)==3?6:((globalCursorState+1)%7);//  skip the months
				}else{
					//left
					globalCursorState = ((globalCursorState+6)%7)==5?2:((globalCursorState+6)%7);// Skip the months
				}
			}else{
				if(input&1){
					//UP
					switch(globalCursorState){
						case 0:
							customAlarm.hours = (customAlarm.hours+1)%24;
							break;
						case 1:
							customAlarm.minutes = (customAlarm.minutes+1)%60;
							break;
						case 2:
							customAlarm.seconds = (customAlarm.seconds+1)%60;
							break;
						case 3:
							customAlarm.day_of_month = (customAlarm.day_of_month)%31 +1;
							break;
						case 4:
							customAlarm.month = (customAlarm.month)%12 +1;
							break;
						case 5:
							customAlarm.year = (customAlarm.year+1)%4000;
							break;
						case 6:
							saveRequested = !saveRequested;
							break;
					}
					
				}else{
					//DOWN
					switch(globalCursorState){
						case 0:
							customAlarm.hours = (customAlarm.hours+23)%24;
							break;
						case 1:
							customAlarm.minutes = (customAlarm.minutes+59)%60;
							break;
						case 2:
							customAlarm.seconds = (customAlarm.seconds+59)%60;
							break;
						case 3:
							customAlarm.day_of_month = (customAlarm.day_of_month+29)%31 +1;
							break;
						case 4:
							customAlarm.month = (customAlarm.month+10)%12 +1;
							break;
						case 5:
							customAlarm.year = (customAlarm.year+3999)%4000;
							break;
						case 6:
							saveRequested = !saveRequested;
							break;
					}
				}
			}
		break;
		case 2:
			//DEAL WITH DIRECTION IN ALARM change- 1
			//DEAL WITH DIRECTION IN TIME change - 2
			if(input&0b1010){
				//Flip around selection
				if(input&0b1000){
					//right
					globalCursorState = (globalCursorState+1)%7;
				}else{
					//left
					globalCursorState = (globalCursorState+6)%7;
				}
			}else{
				if(input&1){
					//UP
					switch(globalCursorState){
						case 0:
							customTime.hours = (customTime.hours+1)%24;
							break;
						case 1:
							customTime.minutes = (customTime.minutes+1)%60;
							break;
						case 2:
							customTime.seconds = (customTime.seconds+1)%60;
							break;
						case 3:
							customTime.day_of_month = (customTime.day_of_month)%31 +1;
							break;
						case 4:
							customTime.month = (customTime.month)%12 +1;
							break;
						case 5:
							customTime.year = (customTime.year+1)%4000;
							break;
						case 6:
							saveRequested = !saveRequested;
							break;
					}
					
				}else{
					//DOWN
					switch(globalCursorState){
						case 0:
							if(globalState==1) customTime.hours = (customTime.hours+23)%24;
							break;
						case 1:
							if(globalState==1) customTime.minutes = (customTime.minutes+59)%60;
							break;
						case 2:
							if(globalState==1) customTime.seconds = (customTime.seconds+59)%60;
							break;
						case 3:
							if(globalState==1) customTime.day_of_month = (customTime.day_of_month+29)%31 +1;
							break;
						case 4:
							if(globalState==1) customTime.month = (customTime.month+10)%12 +1;
							break;
						case 5:
							if(globalState==1) customTime.year = (customTime.year+3999)%4000;
							break;
						case 6:
							saveRequested = !saveRequested;
							break;
					}
				}
			}
			break;
			
	}
}

//Prepare to switch the state;
void prepareStateChange(){
	switch(globalState){
		case 0://0->1
			customAlarm = savedAlarm;
			globalCursorState = 0;
			break;
		case 1://1->2; save 1 if required, load
			if(saveRequested)
				RTC_Set_Alarm_Time(customAlarm);
			customTime = savedAlarm;
			globalCursorState = 0;saveRequested = 0;
			break;
		case 2://2->0
			if(saveRequested)
				RTC_Set_Time(customTime);
			break;
			
	}
}

void displayState(void){
		switch(globalState){
			case 0:
				sprintf(currentTime,"%02d:%02d:%02d        ",savedTime.hours,savedTime.minutes,savedTime.seconds);
				sprintf(currentDate,"%02d %s %04d    %c",savedTime.day_of_month,dateTimes[savedTime.month],savedTime.year,saveRequested?' ':' ');
				break;
			case 1:
				sprintf(currentTime,"%02d:%02d:%02d       %d",customAlarm.hours,customAlarm.minutes,customAlarm.seconds,globalCursorState);
				sprintf(currentDate,"               %c",saveRequested?'Y':'N');
				break;
			case 2:
				sprintf(currentTime,"%02d:%02d:%02d       %d",customTime.hours,customTime.minutes,customTime.seconds,globalCursorState);
				sprintf(currentDate,"%02d %s %04d    %c",customTime.day_of_month,dateTimes[customTime.month],customTime.year,saveRequested?'Y':'N');
				break;
		}
		
		
		
		//LCD print
		
}


void changeState(void){
	globalState = (globalState+1)%3;
}
void initSpeed(void){
	PLL0CON = 1;
	PLL0CFG = 0b00100100;
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;
	while(!(PLL0STAT &0x400));
	PLL0CON=0x3;
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;
	VPBDIV = 0x0;
}

void timerInit(void){
	T0TCR = 0;
	T0PR = PRESCALEMULT/4 - 1;
	T0TC = 0;
	T0MR0 = 1000;T0MR1 = 2000;T0MR2 = 3000;T0MR3 = 4000;
	
	
	T0MCR = 1|1<<3|1<<6|0x3<<9;		//Reset at 4th end
	VICIntEnable |= 0x10;
	VICVectAddr0 = (unsigned long)timerInterruptHandler;		//interrupts run 1/4th a second.
	VICVectCntl0 = 0x24;
	T0TCR = 1;
}

void adc1Init(void){
	PINSEL0 |= 0xf300000;
}
void rtcInit(void){
	PREINT = 0x01C8; /* For 15MHz Fpclk, PREINT value */
	PREFRAC = 0x61C0; /*	For 15MHz Fpclk, PREFRAC value */
	
	CCR = 1|1<<4;//Enable, and set to use PCLK
	//DEBUG
	//struct RTC_Time rt = RTC_Get_Time();
	//rt.seconds+=10;
	//RTC_Set_Time(rt);
	//RTC_Set_Alarm_Time(rt);
	//setup an interrupt
	VICIntEnable |= 0x00002000;
	VICVectAddr1 = (unsigned long) rtcInterruptHandler;
	VICVectCntl1 = 0x20|13;
	
	AMR = 0x1F<<3;		//ignore everything but hhmmss
	
	//CCR = CCR - CCR&2;//remove reset
	
	CIIR = 1;								//every second
	ILR = 3;								//interrupts for alarm, clear all
	
}
//enter right down left up
unsigned int getJoyStickValue(void){
	AD1CR = 0x200300|1<<4;
	AD1CR |= 1<<24;
	while((AD1STAT&1<<4)==0);
	ADCResult = AD1DR4>>6&0x3ff;
	//sanity - store
	pastADCResult = ADCResult;
	
	if(ADCResult==0) return 0; 													//nothing
	else if(ADCResult > 180 && ADCResult < 185) return 1;		//up
	else if(ADCResult > 315 && ADCResult < 325) return 2;		//left
	else if(ADCResult > 460 && ADCResult < 470) return 4;		//down
	else if(ADCResult > 614 && ADCResult < 624) return 8;		//right
	else if(ADCResult > 770 && ADCResult < 800) return 16;	//enter
	else return 0;																		//random - nothing
	
	
	LCDAction(0,45,1<<7);
	LCDPrint(currentTime);
	LCDAction(0,45,1<<7|0x40);
	LCDPrint(currentDate);
}


void RTC_Set_Time( RTC_Time Set_Time){
	SEC = Set_Time.seconds;
	MIN = Set_Time.minutes;
	HOUR = Set_Time.hours;
	DOM = Set_Time.day_of_month;
	DOW = Set_Time.day_of_week;
	DOY = Set_Time.day_of_year;
	MONTH = Set_Time.month;
	YEAR = Set_Time.year;
}

void RTC_Set_Alarm_Time( RTC_Time Alarm_Time){
	ALSEC = Alarm_Time.seconds;
	ALMIN = Alarm_Time.minutes;
	ALHOUR = Alarm_Time.hours;
	ALDOM = Alarm_Time.day_of_month;
	ALDOW = Alarm_Time.day_of_week;
	ALDOY = Alarm_Time.day_of_year;
	ALMON = Alarm_Time.month;
	ALYEAR = Alarm_Time.year;
}

RTC_Time RTC_Get_Time(void){
	RTC_Time time;
	time.seconds = SEC;
	time.minutes = MIN;
	time.hours = HOUR;
	time.day_of_month = DOM;
	time.day_of_week = DOW;
	time.day_of_year = DOY;
	time.month = MONTH;
	time.year = YEAR;
	return time;
}




void LCDPrint(const char* text){
	while(*text){
		LCDAction(1,45,*text);
		text++;
	}
}

void LCDSendUpper(unsigned int val){
	IOCLR0 = 0xf0000; IOSET0 = ((val>>4)&0xf)<<16;
}

//0 for cmd, 1 for data
void LCDActionFinish(unsigned int num,unsigned int delayToGive){
	if(num)IOSET0 = 1<<20;
	else IOCLR0 = 1<<20;
	
	IOSET1 = 1<<25;
	delay(delayToGive);
	IOCLR1 = 1<<25;
}

void LCDInit(void){
	LCDReset();
	LCDAction(0,45,0x28u);
	LCDAction(0,45,0xEu);
	LCDAction(0,1600,0x1u);
	LCDAction(0,45,0x80u);
	
}
void delay(unsigned int val){
	T1TCR = 0;
	T1TC = 0;
	T1PR = PRESCALEMULT - 1;
	T1MCR = 0x7;
	T1MR0 = val;
	T1TCR = 1;
	while(!(T1IR&1));
	T1IR = 1;
	T1TCR = 0;
}
void LCDReset(void){
	LCDSendUpper(0x30);
	LCDActionFinish(0,50);
	LCDSendUpper(0x30);
	LCDActionFinish(0,50);
	LCDSendUpper(0x30);
	LCDActionFinish(0,50);
	LCDSendUpper(0x20);
	LCDActionFinish(0,50);
}
void LCDAction(unsigned int actionType,unsigned int delayAmt,unsigned char cmdByte){
	LCDSendUpper(cmdByte);
	LCDActionFinish(actionType,delayAmt/2);
	LCDSendUpper(cmdByte<<4);
	LCDActionFinish(actionType,delayAmt);
}
int main(){
 	initSpeed();
	IODIR0 = 0xC0000003|1<<25|0xFF0000;
	IODIR1 |= 1<<25;
	rtcInit();
	timerInit();
	adc1Init();
	LCDInit();
	while(1){
		//PCON = 1;//Idle whenever possible
	}
}
