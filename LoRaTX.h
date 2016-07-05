/*
**************************************************************************************************
Original libraries Copyright of Stuart Robinson - 02/07/2015 15:00
Modified by Roberto Cazzaro

These programs may be used free of charge for personal, recreational and educational purposes only.

This program, or parts of it, may not be used for or in connection with any commercial purpose without
the explicit permission of the author Stuart Robinson.

The programs are supplied as is, it is up to individual to decide if the programs are suitable for the
intended purpose and free from errors.
**************************************************************************************************
*/

byte  LoRa_TXBUFF[128];			//buffer for packet to send
double LoRa_Frequency;			//last programmed frequency

//LoRa names for bandwidth settings
const byte LoRa_BW7_8 = 0;      //7.8khz
const byte LoRa_BW10_4 = 16;    //10.4khz
const byte LoRa_BW15_6 = 32;    //15.6khz
const byte LoRa_BW20_8 = 48;    //20.8khz
const byte LoRa_BW31_2 = 64;    //31.2khz
const byte LoRa_BW41_7 = 80;    //41.7khz
const byte LoRa_BW62_5 = 96;    //62.5khz
const byte LoRa_BW125 = 112;    //125khz
const byte LoRa_BW250 = 128;    //250khz
const byte LoRa_BW500 = 144;    //500khz

//Spreading Factors
const byte LoRa_SF6 = 6;
const byte LoRa_SF7 = 7;
const byte LoRa_SF8 = 8;
const byte LoRa_SF9 = 9;
const byte LoRa_SF10 = 10;
const byte LoRa_SF11 = 11;
const byte LoRa_SF12 = 12;

//LORA names for coding rate settings
const byte LoRa_CR4_5 = 2;	//4:5
const byte LoRa_CR4_6 = 4;	//4:6
const byte LoRa_CR4_7 = 6;	//4:7
const byte LoRa_CR4_8 = 8;	//4:8

//LORA Header Settings
const byte LoRa_Explicit    = 0;	//Use to set explicit header
const byte LoRa_Implicit    = 1;	//Use to set implicit header

//Misc definitions
const byte LoRa_Deviation = 0x52;
const byte LoRa_LowDoptON = 0x08;       //value to turn low data rate optimization on
const byte LoRa_LowDoptOFF = 0x00;      //value to turn low data rate optimization off
const byte LoRa_PrintASC = 0;           //value to cause buffer print to appear as ASCII
const byte LoRa_PrintNum = 1;           //value to cause buffer print to appear as decimal numbers
const byte LoRa_PrintHEX = 2;           //value to cause buffer print to appear as hexadecimal numbers

//SX1278 Register names
const byte LoRa_RegFifo = 0x00;
const byte LoRa_WRegFifo = 0x80;
const byte LoRa_RegOpMode = 0x01;
const byte LoRa_RegFdevLsb = 0x05;
const byte LoRa_RegFrMsb = 0x06;
const byte LoRa_RegFrMid = 0x07;
const byte LoRa_RegFrLsb = 0x08;
const byte LoRa_RegPaConfig = 0x09;
const byte LoRa_RegOcp = 0x0B;
const byte LoRa_RegLna = 0x0C;
const byte LoRa_RegFifoAddrPtr = 0x0D;
const byte LoRa_RegFifoTxBaseAddr = 0x0E;
const byte LoRa_RegIrqFlagsMask = 0x11;
const byte LoRa_RegIrqFlags = 0x12;
const byte LoRa_RegRxNbBytes = 0x13;
const byte LoRa_RegRxHeaderCntValueMsb = 0x14;
const byte LoRa_RegRxHeaderCntValueLsb = 0x15;
const byte LoRa_RegRxPacketCntValueMsb = 0x16;
const byte LoRa_RegRxPacketCntValueLsb = 0x17;
const byte LoRa_RegPktSnrValue = 0x19;
const byte LoRa_RegPktRssiValue = 0x1A;
const byte LoRa_RegRssiValue = 0x1B;
const byte LoRa_RegFsiMSB = 0x1D;
const byte LoRa_RegFsiLSB = 0x1E;
const byte LoRa_RegModemConfig1 = 0x1D;
const byte LoRa_RegModemConfig2 = 0x1E;
const byte LoRa_RegSymbTimeoutLsb = 0x1F;
const byte LoRa_RegPreambleLsb = 0x21;
const byte LoRa_RegPayloadLength = 0x22;
const byte LoRa_RegFifoRxByteAddr = 0x25;
const byte LoRa_RegModemConfig3 = 0x26;
const byte LoRa_RegPacketConfig2 = 0x31;
const byte LoRa_TXdefaultpower = 10;

