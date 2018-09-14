// SPIAVR.cpp
/*
 * Copyright (c) 2018 by Danny Swarzman <danny@stowlake.com>
 *
 * SPI library for AVR models of Arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "SPIAVR.h"
//
// Three bits are used to specify speed for the SPI clock in this class
// They represent dividers of the processor clock rate, F_CPU.
//
// 	SPR1 SPR0 ~SPI2X Freq
//   0    0     0   F_CPU/2
//   0    0     1   F_CPU/4
//   0    1     0   F_CPU/8
//   0    1     1   F_CPU/16
//   1    0     0   F_CPU/32
//   1    0     1   F_CPU/64
//   1    1     0   F_CPU/64
//   1    1     1   F_CPU/128
//


void SPIAVRClass::setSpeedBitsForClock(uint8_t clockCeiling)
{
	uint8_t speedBits;
	for (speedBits=0; speedBits<7 && F_CPU>>(speedBits+1) < clockCeiling; speedBits++ )	
		; // Empty loop
	if ( speedBits == 0b110 )
		speedBits = 0b111;
	_clockRateSelect = speedBits >> 1;
	_doubleSpeed = speedBits&1;
}

// 
void SPIAVRClass::setMode( bool clockPolarityLeadingEdgeFalling, bool clockPhaseLeadingEdgeSetup,
	bool dataOrderMSBFirst )
{
	_clockPolarityLeadingEdgeFalling = clockPolarityLeadingEdgeFalling;
	_clockPhaseLeadingEdgeSetup = clockPhaseLeadingEdgeSetup;
	_dataOrderMSBFirst = dataOrderMSBFirst;
}

void SPIAVRClass::beginMasterTransaction ( uint8_t slaveSelect, uint8_t *inputBuffer, 
	uint8_t *outputBuffer, uint16_t messageLength )
{
	// Don't do anything if there is an transaction in progress or if data are available.
	if ( _transactionIsActive || _dataIsReady )
	{
		errorCode = TRANSACTION_ACTIVE_ERROR;
		return;
	}
	
	if ( _isSlave )
		reset();
	_ss = slaveSelect;	
	_transactionIsActive = false;
	_dataIsReady = false;
	_inputBuffer = inputBuffer;
	_outputBuffer = outputBuffer;
	_messageLength = messageLength;
	_isMaster = true;
	
	// In master mode, the hardware will make _mosi an input.
	// The program needs to set the mode for the outputs.	
	pinMode ( _sck, OUTPUT );
	pinMode ( _ss, OUTPUT );
	pinMode ( _mosi, OUTPUT );
	_outputBufferPosition = 0;
	_inputBufferPosition = 0;
		
	*_spcr = 1 << SPE | 1 << SPIE | 1 << MSTR;
	*_spsr |= _doubleSpeed ? _doubleSpeed : 0;		
  digitalWrite ( _ss, LOW );
	*_spdr = *(_outputBuffer + _outputBufferPosition++);
	
}

void SPIAVRClass::beginSlaveListening ( uint8_t *inputBuffer, uint8_t *outputBuffer, 
	uint16_t messageLength )
{
	if ( _transactionIsActive || _dataIsReady )
	{
		_errorCode = TRANSACTION_ACTIVE_ERROR;
		return;
	}
	
  if ( _isSlave )
		reset();
	  _outputBuffer = outputBuffer;
	  _inputBuffer = inputBuffer;
    _inputBufferPosition = 0;
    _outputBufferPosition = 0;
	  _messageLength = messageLength;	
	  _transactionIsActive = false;
	  _dataIsReady = false;
	  _isSlave = true;

	// In slave mode, the hardware will make all the lines inputs, except mosi	
	pinMode ( _miso, OUTPUT );
	*_spcr = 1 << SPE | 1 << SPIE;
	*_spdr = *(_outputBuffer + _outputBufferPosition++);	
}

void SPIAVRClass::reset()
{
	_isSlave = false;
	_isMaster = false;
	_transactionIsActive = false;
	_dataIsReady = false;
	*_spcr = 0;
	pinMode ( _ss, INPUT );
	pinMode ( _mosi, INPUT );
	pinMode ( _miso, INPUT );
	pinMode ( _sck, INPUT );		
}

void SPIAVRClass::isr()
{
	_transactionIsActive = true;
	uint8_t clu = *_spdr;
  	if ( _inputBufferPosition < _messageLength )
		*(_inputBuffer + _inputBufferPosition++)=clu;
	if ( _outputBufferPosition < _messageLength ) 
		*_spdr = *(_outputBuffer + _outputBufferPosition++);
	else
		_dataIsReady = true;
}

