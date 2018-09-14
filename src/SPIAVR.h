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

#ifndef _SPIAVR_H_INCLUDED
#define _SPIAVR_H_INCLUDED

#include <Arduino.h>

// Run SPI in either slave or master mode.

// The SPI peripheral can run as SPI master or SPI slave
// or it can be turned off.
//

#define CLOCK_PPOLARITY_LEADING_EDGE_FALLING_DEFAULT false
#define CLOCK_PHASE_LEADING_EDGE_SETUP_DEFAULT false
#define CLOCK_RATE_DEFAULT 0
#define DOUBLE_SPEED_DEFAULT 0
#define DATA_ORDER_MSB_FIRST_DEFAULT false



class SPIAVRClass
{
protected:

	// Flags used to construct control bits for Master
	uint8_t _clockRateSelect;
	uint8_t _doubleSpeed;
	bool _clockPolarityLeadingEdgeFalling;
	bool _clockPhaseLeadingEdgeSetup;
	bool _dataOrderMSBFirst;
	
	int8_t _errorCode;
	typedef volatile uint8_t registerT;
	
	registerT   *_spcr;
	registerT   *_spdr;
	registerT   *_spsr;

	uint8_t _ss;
	uint8_t _miso;
	uint8_t _mosi;
	uint8_t _sck;
	uint8_t *_inputBuffer;
	uint8_t *_outputBuffer;
	uint8_t isAnError[];
	uint16_t _inputBufferPosition;
	uint16_t _outputBufferPosition;
	uint16_t _messageLength;
	bool _isMaster;
	bool _isSlave;
	bool _transactionIsActive;
	bool _dataIsReady;

public:
	enum errorCodeT { NO_ERROR, TRANSACTION_ACTIVE_ERROR };
	errorCodeT errorCode;


	SPIAVRClass ( registerT * spcr, registerT * spdr, registerT * spsr, 
		uint8_t miso, uint8_t mosi, uint8_t sck, uint8_t ss )
	{
		_spcr = spcr;
		_spdr = spdr;
		_spsr = spsr;
		_miso = miso;
		_mosi = mosi;
		_sck = sck;
		_ss = ss;
		
		_clockRateSelect = CLOCK_RATE_DEFAULT;
		_doubleSpeed = DOUBLE_SPEED_DEFAULT;
		_clockPolarityLeadingEdgeFalling = CLOCK_PPOLARITY_LEADING_EDGE_FALLING_DEFAULT;
		_clockPhaseLeadingEdgeSetup = CLOCK_PHASE_LEADING_EDGE_SETUP_DEFAULT;
		_dataOrderMSBFirst = DATA_ORDER_MSB_FIRST_DEFAULT;
		
	}
		 
	void setSpeedBitsForClock(uint8_t clockCeiling);

	void setMode( bool clockPolarityLeadingEdgeFalling, bool clockPhaseLeadingEdgeSetup,
		bool dataOrderMSBFirst );
		
	void beginMasterTransaction ( uint8_t slaveSelect, uint8_t *inputBuffer, 
		uint8_t outputBuffer[], uint16_t messageLength );
	void beginSlaveListening ( uint8_t *inputBuffer, uint8_t *outputBuffer, 
		uint16_t messageLength  );

	bool isSlave() {return _isSlave; };
	bool isMaster() {return _isMaster; };
	bool transactionIsActive() {return _transactionIsActive; };
	bool dataIsReady() {return _dataIsReady; };
	void isr();
	void reset();
	
};
extern SPIAVRClass SPIAVR;

#endif // _SPIAVR_H_INCLUDED
