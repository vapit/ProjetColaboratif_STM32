/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2s.c
  * @brief   This file provides code for the configuration
  *          of the I2S instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2s.h"

/* USER CODE BEGIN 0 */
#include "string.h"

#define BUFFER_SIZE 1024
uint16_t rxBuffer[BUFFER_SIZE];
uint16_t txBuffer[BUFFER_SIZE];


/* USER CODE END 0 */

I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi3_tx;

/* I2S3 init function */
void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */

  /* USER CODE END I2S3_Init 0 */

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_16K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_ENABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

}

void HAL_I2S_MspInit(I2S_HandleTypeDef* i2sHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2sHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspInit 0 */

  /* USER CODE END SPI3_MspInit 0 */
    /* I2S3 clock enable */
    __HAL_RCC_SPI3_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**I2S3 GPIO Configuration
    PA4     ------> I2S3_WS
    PB12     ------> I2S3_CK
    PC7     ------> I2S3_MCK
    PB4     ------> I2S3_ext_SD
    PB5     ------> I2S3_SD
    */
    GPIO_InitStruct.Pin = CODEC_I2S3_WS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(CODEC_I2S3_WS_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CODEC_I2S3_SCK_Pin|CODEC_I2S3ext_SD_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_SPI3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CODEC_I2S3_MCK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(CODEC_I2S3_MCK_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CODEC_I2S3_SD_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(CODEC_I2S3_SD_GPIO_Port, &GPIO_InitStruct);

    /* I2S3 DMA Init */
    /* SPI3_TX Init */
    hdma_spi3_tx.Instance = DMA1_Stream5;
    hdma_spi3_tx.Init.Channel = DMA_CHANNEL_0;
    hdma_spi3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_spi3_tx.Init.Mode = DMA_CIRCULAR;
    hdma_spi3_tx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_spi3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_spi3_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(i2sHandle,hdmatx,hdma_spi3_tx);

    /* I2S3 interrupt Init */
    HAL_NVIC_SetPriority(SPI3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SPI3_IRQn);
  /* USER CODE BEGIN SPI3_MspInit 1 */

  /* USER CODE END SPI3_MspInit 1 */
  }
}

void HAL_I2S_MspDeInit(I2S_HandleTypeDef* i2sHandle)
{

  if(i2sHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspDeInit 0 */

  /* USER CODE END SPI3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI3_CLK_DISABLE();

    /**I2S3 GPIO Configuration
    PA4     ------> I2S3_WS
    PB12     ------> I2S3_CK
    PC7     ------> I2S3_MCK
    PB4     ------> I2S3_ext_SD
    PB5     ------> I2S3_SD
    */
    HAL_GPIO_DeInit(CODEC_I2S3_WS_GPIO_Port, CODEC_I2S3_WS_Pin);

    HAL_GPIO_DeInit(GPIOB, CODEC_I2S3_SCK_Pin|CODEC_I2S3ext_SD_Pin|CODEC_I2S3_SD_Pin);

    HAL_GPIO_DeInit(CODEC_I2S3_MCK_GPIO_Port, CODEC_I2S3_MCK_Pin);

    /* I2S3 DMA DeInit */
    HAL_DMA_DeInit(i2sHandle->hdmatx);

    /* I2S3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SPI3_IRQn);
  /* USER CODE BEGIN SPI3_MspDeInit 1 */

  /* USER CODE END SPI3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void I2S_init(void){
  
  //StartAudioProcessing(); not working for now

}
void I2S_periodic(void){

}
void StartAudioProcessing() {
    // Démarrer la réception
    if (HAL_I2S_Receive_DMA(&hi2s3, rxBuffer, BUFFER_SIZE) != HAL_OK) {
        Error_Handler();
    }

    // Démarrer la transmission
    if (HAL_I2S_Transmit_DMA(&hi2s3, txBuffer, BUFFER_SIZE) != HAL_OK) {
        Error_Handler();
    }
}
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
    // Copier ou traiter la première moitié des données
    memcpy(txBuffer, rxBuffer, BUFFER_SIZE / 2 * sizeof(uint16_t));
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
    // Copier ou traiter la deuxième moitié des données
    memcpy(&txBuffer[BUFFER_SIZE / 2], &rxBuffer[BUFFER_SIZE / 2], BUFFER_SIZE / 2 * sizeof(uint16_t));
}
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s) {
    // Réagir en cas d'erreur (redémarrer DMA, enregistrer une erreur, etc.)
    Error_Handler();
}

/* USER CODE END 1 */
