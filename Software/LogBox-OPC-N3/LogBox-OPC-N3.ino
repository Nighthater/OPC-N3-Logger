/*
██╗░░░░░░█████╗░░██████╗░██████╗░░█████╗░██╗░░██╗  ░█████╗░██████╗░░█████╗░░░░░░░███╗░░██╗██████╗░
██║░░░░░██╔══██╗██╔════╝░██╔══██╗██╔══██╗╚██╗██╔╝  ██╔══██╗██╔══██╗██╔══██╗░░░░░░████╗░██║╚════██╗
██║░░░░░██║░░██║██║░░██╗░██████╦╝██║░░██║░╚███╔╝░  ██║░░██║██████╔╝██║░░╚═╝█████╗██╔██╗██║░█████╔╝
██║░░░░░██║░░██║██║░░╚██╗██╔══██╗██║░░██║░██╔██╗░  ██║░░██║██╔═══╝░██║░░██╗╚════╝██║╚████║░╚═══██╗
███████╗╚█████╔╝╚██████╔╝██████╦╝╚█████╔╝██╔╝╚██╗  ╚█████╔╝██║░░░░░╚█████╔╝░░░░░░██║░╚███║██████╔╝
╚══════╝░╚════╝░░╚═════╝░╚═════╝░░╚════╝░╚═╝░░╚═╝  ░╚════╝░╚═╝░░░░░░╚════╝░░░░░░░╚═╝░░╚══╝╚═════╝░


MIT License

Copyright (c) 2023 NIGHTHATER

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


//█▄▄ ▄▀█ █▀ █ █▀▀   █░░ █ █▄▄ █▀█ ▄▀█ █▀█ █ █▀▀ █▀
//█▄█ █▀█ ▄█ █ █▄▄   █▄▄ █ █▄█ █▀▄ █▀█ █▀▄ █ ██▄ ▄█


#include "math.h"
#include <Wire.h>
#include <SPI.h>

// RTC | by Makuna
// https://www.arduino.cc/reference/en/libraries/rtc-by-makuna/
#include <RtcDS3231.h>

// OLED DISPLAY SSD1306 | by Adafruit
// https://www.arduino.cc/reference/en/libraries/adafruit-ssd1306/
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <splash.h>

// OPC-N3 Aerosol Sensor | by marlon360
// https://github.com/marlon360/opcn3-arduino
#include <OPCN3.h>

// SD Card SPI Reader | by Arduino
// https://www.arduino.cc/reference/en/libraries/sd/
#include <SD.h>


//█▀▄ █▀▀ █▀▀ █░░ ▄▀█ █▀█ ▄▀█ ▀█▀ █ █▀█ █▄░█ █▀
//█▄▀ ██▄ █▄▄ █▄▄ █▀█ █▀▄ █▀█ ░█░ █ █▄█ █░▀█ ▄█

//Pins
//Empty | Reserverd for UTIL Pins

//RTC
RtcDS3231<TwoWire> Rtc(Wire);

//OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//OPC-N3
OPCN3 myOPCN3(15); // SS PIN in constructor

//SD Card Reader
const int CSpin = 16; //CS PIN SD CARD
File sensorData;


//█░█ ▄▀█ █▀█ █ ▄▀█ █▄▄ █░░ █▀▀ █▀
//▀▄▀ █▀█ █▀▄ █ █▀█ █▄█ █▄▄ ██▄ ▄█

//Measurements
float VAR_Temperature = 0.0;
float VAR_Humidity = 0.0;
float VAR_PM_10 = 0.0;
float VAR_PM_2_5 = 0.0;
float VAR_PM_1 = 0.0;

uint16_t VAR_BINS[24];


//Current Times
int TIME_Year = 0;
int TIME_Month = 0;
int TIME_Day = 0;
int TIME_Hour = 0;
int TIME_Min = 0;
int TIME_Sec = 0;

//File name
String IDevice = "G03"; // Change for Device Name //------------------------------------------------------------------------------------------------------- CUSTOMIZED
String TMonth = "";
String TDay = "";
String THour = "";
String TMin = "";
String TSec = "";
String FileName = "";

bool FLAG_new_file = false;

//Measurement index count
int VAR_Index = 0;

//Timer and Timing variables
unsigned long TIME_last_SD_save;
unsigned long TIME_last_display_update;
unsigned long TIME_last_RTC_update;
unsigned long TIME_last_OPC_readout;
unsigned long TIME_measurement_start;

//Timer Intervals in ms
int INTERVAL_SD_Save 		= 	5000; 			// Determines the Interval in which measurements are taken / 	in miliseconds 5000ms recommended
int INTERVAL_Display_Update 	= 	200;			// Determines the Interval in which Display is updated / 	in miliseconds 200ms recommended
int INTERVAL_RTC_Update 	= 	200;			// Determines the Interval in which RTC is updated / 		in miliseconds 200ms recommended
int INTERVAL_OPC_Readout 	= 	2000;			// Determines the Interval in which OPC is updated / 		in miliseconds 2000ms recommended

//------------------------------------------//
//░██████╗███████╗████████╗██╗░░░██╗██████╗░//
//██╔════╝██╔════╝╚══██╔══╝██║░░░██║██╔══██╗//
//╚█████╗░█████╗░░░░░██║░░░██║░░░██║██████╔╝//
//░╚═══██╗██╔══╝░░░░░██║░░░██║░░░██║██╔═══╝░//
//██████╔╝███████╗░░░██║░░░╚██████╔╝██║░░░░░//
//╚═════╝░╚══════╝░░░╚═╝░░░░╚═════╝░╚═╝░░░░░//
//------------------------------------------//

void setup() {
	// Begin of Script
	//Errorcodes
	// 0x1x SD Card
	// 0x2x Display
	// 0x3x
	// 0x4x
	
	// Begin Serial Comms
	Serial.begin(115200);
	delay(random(50,150));
	
	//initialize OLED Display
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
		Serial.println(F("FATAL ERROR 0x2B - SSD1306 allocation failed"));
		for(;;); // Don't proceed, loop forever
	}

	display.clearDisplay();
	display.display();

	//Serial
	
	
	//Only the logo
	OLED_ASYNC_display_Startup_Logo();
	delay(random(1000,2000));
	
	//Initialize the Status bar
	OLED_ASYNC_display_Startup_Bar();
	delay(random(1500,2500));
	
	OLED_ASYNC_display_Startup_Bar_Fill(random(2,8));
	delay(random(50,300));

	Serial.print(F("compiled: "));
	Serial.print(__DATE__);
	Serial.println(__TIME__);

	//Initialize RTC
	Rtc.Begin();     //Starts I2C
	Rtc.Enable32kHzPin(false);
	Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 

	OLED_ASYNC_display_Startup_Bar_Fill(random(13,20));
	delay(random(250,750));

	//Get Curent Date and Time
	RtcDateTime now = Rtc.GetDateTime();

	TIME_Year = now.Year();
	TIME_Month = now.Month();
	TIME_Day = now.Day();
	TIME_Hour = now.Hour();
	TIME_Min = now.Minute();
	TIME_Sec = now.Second();

	//Prepare the Filename
	TMonth = String(TIME_Month);
	TDay = String(TIME_Day);
	THour = String(TIME_Hour);
	TMin = String(TIME_Min);
	TSec = String(TIME_Sec);

	//Append zeroes to Month, Day, etc, if neccescary
	if(TIME_Month<10)
	{
		TMonth = '0' + String(TIME_Month);
	}
	if(TIME_Day<10)
	{
		TDay = '0' + String(TIME_Day);
	}
	if(TIME_Hour<10)
	{
		THour = '0' + String(TIME_Hour);
	}
	if(TIME_Min<10)
	{
		TMin = '0' + String(TIME_Min);
	}
	if(TIME_Sec<10)
	{
		TSec = '0' + String(TIME_Sec);
	}
	//Create Filename
	FileName = IDevice + '_' + String(TIME_Year) + '-' + TMonth + '-' + TDay + 'T' + THour + '-' + TMin + '-' + TSec + ".csv";

	OLED_ASYNC_display_Startup_Bar_Fill(random(25,35));
	delay(random(250,750));

	//initialize OPC-N3
	// turn on laser, fan and set high gain, use delay to make sure it is setup proper.
	delay(50);
	myOPCN3.initialize();
	delay(500);

	OLED_ASYNC_display_Startup_Bar_Fill(random(45,65));
	delay(random(500,1000));

	//Initialize SD CARD
	pinMode(CSpin, OUTPUT);
	if (!SD.begin(CSpin)) {
		Serial.println(F("FATAL ERROR 0x1A - SD-Card failed, or not present"));
		// don't do anything more:
		for(;;);
	}
	Serial.println(F("SD Card initialized."));
	//Write new File

	sensorData = SD.open(FileName, FILE_WRITE);
	sensorData.close();  //Closing the file

	Serial.println(FileName);

	OLED_ASYNC_display_Startup_Bar_Fill(random(70,80));
	delay(random(1000,1500));

	sensorData = SD.open(FileName, FILE_WRITE);
	if (sensorData) {
		//Write csv header
		sensorData.println("TIME;INDEX;TEMPERATURE;HUMIDITY;PM10;PM2.5;PM1;BIN0;BIN1;BIN2;BIN3;BIN4;BIN5;BIN6;BIN7;BIN8;BIN9;BIN10;BIN11;BIN12;BIN13;BIN14;BIN15;BIN16;BIN17;BIN18;BIN19;BIN20;BIN21;BIN22;BIN23");
		// close the file:
		sensorData.close();
		Serial.print(F("New File Created: "));
		Serial.println(FileName);
	} else {
		Serial.println(F("FATAL ERROR 0x1B - Cannot write to new File"));
		for(;;);
	}

	OLED_ASYNC_display_Startup_Bar_Fill(100);
	delay(random(1000,1500));

	display.clearDisplay();
	display.display();
	delay(random(150,450));

	//get the measurement since the start
	TIME_measurement_start = millis();

	//set the timing variables to relative zero for the main loop
	TIME_last_SD_save = millis();
	TIME_last_display_update = millis();
	TIME_last_RTC_update = millis();
	TIME_last_OPC_readout = millis();


  SPI.setClockDivider(SPI_CLOCK_DIV32);
	Serial.println(F("Timers set, Setup complete"));

}


//--------------------------------//
//██╗░░░░░░█████╗░░█████╗░██████╗░//
//██║░░░░░██╔══██╗██╔══██╗██╔══██╗//
//██║░░░░░██║░░██║██║░░██║██████╔╝//
//██║░░░░░██║░░██║██║░░██║██╔═══╝░//
//███████╗╚█████╔╝╚█████╔╝██║░░░░░//
//╚══════╝░╚════╝░░╚════╝░╚═╝░░░░░//
//--------------------------------//


void loop() {
	//All Values, Inputs and Outputs will be updated asyncronous

	//Check for RTC Update
	if(millis() - TIME_last_RTC_update > INTERVAL_RTC_Update)
	{
		//UPDATE RTC
		TIME_last_RTC_update = millis() - (millis() - TIME_last_RTC_update - INTERVAL_RTC_Update);
		RtcDateTime now = Rtc.GetDateTime();

		TIME_Year = now.Year();
		TIME_Month = now.Month();
		TIME_Day = now.Day();
		TIME_Hour = now.Hour();
		TIME_Min = now.Minute();
		TIME_Sec = now.Second();

	}

	//Check for Display Update
	if(millis() - TIME_last_display_update > INTERVAL_Display_Update)
	{
		//UPDATE DISPLAY
		TIME_last_display_update = millis() - (millis() - TIME_last_display_update - INTERVAL_Display_Update);

		//Upper Part
		OLED_ASYNC_display_divider();
		OLED_ASYNC_display_clock(TIME_Hour,TIME_Min,TIME_Sec);
		OLED_ASYNC_display_HBRS_Logo_top_right();

		//Lower Part
		OLED_ASYNC_display_Identifier(IDevice);
		OLED_ASYNC_display_vertical_divider();

		OLED_ASYNC_display_Elapsed_Time();
		OLED_ASYNC_display_HUM_and_TMP(VAR_Temperature,int(VAR_Humidity));
		OLED_ASYNC_display_PM_values(VAR_PM_10,VAR_PM_2_5,VAR_PM_1);
	}

	//Check for OPC Readout
	if(millis() - TIME_last_OPC_readout > INTERVAL_OPC_Readout)
	{
		//UPDATE OPC
		TIME_last_OPC_readout = millis() - (millis() - TIME_last_OPC_readout - INTERVAL_OPC_Readout);
		OLED_ASYNC_display_OPC_update_icon(15,3);
		
		//OPC readout:
		SPI.setClockDivider(SPI_CLOCK_DIV32);
		SPI.setDataMode(SPI_MODE1);
		HistogramData hist = myOPCN3.readHistogramData();
		
		VAR_Temperature = hist.getTempC();
    VAR_Humidity = hist.getHumidity();
		
		VAR_PM_10 = hist.pm10;
		VAR_PM_2_5 = hist.pm2_5;
		VAR_PM_1 = hist.pm1;

    VAR_BINS[0] = hist.binCount0;
    VAR_BINS[1] = hist.binCount1;
    VAR_BINS[2] = hist.binCount2;
    VAR_BINS[3] = hist.binCount3;
    VAR_BINS[4] = hist.binCount4;
    VAR_BINS[5] = hist.binCount5;
    VAR_BINS[6] = hist.binCount6;
    VAR_BINS[7] = hist.binCount7;
    VAR_BINS[8] = hist.binCount8;
    VAR_BINS[9] = hist.binCount9;
    VAR_BINS[10] = hist.binCount10;
    VAR_BINS[11] = hist.binCount11;
    VAR_BINS[12] = hist.binCount12;
    VAR_BINS[13] = hist.binCount13;
    VAR_BINS[14] = hist.binCount14;
    VAR_BINS[15] = hist.binCount15;
    VAR_BINS[16] = hist.binCount16;
    VAR_BINS[17] = hist.binCount17;
    VAR_BINS[18] = hist.binCount18;
    VAR_BINS[19] = hist.binCount19;
    VAR_BINS[20] = hist.binCount20;
    VAR_BINS[21] = hist.binCount21;
    VAR_BINS[22] = hist.binCount22;
    VAR_BINS[23] = hist.binCount23;
		
		delay(100);
		OLED_ASYNC_remove_OPC_update_icon(15,3);

	}

	//Check for SD Card save
	if(millis() - TIME_last_SD_save > INTERVAL_SD_Save)
	{
		SPI.setDataMode(SPI_MODE0);
		//UPDATE SD card
		TIME_last_SD_save = millis() - (millis() - TIME_last_SD_save - INTERVAL_SD_Save);

		OLED_ASYNC_display_save_icon(3,3);
		saveData(sensorData, FileName, VAR_Index, VAR_Temperature, VAR_PM_10, VAR_PM_2_5, VAR_PM_1, TIME_Hour, TIME_Min, TIME_Sec, VAR_BINS);
		VAR_Index = VAR_Index + 1;
		
		if( (VAR_Index % 17280 == 0) && (VAR_Index != 0) ) //Since VAR_Index is a int it can only reach up to ~32k until it flips into the negative
		{
			//Schedule creation of new File every 17280 entries (24 h)
			FLAG_new_file = true;
		}
		delay(100);
		OLED_ASYNC_remove_save_icon(3,3);
	}
	
	if(FLAG_new_file == true)
	{
		FLAG_new_file = false;
		
		//Create new File
		//Prepare the Filename
		TMonth = String(TIME_Month);
		TDay = String(TIME_Day);
		THour = String(TIME_Hour);
		TMin = String(TIME_Min);
		TSec = String(TIME_Sec);

		//Append zeroes to Month, Day, etc, if neccescary
		if(TIME_Month<10)
		{
			TMonth = '0' + String(TIME_Month);
		}
		if(TIME_Day<10)
		{
			TDay = '0' + String(TIME_Day);
		}
		if(TIME_Hour<10)
		{
			THour = '0' + String(TIME_Hour);
		}
		if(TIME_Min<10)
		{
			TMin = '0' + String(TIME_Min);
		}
		if(TIME_Sec<10)
		{
			TSec = '0' + String(TIME_Sec);
		}
		//Create Filename
		FileName = IDevice + '_' + String(TIME_Year) + '-' + TMonth + '-' + TDay + 'T' + THour + '-' + TMin + '-' + TSec + ".csv";
		
		sensorData = SD.open(FileName, FILE_WRITE);
		sensorData.close();  //Closing the file

		sensorData = SD.open(FileName, FILE_WRITE);
		if (sensorData) {
			//Write csv header
			sensorData.println("TIME;INDEX;TEMPERATURE;HUMIDITY;PM10;PM2.5;PM1;BIN0;BIN1;BIN2;BIN3;BIN4;BIN5;BIN6;BIN7;BIN8;BIN9;BIN10;BIN11;BIN12;BIN13;BIN14;BIN15;BIN16;BIN17;BIN18;BIN19;BIN20;BIN21;BIN22;BIN23");
			// close the file:
			sensorData.close();
			Serial.print(F("New File Created: "));
			Serial.println(FileName);
		} else {
			Serial.println(F("FATAL ERROR 0x1B - Cannot write to new File"));
			for(;;);
		}
	}

}



//---------------------------------------------------------------------------------------//
//░██████╗██╗░░░██╗██████╗░██████╗░░█████╗░██╗░░░██╗████████╗██╗███╗░░██╗███████╗░██████╗//
//██╔════╝██║░░░██║██╔══██╗██╔══██╗██╔══██╗██║░░░██║╚══██╔══╝██║████╗░██║██╔════╝██╔════╝//
//╚█████╗░██║░░░██║██████╦╝██████╔╝██║░░██║██║░░░██║░░░██║░░░██║██╔██╗██║█████╗░░╚█████╗░//
//░╚═══██╗██║░░░██║██╔══██╗██╔══██╗██║░░██║██║░░░██║░░░██║░░░██║██║╚████║██╔══╝░░░╚═══██╗//
//██████╔╝╚██████╔╝██████╦╝██║░░██║╚█████╔╝╚██████╔╝░░░██║░░░██║██║░╚███║███████╗██████╔╝//
//╚═════╝░░╚═════╝░╚═════╝░╚═╝░░╚═╝░╚════╝░░╚═════╝░░░░╚═╝░░░╚═╝╚═╝░░╚══╝╚══════╝╚═════╝░//
//---------------------------------------------------------------------------------------//


//█▀ █▀▄   █▀▀ ▄▀█ █▀█ █▀▄   █▀ ▄▀█ █░█ █ █▄░█ █▀▀
//▄█ █▄▀   █▄▄ █▀█ █▀▄ █▄▀   ▄█ █▀█ ▀▄▀ █ █░▀█ █▄█

void saveData(File sensorData, String FileName, int VAR_Index, float VAR_Temperature, float VAR_PM_10, float VAR_PM_2_5, float VAR_PM_1, int TIME_Hour, int TIME_Min, int TIME_Sec, uint16_t VAR_BINS[24]){
	if(SD.exists(FileName)) // check the file is still there
	//sensorData.println("DATE;TIME;INDEX;TEMPERATURE;HUMIDITY;PM10;PM2.5;PM1;BIN0;BIN1;BIN2;BIN3;BIN4;BIN5;BIN6;BIN7;BIN8;BIN9;BIN10;BIN11;BIN12;BIN13;BIN14;BIN15;BIN16;BIN17;BIN18;BIN19;BIN20;BIN21;BIN22;BIN23");
	{ 
		// now append new data file
		sensorData = SD.open(FileName, FILE_WRITE);
		if (sensorData)
		{
			//Write to SD Card
			
			//sensorData.print(DATE); //STILL TODO
			sensorData.print(TIME_Hour);
			sensorData.print(':');
			sensorData.print(TIME_Min);
			sensorData.print(':');
			sensorData.print(TIME_Sec);
			sensorData.print(';');
			sensorData.print(VAR_Index);
			sensorData.print(';');
			sensorData.print(VAR_Temperature,1); // Maybe replace . with ,
			sensorData.print(';');
			sensorData.print(float(VAR_Humidity),0); // Maybe replace . with ,
			sensorData.print(';');
			sensorData.print(VAR_PM_10,3); // Maybe replace . with ,
			sensorData.print(';');
			sensorData.print(VAR_PM_2_5,3); // Maybe replace . with ,
			sensorData.print(';');
			sensorData.print(VAR_PM_1,3); // Maybe replace . with ,
      for(int i=0;i<=24;i++)
      {
          sensorData.print(';');
          sensorData.print(VAR_BINS[i]);
          
      }
      sensorData.println(';');
			sensorData.close(); // close the file
			
			//Print on Console
			//Serial.print(DATE); //STILL TODO
			Serial.print(VAR_Index);
			Serial.print(';');
			Serial.print(TIME_Hour);
			Serial.print(':');
			Serial.print(TIME_Min);
			Serial.print(':');
			Serial.print(TIME_Sec);
			Serial.print(';');
			Serial.print(VAR_Temperature); // Maybe replace . with ,
			Serial.print(';');
			Serial.print(float(VAR_Humidity),0); // Maybe replace . with ,
			Serial.print(';');
			Serial.print(VAR_PM_10); // Maybe replace . with ,
			Serial.print(';');
			Serial.print(VAR_PM_2_5); // Maybe replace . with ,
			Serial.print(';');
			Serial.print(VAR_PM_1); // Maybe replace . with ,
      for(int i=0;i<=24;i++)
      {
          Serial.print(';');
          Serial.print(VAR_BINS[i]);
          
      }
      Serial.println(';');
		}
	}
	else{
		Serial.println("0x1A - Error writing to file !");
		//TODO ADD ALERT
	}
}


//█▀ ▀█▀ ▄▀█ █▀█ ▀█▀ █░█ █▀█
//▄█ ░█░ █▀█ █▀▄ ░█░ █▄█ █▀▀

void OLED_ASYNC_display_Startup_Logo()
{
	display.clearDisplay();
	display.fillCircle(47, 28, 11, WHITE);
	display.fillCircle(47, 28, 6, BLACK);
	display.fillCircle(80, 28, 11, WHITE);
	display.display();
}

void OLED_ASYNC_display_Startup_Bar()
{
	display.fillRect(19,48,90,5,WHITE);
	display.fillRect(18,49,92,3,WHITE);
	display.fillRect(19,49,90,3,BLACK);
	display.display();
}

void OLED_ASYNC_display_Startup_Bar_Fill(int percentage)
{
	display.drawFastHLine(20,50,int(float(percentage)/100.0*88.0),WHITE);
	display.display();
}

//█▀█ █░░ █▀▀ █▀▄   █░█ █
//█▄█ █▄▄ ██▄ █▄▀   █▄█ █

void OLED_ASYNC_display_divider()
//shows a horizontal line dividing the display
//0,0 is at top right
{
	display.drawFastHLine(0,15,128,WHITE);
	display.display();
}

void OLED_ASYNC_display_vertical_divider()
{
	display.drawFastVLine(71,19,31,WHITE);
	display.display();
}

void OLED_ASYNC_display_save_icon(int x, int y)
// displays a save icon at x,y where the coords are the top right corner
{
	display.drawFastHLine(x,y+7,8,WHITE); // Bottom Outine
	display.drawFastHLine(x+1,y+1,6,WHITE); // Top Outline
	display.drawFastVLine(x,y+2,5,WHITE); // Left Outline
	display.drawFastVLine(x+7,y+2,5,WHITE); // Right Outline

	display.drawFastVLine(x+2,y+4,3,WHITE); // Left Faceline
	display.drawFastVLine(x+5,y+4,3,WHITE); // Right Faceline
	display.drawFastHLine(x+2,y+4,3,WHITE); // Top Faceline

	display.drawPixel(x+2,y+1,WHITE);
	display.drawPixel(x+3,y+2,WHITE);
	display.drawPixel(x+4,y+2,WHITE);
	display.drawPixel(x+5,y+1,WHITE); //Metal Tray Pixels
	display.display();
}

void OLED_ASYNC_remove_save_icon(int x, int y)
{
	display.fillRect(x,y,8,8,BLACK);
	display.display();
}

void OLED_ASYNC_display_clock(int TIME_Hour, int TIME_Min, int TIME_Sec)
//draws the current clock
{
	display.fillRect(41,4,49,10,BLACK); // black out current clock

	display.setCursor(41,4);
	display.setTextColor(WHITE);
	if(TIME_Hour<10)
	{
		display.print('0');
	}
	display.print(String(TIME_Hour)+':');
	if(TIME_Min<10)
	{
		display.print('0');
	}
	display.print(String(TIME_Min)+':');
	if(TIME_Sec<10)
	{
		display.print('0');
	}
	display.print(TIME_Sec);
	display.display();
}

void OLED_ASYNC_display_HBRS_Logo_top_right()
// Draws the HBRS logo in the top Right
{
	display.fillCircle(105, 7, 4, WHITE);
	display.fillCircle(105, 7, 2, BLACK);
	display.fillCircle(118, 7, 4, WHITE);
	display.display();
}

void OLED_ASYNC_display_OPC_update_icon(int x, int y)
{
	display.drawFastVLine(x  ,y+6,2,WHITE);
	display.drawFastVLine(x+2,y+4,4,WHITE);
	display.drawFastVLine(x+4,y+2,6,WHITE);
	display.drawFastVLine(x+6,y  ,8,WHITE);
	display.display();
}

void OLED_ASYNC_remove_OPC_update_icon(int x, int y)
{
	display.fillRect(x,y,7,8,BLACK);
	display.display();
}

void OLED_ASYNC_display_Elapsed_Time()
{
	unsigned long currentMillis = millis();
	unsigned long seconds = currentMillis / 1000;
	unsigned long minutes = seconds / 60;
	unsigned long hours = minutes / 60;
	unsigned long days = hours / 24;
	currentMillis %= 1000;
	seconds %= 60;
	minutes %= 60;
	hours %= 24;

	//Fill the previously written pixels with Black
	display.fillRect(80,41,48,7,BLACK);

	display.setCursor(80,41);
	//if minutes<10 then print a 0?

	display.print("E ");
	display.print(hours);
	display.print("h ");
	display.print(minutes);
	display.print("m");
	display.display();
}

void OLED_ASYNC_display_Identifier(String IDevice)
{
	display.fillRect(20,54,104,7,BLACK);
	display.setCursor(20,54);
	display.print("OPC-N3-HBRS-");
  display.print(IDevice);

	display.display();
}

void OLED_ASYNC_display_HUM_and_TMP(float temperature, int humidity)
{
	display.fillRect(80,21,48,7,BLACK);
	display.fillRect(80,31,48,7,BLACK);
	display.setCursor(80,21);
	display.print("T ");
	display.print(temperature, 1);
	display.print(" C");

	display.setCursor(80,31);
	display.print("H ");
	display.print(humidity);
	display.print("%");

	display.display();
}

void OLED_ASYNC_display_PM_values(float PM_10 ,float PM_2_5, float PM_1)
{
	display.fillRect(8,21,62,7,BLACK);
	display.fillRect(8,31,62,7,BLACK);
	display.fillRect(8,41,48,7,BLACK);

	display.setCursor(8,21);
	display.print(PM_10,1);
	display.print("ug/m3");

	display.setCursor(8,31);
	display.print(PM_2_5,1);
	display.print("ug/m3");

	display.setCursor(8,41);
	display.print(PM_1,1);
	display.print("ug/m3");

	display.display();
}
