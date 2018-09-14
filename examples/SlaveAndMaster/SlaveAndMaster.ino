// SPIAVRExample.ino
#include  "SPIAVR.h"

// Each SPI transaction is an exchange.
// BUFFER_SIZE bytes are sent. BUFFER_SIZE bytes are received.

// There are two user commands via the Serial port:

// S means enter slave mode and wait for an exchange.
// M means enter master mode and start an exchange.

// During a transaction, the master sends a series of bytes
// and the slave sends exactly the same number of bytes at the same time.
//
// The slave must have the data ready to sent while it waits for
// the master to start the transaction.
//
// After one transaction, the slave will shut down until it is restarted.
//
// This sketch sends a series "MMMMM..." in master mode and "SSSSS..." as slave
//
#define BUFFER_SIZE 8

uint8_t inputBuffer[BUFFER_SIZE];
uint8_t outputBuffer[BUFFER_SIZE];

#define BAUDRATE 38400

SPIAVRClass SPIAVR(&SPCR,&SPDR,&SPSR,MISO,MOSI,SCK,SS);
ISR(SPI_STC_vect) {SPIAVR.isr();};

void setup()
{
	Serial.begin(BAUDRATE);
	Serial.println(F("SPAVRExample"));
	
	showMenu();
}

void loop()
{
	if ( Serial.available() )
	{
		uint8_t clu = Serial.read();
		switch (clu)
		{
			case 'S':startSlave();
				break;
			case 'M':startMaster();
				break;
		}
		showMenu();
	}
	
	if ( SPIAVR.dataIsReady() )
	{
		report();
		SPIAVR.reset();
		showMenu();
	}
}

void showMenu()
{
	Serial.println(F("S to start slave. M to start master"));
}

void startSlave()
{
  Serial.println (F("Slave"));
	clearBuffers('S');
	SPIAVR.beginSlaveListening ( inputBuffer,  outputBuffer, BUFFER_SIZE );
}

void startMaster()
{
  Serial.println (F("Master"));
	clearBuffers('M');
	SPIAVR.beginMasterTransaction ( SS, inputBuffer, outputBuffer, BUFFER_SIZE );
}



void clearBuffers(char letter)
{  
  for (uint8_t iByte = 0; iByte<BUFFER_SIZE; iByte++)
  {
    inputBuffer[iByte] = 'X';
    outputBuffer[iByte] = iByte+'0';
  }
  outputBuffer[0] = letter;
  outputBuffer[BUFFER_SIZE-1] = letter;
 }

void report()
{
  String preface = "Received:\"";
  String contents = String ( "" );
  String hexPart = String("");
  for ( int iChar=0; iChar<BUFFER_SIZE; iChar++ )
  {
    uint8_t value = inputBuffer[iChar];
    hexPart += String (value / 0x10, HEX) + String (value % 0x10, HEX) + String(" ");
    contents += value < 0x20 ? String ( '?' ) : String ( char(value) );
  }
  Serial.println ( preface + contents + "\"" + hexPart );
}
