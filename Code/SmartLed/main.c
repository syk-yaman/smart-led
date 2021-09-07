#include <avr/io.h>
#include "rtc.h"
#include <util/delay.h>
#include <avr/wdt.h>

//Macros
#define bit_is_low(sfr,bit)(!(_SFR_BYTE(sfr) & _BV(bit)))
#define bit_is_high(sfr,bit)(_SFR_BYTE(sfr) & _BV(bit))

//Head of methods
void checkElectricityStatus();
int isActiveTime(rtc_t rtc);
void changeOutputLedStatus(int status);
void changeElectricityInidicator(int status);
void changeTimeInidicator(int status); 
void changeAutoInidicator(int status);
void setRtcTime(rtc_t rtc);
void initPorts();
void checkManualButton();
void checkOnButton();
void checkOffButton();
void blinkAccordingToHourNumber(rtc_t rtc);
void checkShowTimeButton(rtc_t rtc);

//Button status
int manualButtonStatus = 1;
int turnOnButtonStatus = 1;
int turnOffButtonStatus = 1;
int showTimeButtonStatus = 1;
int electricityStatus = 0;

//Variables
int isManualControl = 0;

int main()
{
	OSCCAL=0xFF;
	rtc_t rtc;
	RTC_Init();
	initPorts();
	wdt_enable(WDTO_2S);
	//setRtcTime(rtc);
	
	while(1)
	{
		wdt_reset();
		RTC_GetDateTime(&rtc);
		checkShowTimeButton(rtc);

		checkManualButton();
		checkElectricityStatus();

		if(isManualControl == 1){
			changeAutoInidicator(1);
			} else {
			changeAutoInidicator(0);
		}

		if(isManualControl == 0)
		{
			if(isActiveTime(rtc) == 1)
			{
				changeTimeInidicator(1);
				if(electricityStatus==1)
				{
					changeOutputLedStatus(0);
					changeElectricityInidicator(1);
				}
				else
				{
					changeOutputLedStatus(1);
					changeElectricityInidicator(0);
				}
			}
			else
			{
				changeTimeInidicator(0);
				changeOutputLedStatus(0);
			}
		}
		else
		{
			checkOnButton();
			checkOffButton();
		}

		if(electricityStatus==1)
		{
			changeElectricityInidicator(1);
		}
		else
		{
			changeElectricityInidicator(0);
		}
		
	}
	return (0);
}

void checkManualButton()
{
	if(bit_is_low(PIND,1))  //Manual control button pressed (PIND & 0b00000010)==0
	{
		if(manualButtonStatus == 1)
		{
			manualButtonStatus = 0;

			if(isManualControl == 1)
			{
				isManualControl = 0;
			}
			else
			{
				isManualControl = 1;
			}

		}
	}
	else
	{
		if(manualButtonStatus == 0)
		{
			manualButtonStatus = 1;
		}
	}
}

void checkOnButton()
{
	if(bit_is_low(PIND,2))  //Manual control button pressed (PIND & 0b00000010)==0
	{
		if(turnOnButtonStatus == 1)
		{
			turnOnButtonStatus = 0;
			changeOutputLedStatus(1);
		}
	}
	else
	{
		if(turnOnButtonStatus == 0)
		{
			turnOnButtonStatus = 1;
		}
	}
}

void checkOffButton()
{
	if(bit_is_low(PIND,3))  //Manual control button pressed (PIND & 0b00000010)==0
	{
		if(turnOnButtonStatus == 1)
		{
			turnOnButtonStatus = 0;
			changeOutputLedStatus(0);
		}
	}
	else
	{
		if(turnOnButtonStatus == 0)
		{
			turnOnButtonStatus = 1;
		}
	}
}

void checkShowTimeButton(rtc_t rtc)
{
	if(bit_is_low(PIND,4))  //Manual control button pressed (PIND & 0b00000010)==0
	{
		if(showTimeButtonStatus == 1)
		{
			showTimeButtonStatus = 0;
			blinkAccordingToHourNumber(rtc);
		}
	}
	else
	{
		if(showTimeButtonStatus == 0)
		{
			showTimeButtonStatus = 1;
		}
	}

}

void checkElectricityStatus(){
	if(bit_is_low(PIND,0))  //Manual control button pressed (PIND & 0b00000010)==0
	{
		if(electricityStatus == 0)
		{
			electricityStatus = 1;
		}
	}
	else
	{
		if(electricityStatus == 1)
		{
			electricityStatus = 0;
		}
	}
}

