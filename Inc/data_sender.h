/*
 * data_sender.h
 *
 *  Created on: Aug 24, 2018
 *      Author: DEFAULTUSER
 */

#ifndef DATA_SENDER_H_
#define DATA_SENDER_H_

enum ProcessingStateTypeDef
{
	DONE,
	RECEIVED,
	IN_PROGRESS
};

typedef struct
{
	uint8_t buffer[64];
	volatile uint8_t nBytesReceived;
	volatile uint8_t state;
} IC_CTRL_DataFrameTypeDef;

typedef struct
{
	volatile uint8_t nFramesLeft;
	uint8_t *pFrameData;
	volatile uint8_t *pFrameState;
} IC_SENDER_StateTypeDef;

static void SendData(uint8_t* pData, SPI_HandleTypeDef *hspi);
uint8_t StartDataProcessingAsync(uint8_t* pFrame, uint8_t nBytesReceived);

#endif /* DATA_SENDER_H_ */