void LoRa_ResetDev()
{
  //resets the LoRa device
  //Serial.print("LORA.ResetDev()");
  //Serial.println();
  digitalWrite(LoRa_Reset, LOW);		// take reset line low
  delay(5);
  digitalWrite(LoRa_Reset, HIGH);	// take it high
}


void LoRa_Write(byte LoRa_LReg, byte LoRa_LData)
{
  //write a byte to a LoRa register
  //Serial.print("LoRa_Write() ");
  //LoRa_PrintAsHEX(LoRa_LReg);
  //Serial.print(" ");
  //LoRa_PrintAsHEX(LoRa_LData);
  //Serial.println();
  digitalWrite(LoRa_NSS, LOW);		// set NSS low
  SPI.transfer(LoRa_LReg | 0x80);		// mask address for write
  SPI.transfer(LoRa_LData);			// write the byte
  digitalWrite(LoRa_NSS, HIGH);			// set NSS high
}


byte LoRa_Read(byte LoRa_LReg)
{
  //read a byte to a LoRa register
  //Serial.print("LoRa_Read()");
  //Serial.println();
  byte LoRa_LRegData;
  digitalWrite(LoRa_NSS, LOW);		// set NSS low
  SPI.transfer(LoRa_LReg & 0x7F);		// mask address for read
  LoRa_LRegData = SPI.transfer(0);	// read the byte
  digitalWrite(LoRa_NSS, HIGH);		// set NSS high
  return LoRa_LRegData;
}


void LoRa_SetFreq(byte LoRa_LFMsb, byte LoRa_LFMid, byte LoRa_LFLsb)
{
  LoRa_Write(LoRa_RegFrMsb, LoRa_LFMsb);
  LoRa_Write(LoRa_RegFrMid, LoRa_LFMid);
  LoRa_Write(LoRa_RegFrLsb, LoRa_LFLsb);
  Serial.println(LoRa_LFMsb);
  Serial.println(LoRa_LFMid);
  Serial.println(LoRa_LFLsb);
}


void LoRa_SetFreqF(float LoRa_LFreq)
{
  //set the LoRa frequency
  byte LoRa_LFMsb, LoRa_LFMid, LoRa_LFLsb;
  long LoRa_LLongFreq;
  LoRa_LLongFreq = ((LoRa_LFreq * 1000000) / 61.03515625);
  Serial.print("LoRa_setFreq() ");
  Serial.print(LoRa_LFreq);
  Serial.print(" 0x");
  Serial.print(LoRa_LLongFreq, HEX);
  Serial.println();
  LoRa_LFMsb =  LoRa_LLongFreq >> 16;
  LoRa_LFMid = (LoRa_LLongFreq & 0x00FF00) >> 8;
  LoRa_LFLsb = (LoRa_LLongFreq & 0x0000FF);
  LoRa_SetFreq(LoRa_LFMsb, LoRa_LFMid, LoRa_LFLsb);
  //LoRa_Write(LoRa_RegFrMsb, LoRa_LFMsb);
  //LoRa_Write(LoRa_RegFrMid, LoRa_LFMid);
  //LoRa_Write(LoRa_RegFrLsb, LoRa_LFLsb);
}


float LoRa_GetFreq()
{
  //get the current set LoRa frequency
  //Serial.print("LoRa_GetFreq() ");
  //Serial.println();
  byte LoRa_LFMsb, LoRa_LFMid, LoRa_LFLsb;
  unsigned long LoRa_Ltemp;
  float LoRa_Ltemp1;
  LoRa_LFMsb = LoRa_Read(LoRa_RegFrMsb);
  Serial.println(LoRa_LFMsb);
  LoRa_LFMid = LoRa_Read(LoRa_RegFrMid);
  Serial.println(LoRa_LFMid);
  LoRa_LFLsb = LoRa_Read(LoRa_RegFrLsb);
  Serial.println(LoRa_LFLsb);
  LoRa_Ltemp = ((LoRa_LFMsb * 0x10000ul) + (LoRa_LFMid * 0x100ul) + LoRa_LFLsb);
  LoRa_Ltemp1 = ((LoRa_Ltemp * 61.03515625) / 1000000ul);
  return LoRa_Ltemp1;
}


void LoRa_Setup()
{
  //initialize LoRa device registers
  //Serial.print("LoRa_Setup()");
  //Serial.println();
  LoRa_ResetDev();								// Clear all registers to default
  LoRa_Write(LoRa_RegOpMode, 0x08);				// RegOpMode, need to set to sleep mode before configure for LoRa mode
  LoRa_Write(LoRa_RegOcp, 0x0B);					// RegOcp
  LoRa_Write(LoRa_RegLna, 0x23);					// RegLna
  LoRa_Write(LoRa_RegSymbTimeoutLsb, 0xFF);		// RegSymbTimeoutLsb
  LoRa_Write(LoRa_RegPreambleLsb, 0x0C);			// RegPreambleLsb, default
  LoRa_Write(LoRa_RegFdevLsb, LoRa_Deviation);	// LSB of deviation, 5kHz
}


