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
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_32K;
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(i2sHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspInit 0 */

  /* USER CODE END SPI3_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_PLLI2S|RCC_PERIPHCLK_I2S_APB1;
    PeriphClkInitStruct.PLLI2S.PLLI2SN = 50;
    PeriphClkInitStruct.PLLI2S.PLLI2SM = 4;
    PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
    PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
    PeriphClkInitStruct.PLLI2SSelection = RCC_PLLI2SCLKSOURCE_PLLSRC;
    PeriphClkInitStruct.I2sApb1ClockSelection = RCC_I2SAPB1CLKSOURCE_PLLI2S;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

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

  /* USER CODE BEGIN SPI3_MspDeInit 1 */

  /* USER CODE END SPI3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
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
