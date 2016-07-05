/* LoRaTx.ino - a sample LoRa module */
/*
  Copyright (c) 2015 Roberto Cazzaro.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
*/

// Arduino pins for LoRa DRF1278F
// Arduino pin -> DRF1278F pin, Function
// D7 -> LED2 (to show when LoRa module is active)
// D8 -> 1 RESET
// D9 -> 4 DIO2, used for PWM 
// D10 -> 13 NSS
// D11 -> 12 MOSI
// D12 -> 11 MISO 
// D13 -> 10 SCK 

// Arduino pins for uBlox
// D2 to GPS Tx
// D3 to GPS Rx 

// Other pins
// A0 -> main power supply voltage sensing

#include "uBloxLib.h";
#include "SPI.h";
#include <SoftwareSerial.h>;  // remember to increase buffer to 128 in SoftwareSerial.h #define _SS_MAX_RX_BUFF 128 // RX buffer size TODO
SoftwareSerial GPS(2,3);	// RX, TX

// Pin assignments
const byte LoRa_NSS = 10;   //LoRa NSS
const byte LoRa_PWM = 9;    //DIO2 used for PWM/Tone generation
const byte LoRa_Reset = 8;  //LoRa RESET
const byte LoRa_Active = 7; // LoRa Activity LED
const byte V_Sense = 0;     // A0 used to sense main power supply

//LoRa Tx/Rx constants
const float LoRa_Freq = 434.700;   // change this to change standard frequency
const byte LoRa_PktSystem = 1;     // system message (main power voltage), 1 byte
const byte LoRa_PktMin = 5;        // minimum gps info (lat, long, fix/HDOP), 9 bytes
const byte LoRa_PktGPS = 10;       // messages containing full location info, 23 bytes

const float uBlox_GoodHDOP = 3.50f;   // define threshold for what constitutes a good HDOP lock for take off

#include "LoRaTX.h";

uBlox_t uBlox;  //defined uBlox structure. Need struct in its own header file to work around Arduino limitations
#include "Utils.h";

String InputString = "";     //data in buffer is copied to this string
String Outputstring = "";
int GpsFix = false;
int GpsHDOP = false;
byte iCtr = 0;      // used to keep track of when to send full message
  
void setup()
{
  Serial.begin(9600);
  Serial.println("LoRa Tx");
  Serial.println();

  // LoRa init
  pinMode(LoRa_Active, OUTPUT);
  digitalWrite(LoRa_Active, LOW);
  pinMode(LoRa_Reset, OUTPUT);
  digitalWrite(LoRa_Reset, LOW);
  pinMode (LoRa_NSS, OUTPUT);
  digitalWrite(LoRa_NSS, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  LoRa_ResetDev();			//Reset LoRa
  LoRa_Setup();				//Initialize LoRa
  LoRa_SetFreqF(LoRa_Freq); 

  // GPS init. Use uBlox poll mode
  GPS.begin(9600); 
  if (!SetUBX())
  {
    Serial.print("There is a problem initializing uBlox");
    LoRa_Notes(1);  // failure sound
  }
  else
  {
    Serial.print("uBlox initialized");
    LoRa_Notes(0);  // success sound
  }
}

void loop()
{
  unsigned char sSerialize[23];  // serialized string
    
  if (ReadUBX(&uBlox))   // Polls uBlox chip and stores current uBlox data in Current struct
  {
    Serial.print("Nav = ");
    Serial.print(uBlox.Nav);
    Serial.print(" GpsFix ");
    Serial.println(GpsFix);
    if (!GpsFix && !(uBlox.Nav == "NF" || uBlox.Nav == "DR" || uBlox.Nav == "TT"))  // if we have a fix (any fix) for the first time, play sound
    {
      GpsFix = true; 
      Serial.print("Sending tone 2 ");
      Serial.println(GpsFix);
      LoRa_Notes(2);  // Close Encounters of the third kind
    }
    if (GpsHDOP <4 && uBlox.HDOP.toFloat() <= uBlox_GoodHDOP)  // if we have a good HDOP 3 times, play sound once
    {

      GpsHDOP++; 
      if (GpsHDOP>3)
      {
        Serial.println("Sending tone 3");
        LoRa_Notes(3);  // finally a good position, play Beethoven's Ode to joy
      }
    }

Serial.print("Free RAM ");
Serial.println(freeRam());
  }
  else
  {
  Serial.println(">>>>>>>> ERROR <<<<<<<<<");
//ToDo <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
// create null string, or send last known good
  }

  LoRa_SetModem(LoRa_BW41_7, LoRa_SF12, LoRa_CR4_5, LoRa_Explicit, LoRa_LowDoptON); //Setup the LoRa modem parameters, lowest speed
  
  iCtr++;
  if (iCtr == (iCtr / 5) * 5)
  {                                             // send long message
    Serialize(sSerialize);  //Serialize uBLox data, ready for transmission
Serial.print("Send long message= ");
Serial.println(iCtr);
    Serial.println(LoRa_SendStr(sSerialize, 23, LoRa_PktGPS, 10, 10));  
    iCtr = 0;
  }
  else
  {                                             // send short message
    SerializeLLH(sSerialize);  //Serialize uBLox data, ready for transmission
Serial.print("Send short message= ");
Serial.println(iCtr);
    Serial.println(LoRa_SendStr(sSerialize, 10, LoRa_PktMin, 10, 10));      
  }

Serial.print("sSerialize ");
for (int i = 0;  i < 23; i++)
{
  if (sSerialize[i] < 0x10) Serial.print("0"); // print leading 0 if required
  Serial.print(sSerialize[i], HEX);
  Serial.print(" ");
}  
Serial.println();

  delay(5000);

/*  Normal rate mode
 *     initLora(); 
  lora_SetModem(lora_BW41_7, lora_SF8, lora_CR4_5, lora_Explicit, lora_LowDoptOFF); //Setup the LoRa modem parameters
  lora_Send(0, 1, 2, 255, thisnode, 10, 10);              //send an alive packet, contains supply volts
 */

/* low rate mode
 * initLora();
    lora_SetModem(lora_BW41_7, lora_SF12, lora_CR4_5, lora_Explicit, lora_LowDoptON); //Setup the LoRa modem parameters
    SendLocation();
 */

}
