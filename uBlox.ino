/* uBlox.ino - a simple set of primitives for uBlox GPS modules */
/*
  Copyright (c) 2015 Roberto Cazzaro.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

*/

void SendUBX(uint8_t *MSG, uint8_t len)    // sends UBX message to uBlox units. Compatible with uBLox6, 7 and 8
{
  for (int i = 0; i<len; i++)
  {
    GPS.write(MSG[i]);
  }
}

bool SetUBX()    // Set uBlox to use poll mode, disable all NMEA messages from the serial port we use
{
  int TimeOut = 0;
  // set UBX protocol-only on PORT1, 9600 baud. It removes NMEA from Port 1, de facto disables all messages
  uint8_t setNav[] = {
  0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00,
  0x80, 0x25, 0x00, 0x00, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xA9 
  };
	
  while(true)    //loop until one of the exit conditions is met
  {
    GPS.flush();
    SendUBX(setNav, sizeof(setNav)/sizeof(uint8_t));  // send new configuration info
    
    for (int loopc = 0; loopc<100; loopc++)
    {
      if (GPS.available() == 0) 
      {
        TimeOut += 1;
        if (TimeOut >20 ) return false;
        delay(25);    // wait for data from uBlox, time out after 500 msec
      }
      else 
      {
        break;    // got something from uBlox, exit loop
      }
    }
    if (GPS.available() > 0)  // if we got data back, store return code into AckCode
    {
      TimeOut = 0;     //reset TimeOut to 0
      String AckCode = "";
      while (GPS.available())
      {
        unsigned char c = GPS.read();
        String HexValue = String(c, HEX);
        if (c<0x10) HexValue = "0" + HexValue;  //add leading zero in Hex value
        AckCode += HexValue;
      }

      if (AckCode == "b5620501020006000e37")   // check if we got the right code
      {
        return true;                          // if so, return true
      }
      else
      {
        TimeOut += 1;
        if (TimeOut >20 ) return false;    //if not, wait a bit longer, then return false if timeout
        delay(5);
      }
    }
  }
}

bool ReadUBX(uBlox_t* object) //sends Poll request to uBlox, parses values into struct
{
  int TimeOut=0;
  String sValue="";

  GPS.println("$PUBX,00*33");  // poll navigation data
  while (!GPS.available())    // wait for uBlox to respond
  {
    delay(10);
    TimeOut += 1;
    if (TimeOut > 255) break;  // but don't wait forever
  }

  delay(25);    ///HACKHACK: sometimes buffer starts filling but Arduino processes faster than uBlox sends data, hence a delay is required for the buffer to be complete
  while (GPS.available())
  {
    sValue=sValue + (char) GPS.read(); // reads buffer into String sValue; must use the (char) cast otherwise String concatenation uses integer value instead
  }
  Serial.println(sValue);

  // expects string in the format: $PUBX,00,hhmmss.ss,Latitude,N,Longitude,E,AltRef,NavStat,Hacc,Vacc,SOG,COG,Vvel,ageC,HDOP,VDOP,TDOP,GU,RU,DR,*cs<CR><LF>
  if (sValue.length() > 50)  // check that we read enough characters
  {

// ***** TODO: check checksum

    if (sValue.startsWith("$PUBX,") && sValue.charAt(sValue.length()-5) == '*')       // check that string starts with $PUBX and there is a checksumm
    {
      object->MsgID = "$PUBX";
      int count = sValue.indexOf(",");  // count points to next parameter, using ',' as separator
      count = sValue.indexOf(",", count+1);     // skips second parameter, always 0
      
      int count1 = sValue.indexOf(",", count+1);    // count points to ',' before parameter, count1 to ',' after
      
      object-> sTime = sValue.substring(count+1, count1);   // start populating uBlox structure
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
      object-> Lat = sValue.substring(count+1, count1);
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
      object-> NS = sValue.substring(count+1, count1);
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
      object-> Long = sValue.substring(count+1, count1);
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
      object-> EW = sValue.substring(count+1, count1);
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
      object-> Alt = sValue.substring(count+1, count1);
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
      object-> Nav = sValue.substring(count+1, count1);
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
      object-> HAcc = sValue.substring(count+1, count1);
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
//      object-> VAcc = sValue.substring(count+1, count1);   // skip VAcc, 
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
      object-> Speed = sValue.substring(count+1, count1);
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
//      object-> COG = sValue.substring(count+1, count1);   // skip COG
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
//      object-> VVel = sValue.substring(count+1, count1);   // skip VVel
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
//      object-> AgeC = sValue.substring(count+1, count1);   // skip AgeC
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
      object-> HDOP = sValue.substring(count+1, count1);
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
//      object-> VDOP = sValue.substring(count+1, count1);   // skip VDOP
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
//      object-> TDOP = sValue.substring(count+1, count1);   // skip TDOP
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
      object-> GpsSat = sValue.substring(count+1, count1);
      count = sValue.indexOf(",", count1);     
      count1 = sValue.indexOf(",", count+1);
      object-> Glonass = sValue.substring(count+1, count1);
      
      TimeOut=0;
      sValue="";
      GPS.println("$PUBX,04*37"); // Poll time and date, needed for NMEA messages
      while (!GPS.available())    // wait for uBlox to respond
      {
        delay(10);
        TimeOut += 1;
        if (TimeOut > 255) break;  // but don't wait forever
      }

      delay(25);    ///HACKHACK: sometimes buffer starts filling but Arduino processes faster than uBlox sends data, hence a delay is required for the buffer to be complete
      while (GPS.available())
      {
        sValue=sValue + (char) GPS.read(); // reads buffer into String sValue; must use the (char) cast otherwise String concatenation uses integer value instead
      }
  
      if (sValue.length() > 40)  // check that we read enough characters
      {

// ***** TODO: check checksum

        if (sValue.startsWith("$PUBX,") && sValue.charAt(sValue.length()-5) == '*')       // check that string starts with $PUBX and there is a checksumm
        {
          object->MsgID = "$PUBX";
          int count = sValue.indexOf(",");  // count points to next parameter, using ',' as separator
          count = sValue.indexOf(",", count+1);     // skips second parameter, always 04
      
          int count1 = sValue.indexOf(",", count+1);    // count points to ',' before parameter, count1 to ',' after
      
//      object-> sTime = sValue.substring(count+1, count1);  // Skip time, we have it from before
          count = sValue.indexOf(",", count1);     
          count1 = sValue.indexOf(",", count+1);
          object-> sDate = sValue.substring(count+1, count1);
          // no need to parse anything else, we only needed Date
          return true ;
        }
      }
      else return false;
    }
  }
  else return false;
}