void blinkAccordingToHourNumber(rtc_t rtc)
{

	
	uint8_t hourNum = rtc.hour;
	uint8_t i = 0x00;

	for( i = 0x00 ; i < hourNum ; i = i + 0x01)
	{
		wdt_reset();
		if(i == 0x0A || i == 0x0B || i == 0x0C || i == 0x0D || i == 0x0E || i == 0x0F ||
		i == 0x1A || i == 0x1B || i == 0x1C || i == 0x1D || i == 0x1E || i == 0x1F)
		{
			continue;
		}
		PORTB = PORTB | 0b00010000;
		_delay_ms(30);
		wdt_reset();
		PORTB = PORTB & 0b11101111;
		_delay_ms(40);
	}

	_delay_ms(80);

	uint8_t minNum = rtc.min;
	i = 0x00;

	if(minNum>=0x10)
	{
		minNum = minNum - 0x06 ;
	}
	else if( minNum >= 0x20)
	{
		minNum = minNum - 0x0C ;
	}
	else if( minNum >= 0x20)
	{
		minNum = minNum - 0x12 ;
	}
	else if( minNum >= 0x30)
	{
		minNum = minNum - 0x18 ;
	}
	else if( minNum >= 0x40)
	{
		minNum = minNum - 0x1E ;
	}
	else if( minNum >= 0x50)
	{
		minNum = minNum - 0x24 ;
	}

	for( i = 0x00 ; i < minNum ; i = i + 0x01)
	{
		wdt_reset();
		PORTB = PORTB | 0b00010000;
		_delay_ms(20);
		wdt_reset();
		PORTB = PORTB & 0b11101111;
		_delay_ms(30);
	}

}
int isActiveTime(rtc_t rtc){
	if(rtc.weekDay != 5) //Not Friday
	{
		if(rtc.hour>=0x12)
		{
			if(rtc.hour <= 0x18)
			{
				return 1;
			}
		}
	}
	return 0;
}

void changeOutputLedStatus(int status){
	if(status ==1)
	{
		PORTB = PORTB | 0b00000001;
	}
	else
	{
		PORTB = PORTB & 0b11111110;
	}
}

void changeElectricityInidicator(int status){
	if(status ==1)
	{
		PORTB = PORTB & 0b11111101;
	}
	else
	{
		PORTB = PORTB | 0b00000010;
	}
}

void changeTimeInidicator(int status){
	if(status ==1 )
	{
		PORTB = PORTB & 0b11111011;
	}
	else
	{
		PORTB = PORTB | 0b00000100;
		
	}
}

void changeAutoInidicator(int status){
	if(status ==1 )
	{
		PORTB = PORTB & 0b11110111;
	}
	else
	{
		PORTB = PORTB | 0b00001000;
	}
}

void setRtcTime(rtc_t rtc){
	rtc.hour = 0x12; //  10:40:20 am
	rtc.min =  0x02;
	rtc.sec =  0x00;

	rtc.date = 0x25; //1st Jan 2016
	rtc.month = 0x04;
	rtc.year = 0x18;
	rtc.weekDay = 3;
	
	// Friday: 5th day of week considering monday as first day.
	/*##### Set the time and Date only once. Once the Time and Date is set, comment these lines
	and reflash the code. Else the time will be set every time the controller is reset*/
	RTC_SetDateTime(&rtc);  //  10:40:20 am, 1st Jan 2016
}

void initPorts(){
	DDRD = 0x00; // Set PORT D to input
	DDRB = 0xFF; // Set PORT B to output

	PORTD = 0b11111110; //enable pull-ups

	//DDRD &= ~(1<<1); // configure PB0 as an input
	//PORTD |= (1<<1); // enable the pull-up on PB0

	//DDRD &= ~(1 << DDD0);     // Clear the PD0 pin
	//DDRD &= ~(1 << DDD1);     // Clear the PD1 pin
	//DDRD &= ~(1 << DDD2);     // Clear the PD2 pin
	//DDRD &= ~(1 << DDD3);
	//DDRD &= ~(1 << DDD4);
	//
	//PORTD |= (1 << PORTD0);    // turn On the Pull-up
	//PORTD |= (1 << PORTD1);    // turn On the Pull-up
	//PORTD |= (1 << PORTD2);    // turn On the Pull-up
	//PORTD |= (1 << PORTD3);	   // turn On the Pull-up
	
}