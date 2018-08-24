/*
 * data_sender.c
 *
 *  Created on: Aug 24, 2018
 *      Author: DEFAULTUSER
 */

#include <inttypes.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "data_sender.h"

IC_SENDER_StateTypeDef sender_state;
IC_CTRL_DataFrameTypeDef data_frame[2];	// Double buffers for simple multithreading

void SendData(uint8_t* pData, SPI_HandleTypeDef *hspi)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_SPI_Transmit_DMA(hspi, pData, 4);
}

// Called from interrupt routine
uint8_t StartDataProcessingAsync(uint8_t* pFrame, uint8_t nBytesReceived)
{
	if (!CheckDataIntegrity(pFrame, nBytesReceived)) return -1;
	uint8_t frameToWriteIdx;
	if (data_frame[0].state == IN_PROGRESS)	// Two buffers can't be processed simultaneously, so check the first one
	{
		frameToWriteIdx = 1;
	}
	else
	{
		frameToWriteIdx = 0;
	}
	data_frame[frameToWriteIdx].nBytesReceived = nBytesReceived;
	memcpy(data_frame[frameToWriteIdx].buffer, pFrame, nBytesReceived);
	data_frame[frameToWriteIdx].state = RECEIVED;	// Set state to freshly written
	return 0;
}

void StartDataTransfer(uint8_t frameIdx, SPI_HandleTypeDef *hspi)
{
	data_frame[frameIdx].state = IN_PROGRESS;
	sender_state.pFrameState = &(data_frame[frameIdx].state);
	sender_state.pFrameData = &(data_frame[frameIdx].buffer[5]);	// TODO: add buffer offset
	sender_state.nFramesLeft = data_frame[frameIdx].buffer[4];
	SendData(sender_state.pFrameData, hspi);
}

// Called from DMA completion routine
void ContinueSendingData(SPI_HandleTypeDef *hspi)
{
	if (sender_state.nFramesLeft > 0)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		SendData(sender_state.pFrameData, hspi);
		sender_state.nFramesLeft -= 1;
		sender_state.pFrameData += 4;
		if (sender_state.nFramesLeft == 0)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
			*(sender_state.pFrameState) = DONE;	// Mark current buffer as done
		}
	}
}

// Called by timer
void ProcessDataAsync(SPI_HandleTypeDef *hspi)
{
	if (sender_state.nFramesLeft == 0)
	{
		if (data_frame[0].state == RECEIVED)
		{
			StartDataTransfer(0, hspi);
		}
		else if (data_frame[1].state == RECEIVED)
		{
			StartDataTransfer(1, hspi);
		}
	}
}