void LoRa_TXONDirect(byte LoRa_LTXPower)
{
  //turns on transmitter,in direct mode for FSK and audio  power level is from 2 to 17
//  Serial.print("LoRa_TXONDirect() Pwr ");
//  Serial.print(LoRa_LTXPower);
//  Serial.println("dBm");
  digitalWrite(LoRa_Active, HIGH);
  byte LoRa_Lvar1;
  LoRa_Lvar1 = LoRa_LTXPower + 0xEE;				// has effect of converting 17 into 15
  LoRa_Write(LoRa_RegPaConfig, LoRa_Lvar1);		// set TX power
  LoRa_Write(LoRa_RegOpMode, 0x0B);				// TX on direct mode, low frequency mode
}

void LoRa_TXOFF()
{
  //turns off transmitter
  //Serial.print("LoRa_TXOFF()");
  //Serial.println();
  LoRa_Write(LoRa_RegOpMode, 0x08);					// TX and RX to sleep, in direct mode
  digitalWrite(LoRa_Active, LOW);
}


void LoRa_DirectSetup()
{
  //setup LoRa device for direct modulation mode
  //Serial.print("LoRa_DirectSetup()");
  //Serial.println();
  LoRa_Write(LoRa_RegOpMode, 0x08);
  LoRa_Write(LoRa_RegPacketConfig2, 0x00);			// set continuous mode
}


void LoRa_Tone(int LoRa_LFreq, int LoRa_LToneLen, int LoRa_LTXPower )
{
  //Transmit an FM tone
//  Serial.print("LoRa_Tone() ");
//  Serial.print(LoRa_LFreq);
//  Serial.print(" ");
//  Serial.print(LoRa_LToneLen);
//  Serial.print(" ");
//  Serial.print(LoRa_LTXPower);
//  Serial.print(" ");
  //  Serial.println();
  LoRa_DirectSetup();
  LoRa_TXONDirect(LoRa_LTXPower);						// TXON, power 10
  LoRa_Write(LoRa_RegFdevLsb, LoRa_Deviation);		// We are generating a tone so set the deviation, 5kHz
  tone(LoRa_PWM, LoRa_LFreq);
  delay(LoRa_LToneLen);
  LoRa_TXOFF();
}


void LoRa_SetModem(byte LoRa_LBW, byte LoRa_LSF, byte LoRa_LCR, byte LoRa_LHDR, byte LoRa_LLDROPT)
{
  //setup the LoRa modem parameters
  //Serial.print("LoRa_SetModem()");
  //Serial.println();
  byte LoRa_Lvar1, LoRa_Lvar2;
  LoRa_Lvar1 = LoRa_LBW + LoRa_LCR + LoRa_LHDR;			// calculate value of RegModemConfig1
  LoRa_Lvar2 = LoRa_LSF * 16 + 7;							// calculate value of RegModemConfig2, ($07; Header indicates CRC on, RX Time-Out MSB = 11
  LoRa_Write(LoRa_RegOpMode, 0x08);						// RegOpMode, need to set to sleep mode before configure for LoRa mode
  LoRa_Write(LoRa_RegOpMode, 0x88);						// goto LoRa mode
  LoRa_Write(LoRa_RegModemConfig1, LoRa_Lvar1);
  LoRa_Write(LoRa_RegModemConfig2, LoRa_Lvar2);
  LoRa_Write(LoRa_RegModemConfig3, LoRa_LLDROPT);
}


void LoRa_PrintModem()
{
  //Print the LoRa modem parameters
  Serial.print("LoRa_PrintModem() ");
  Serial.print(LoRa_Read(LoRa_RegModemConfig1));
  Serial.print(" ");
  Serial.print(LoRa_Read(LoRa_RegModemConfig2));
  Serial.print(" ");
  Serial.println(LoRa_Read(LoRa_RegModemConfig3));
}


void LoRa_TXONLoRa(byte LoRa_LTXPower)
{
  //turns on LoRa transmitter, Sends packet, power level is from 2 to 17
  Serial.print("LoRa_TXONLoRa() Pwr ");
  Serial.print(LoRa_LTXPower);
  Serial.println("dBm");
  byte LoRa_Lvar1;
  LoRa_Lvar1 = LoRa_LTXPower + 0xEE;						// has effect of converting 17 into 15
  LoRa_Write(LoRa_RegPaConfig, LoRa_Lvar1);				// set TX power
  LoRa_Write(LoRa_RegOpMode, 0x8B);						// TX on direct mode, low frequency mode
  digitalWrite(LoRa_Active, HIGH);
}


