#include<stdint.h>
unsigned int getJoyStickValue();
void adc0init();
int main();
void changeState();

typedef struct RTC_Time
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day_of_month;
	uint8_t day_of_week;
	uint16_t day_of_year;
	uint8_t month;
	uint16_t year;
}RTC_Time;

void RTC_Set_Time( RTC_Time Set_Time);
void RTC_Set_Alarm_Time( RTC_Time Alarm_Time);
RTC_Time RTC_Get_Time(void);






void directionManip(int);
void prepareStateChange(void);
void displayState(void);

//TODO
void LCDPrint(const char* text);
void LCDAction(unsigned int actionType,unsigned int delayAmt,unsigned char cmdByte);
void LCDActionFinish(unsigned int,unsigned int);
void LCDInit(void);
void LCDReset(void);
void delay(unsigned int);