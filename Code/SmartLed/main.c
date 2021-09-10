#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "rtc.h"

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
void AddToVoltageHistoryReading(int valueToAdd);
float GetMeanVoltageReading();
int ADCsingleREAD(uint8_t adctouse);
void checkPIRandLDR();
void checkRC5Receiver();

//Button status
int manualButtonStatus = 1;
int turnOnButtonStatus = 1;
int turnOffButtonStatus = 1;
int showTimeButtonStatus = 1;

//General Status
int isManualControl = 0;
int electricityStatus = 0;
int LedStatus = 0;

//RC5 Status
int PINC3RC5Status = 0;
int fallingEdgeCountRC5 = 0;
int modeIndex = 0;

//PIR Status
int previousPINB6PIRReceive = 0;

//LDR
int lastFiveValues[5];
int LDR_THRESHOLD = 400;

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
		checkRC5Receiver();
		checkPIRandLDR();
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

void checkPIRandLDR(){
	int valueOfLDR = ADCsingleREAD(0);
	if(bit_is_high(PINB,6) && previousPINB6PIRReceive == 0) //Rising edge
	{
		previousPINB6PIRReceive = 1;
		if(valueOfLDR > LDR_THRESHOLD && isManualControl == 0){
			isManualControl = 1;
			changeOutputLedStatus(1);
		}
	}
	if(bit_is_low(PINB,6) && previousPINB6PIRReceive == 1) //Falling edge
	{
		previousPINB6PIRReceive = 0;
		if(isManualControl == 1){
			isManualControl = 0;
			changeOutputLedStatus(0);
		}
	}
	//TODO:
	//ADC
	//int valueOfADC = ADCsingleREAD(0);
	//AddToVoltageHistoryReading(valueOfADC);
	//float LDRValue = GetMeanVoltageReading();
}

// NOTE: this is just a stupid receiver, it can only know if a button was pressed without specifying which one.
// To add this capability, the PCB should be updated to use interrupts with the infrared receiver
void checkRC5Receiver() {
	if(bit_is_low(PINC,3) && PINC3RC5Status == 1) //Falling edge
	{
		PINC3RC5Status = 0;
		fallingEdgeCountRC5++;
		
		if(fallingEdgeCountRC5>15){
			fallingEdgeCountRC5 = 0;
			
			if (modeIndex == 0){
				isManualControl = 1;
				changeOutputLedStatus(1);
				modeIndex = 1;
			}
			else if(modeIndex == 1)
			{
				isManualControl = 1;
				changeOutputLedStatus(0);
				modeIndex = 2;
			}
			else {
				isManualControl = 0;
				modeIndex = 0;
			}
		}
	}
	
	if(bit_is_high(PINC,3) && PINC3RC5Status == 0) //Rising edge
	{
		PINC3RC5Status = 1;
	}
}

int ADCsingleREAD(uint8_t adctouse)
{
	int ADCval;

	ADMUX = adctouse;         // use #1 ADC
	ADMUX |= ( (0<<REFS1) | (1<<REFS0) );    // use AVcc as the reference
	ADMUX &= ~(1 << ADLAR);   // clear for 10 bit resolution
	
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale for 8Mhz
	ADCSRA |= (1 << ADEN);    // Enable the ADC

	ADCSRA |= (1 << ADSC);    // Start the ADC conversion

	while(ADCSRA & (1 << ADSC));      // Thanks T, this line waits for the ADC to finish


	ADCval = ADCL;
	ADCval = (ADCH << 8) + ADCval;    // ADCH is read so ADC can be updated again

	return ADCval;
}

float GetMeanVoltageReading(){
	int historySize = 0, arraySize = 5;
	float sum = 0;
	for(int i = 0 ; i < arraySize; i++){
		if(lastFiveValues[i]){
			sum = sum + lastFiveValues[i];
			historySize++;
		}
	}
	return sum/historySize;
}

void AddToVoltageHistoryReading(int valueToAdd){
	lastFiveValues[4] = lastFiveValues[3];
	lastFiveValues[3] = lastFiveValues[2];
	lastFiveValues[2] = lastFiveValues[1];
	lastFiveValues[1] = lastFiveValues[0];
	lastFiveValues[0] = valueToAdd;
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
		if(rtc.hour>=0x04)
		{
			if(rtc.hour <= 0x18)
			{
				return 1;
			}
		}
	}
	//TODO: Fix this hard-coded return value
	//PAY ATENTION: THIS IS HARD-CODED AFTER V3
	return 1; 
}

void changeOutputLedStatus(int status){
	if(status ==1)
	{
		PORTB = PORTB | 0b00000001;
		LedStatus = 1;
	}
	else
	{
		PORTB = PORTB & 0b11111110;
		LedStatus = 0;
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
	rtc.hour = 0x03; //  10:40:20 am
	rtc.min =  0x51;
	rtc.sec =  0x00;

	rtc.date = 0x07; //1st Jan 2016
	rtc.month = 0x09;
	rtc.year = 0x21;
	rtc.weekDay = 2;
	
	// Friday: 5th day of week considering monday as first day.
	/*##### Set the time and Date only once. Once the Time and Date is set, comment these lines
	and reflash the code. Else the time will be set every time the controller is reset*/
	RTC_SetDateTime(&rtc);  //  10:40:20 am, 1st Jan 2016
}

void initPorts(){
	DDRC = DDRC & 0b11111110; // Set PIN C.0 to input
	DDRD = 0x00; // Set PORT D to input
	DDRB = 0xBF; // Set PORT B to output except PB.6

	PORTD = 0b11111110; //enable pull-ups
}