void LoRa_Notes(int type)
{
  // plays a series of notes to monitor Tx health before takeoff
//  Serial.print("LoRa Sound: ");
//  Serial.println(type);
  switch(type)
  {
    case 0:   // init sound. Beethoven's 5th
      LoRa_Tone(392, 200, 10);
      delay(50);
      LoRa_Tone(392, 200, 10);  
      delay(50);
      LoRa_Tone(392, 200, 10);
      delay(50);
      LoRa_Tone(311, 750, 10);
      break;
    case 1:  // failure sound. Sad Trombone
      LoRa_Tone(147, 200, 10);
      LoRa_Tone(145, 50, 10);
      delay(50);
      LoRa_Tone(139, 200, 10);  
      LoRa_Tone(137, 50, 10);
      delay(50);
      LoRa_Tone(131, 200, 10);
      LoRa_Tone(129, 50, 10);
      delay(50);
      LoRa_Tone(123, 350, 10);
      LoRa_Tone(121, 500, 10);
      break;
    case 2:  // GPS fix. Close encounters of the third kind
      LoRa_Tone(392, 300, 10);
      delay(50);
      LoRa_Tone(440, 300, 10);  
      delay(50);
      LoRa_Tone(349, 300, 10);
      delay(50);
      LoRa_Tone(175, 300, 10);
      delay(50);
      LoRa_Tone(262, 500, 10);
      break;
    case 3:  // Beethoven's Ode to joy EEFG GFED CCDE EDD
      LoRa_Tone(659, 150, 10);
      delay(50);
      LoRa_Tone(659, 150, 10);
      delay(50);
      LoRa_Tone(698, 150, 10);
      delay(50);
      LoRa_Tone(784, 150, 10);
      delay(50);
      LoRa_Tone(784, 150, 10);
      delay(50);
      LoRa_Tone(698, 150, 10);
      delay(50);
      LoRa_Tone(659, 150, 10);
      delay(50);
      LoRa_Tone(587, 150, 10);
      delay(50);
      LoRa_Tone(523, 150, 10);
      delay(50);
      LoRa_Tone(523, 150, 10);
      delay(50);
      LoRa_Tone(587, 150, 10);
      delay(50);
      LoRa_Tone(659, 150, 10);
      delay(50);
      LoRa_Tone(659, 220, 10);  
      delay(50);
      LoRa_Tone(587, 75, 10);
      delay(50);
      LoRa_Tone(587, 500, 10);
      break;    
  }  
}

boolean LoRa_SendStr(unsigned char LoRa_Array[], byte LoRa_PacketLength, byte LoRa_LTXPacketType, long LoRa_LTXTimeout, byte LoRa_LTXPower)
{
  //fills FIFO with one header byte (packet type), then sends LoRa_PacketLength bytes from LoRa_TXBUFF(128)
  //Serial.print("LoRa_Send() ");
  //Serial.print("TX ");
  //Serial.println(LoRa_LTXPacketType);
  byte LoRa_LRegData;

  LoRa_Write(LoRa_RegOpMode, 0x09);
  LoRa_Write(LoRa_RegIrqFlags, 0xFF);
  LoRa_Write(LoRa_RegIrqFlagsMask, 0xF7);
  LoRa_Write(LoRa_RegFifoTxBaseAddr, 0x00);
  LoRa_Write(LoRa_RegFifoAddrPtr, 0x00);      // start burst write

  digitalWrite(LoRa_NSS, LOW);          // Set NSS low
  SPI.transfer(LoRa_WRegFifo);          // address for burst write
  SPI.transfer(LoRa_LTXPacketType);                     // Write the packet type

  for (int i = 0;  i < LoRa_PacketLength; i++)
  {
    SPI.transfer(LoRa_Array[i]);
  }

  digitalWrite(LoRa_NSS, HIGH);         // finish the burst write

  LoRa_Write(LoRa_RegPayloadLength, LoRa_PacketLength + 1);   // packet length = message plus 1 byte header (packet type)

  LoRa_LTXTimeout = LoRa_LTXTimeout * 945;      // convert seconds to mS, delay in TX done loop is 1ms
  LoRa_TXONLoRa(LoRa_LTXPower);

  do
  {
    delay(1);
    LoRa_LTXTimeout--;
    LoRa_LRegData = LoRa_Read(LoRa_RegIrqFlags);
  }
  while (LoRa_LTXTimeout > 0 && LoRa_LRegData == 0) ;   // use a timeout counter, just in case the TX sent flag is missed

  LoRa_TXOFF();

  if (LoRa_LTXTimeout == 0)
  {
    Serial.print("ERROR,TXtimeout");
    Serial.println();
    return false;
  }
  else
  {
    return true;
  }
}

/*
void LoRa_Init()
{
  LoRa_ResetDev();      // Reset LoRa
  LoRa_Setup();       // Initialize LoRa
  LoRa_SetFreqF(LoRa_Freq); // Set frequancey
}
*/
