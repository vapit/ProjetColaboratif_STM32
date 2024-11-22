/* USER CODE BEGIN Header */
/**
 *******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 *******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 *******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dfsdm.h"
#include "dma.h"
#include "i2c.h"
#include "i2s.h"
#include "quadspi.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app.h"
#include <stdio.h>
#include "string.h"
#include "../../../Drivers/BSP/STM32412G-Discovery/stm32412g_discovery.h"
#include "../../../Drivers/BSP/STM32412G-Discovery/stm32412g_discovery_lcd.h"
//#include "stm32412g_discovery_io.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
  uint16_t x;             // Position X
  uint16_t y;             // Position Y
  uint16_t width;         // Largeur
  uint16_t height;        // Hauteur
  uint32_t color;         // Couleur par défaut
  uint32_t selectedColor; // Couleur en surbrillance
  const char *label;      // Texte du bouton
} Button;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define SaturaLH(N, L, H) (((N) < (L)) ? (L) : (((N) > (H)) ? (H) : (N)))
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern DFSDM_Filter_HandleTypeDef hdfsdm1_filter0;
extern DFSDM_Filter_HandleTypeDef hdfsdm1_filter1;
extern DFSDM_Channel_HandleTypeDef hdfsdm1_channel0;
extern DFSDM_Channel_HandleTypeDef hdfsdm1_channel3;
DMA_HandleTypeDef hLeftDma;
DMA_HandleTypeDef hRightDma;
I2S_HandleTypeDef haudio_i2s;
DMA_HandleTypeDef hdma_i2s_tx;
AUDIO_DrvTypeDef *audio_drv;
int32_t LeftRecBuff[2048];
int32_t RightRecBuff[2048];
int16_t PlayBuff[4096];
uint32_t DmaLeftRecHalfBuffCplt = 0;
uint32_t DmaLeftRecBuffCplt = 0;
uint32_t DmaRightRecHalfBuffCplt = 0;
uint32_t DmaRightRecBuffCplt = 0;
uint32_t PlaybackStarted = 0;
uint32_t i = 0;
Button buttons[4];                  // Tableau pour les 4 boutons
uint8_t selectedButton = 0;         // Indice du bouton sélectionné
uint8_t previousSelectedButton = 0; // Variable pour suivre l'état précédent de la sélection
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */
static void Playback_Init(void);
void InitializeButtons();
void HighlightButton(uint8_t index);
void HandleJoystick();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief Initialisation des boutons à l'écran LCD
 */
void InitializeButtons()
{
  // Définir les positions, dimensions, couleurs et labels des boutons
  buttons[0] = (Button){20, 60, 80, 40, LCD_COLOR_BLUE, LCD_COLOR_RED, "Effet 1"};
  buttons[1] = (Button){140, 60, 80, 40, LCD_COLOR_BLUE, LCD_COLOR_RED, "Effet 2"};
  buttons[2] = (Button){20, 140, 80, 40, LCD_COLOR_BLUE, LCD_COLOR_RED, "Effet 3"};
  buttons[3] = (Button){140, 140, 80, 40, LCD_COLOR_BLUE, LCD_COLOR_RED, "Effet 4"};

  // Dessiner les boutons et afficher leurs labels
  for (uint8_t i = 0; i < 4; i++)
  {
    BSP_LCD_SetTextColor(buttons[i].color);
    BSP_LCD_FillRect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height);

    // Afficher le label centré sur le bouton
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);  // Couleur du texte
    BSP_LCD_SetBackColor(buttons[i].color); // Fond du texte
    BSP_LCD_DisplayStringAt(
        buttons[i].x + (buttons[i].width / 2) - (strlen(buttons[i].label) * 4), // Ajuster le texte à la taille
        buttons[i].y + (buttons[i].height / 2) - 8,                             // Centrer verticalement
        (uint8_t *)buttons[i].label,
        LEFT_MODE);
  }
  previousSelectedButton = selectedButton; // Initialiser l'état précédent
  HighlightButton(0);                      // Mettre le premier bouton en surbrillance
}

/**
 * @brief Met en surbrillance un bouton spécifique
 * @param index L'indice du bouton à surligner
 */
void HighlightButton(uint8_t index)
{
  // Si le bouton sélectionné n'a pas changé, ne pas redessiner
  if (index == previousSelectedButton)
  {
    return;
  }

  // Dessiner les boutons sans leur label
  for (uint8_t i = 0; i < 4; i++)
  {
    if (i == index)
    {
      BSP_LCD_SetTextColor(buttons[i].selectedColor); // Choisir la couleur de surbrillance
    }
    else
    {
      BSP_LCD_SetTextColor(buttons[i].color); // Couleur par défaut
    }
    BSP_LCD_FillRect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height);
  }

  // Réafficher les labels (en maintenant la couleur du texte)
  for (uint8_t i = 0; i < 4; i++)
  {
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);                                          // Couleur du texte
    BSP_LCD_SetBackColor(i == index ? buttons[i].selectedColor : buttons[i].color); // Fond du texte
    BSP_LCD_DisplayStringAt(
        buttons[i].x + (buttons[i].width / 2) - (strlen(buttons[i].label) * 4), // Ajuster le texte à la taille
        buttons[i].y + (buttons[i].height / 2) - 8,                             // Centrer verticalement
        (uint8_t *)buttons[i].label,
        LEFT_MODE);
  }

  // Mettre à jour l'état précédent
  previousSelectedButton = index;
}

/**
 * @brief Réinitialise l'état de tous les boutons (ils retournent à leur couleur bleue sauf celui activé)
 */
void ResetButtons() {
    for (uint8_t i = 0; i < 4; i++) {
        if (i == selectedButton) {
            // Le bouton sélectionné devient vert
            buttons[i].color = LCD_COLOR_GREEN;
            buttons[i].selectedColor = LCD_COLOR_GREEN;
        } else {
            // Les autres boutons retournent à leur couleur initiale bleue
            buttons[i].color = LCD_COLOR_BLUE;
            buttons[i].selectedColor = LCD_COLOR_RED;
        }
    }
    HighlightButton(selectedButton); // Mettre à jour la surbrillance après modification
}

/**
 * @brief Gère les interactions avec le joystick
 */
void HandleJoystick()
{
  JOYState_TypeDef joystickState = BSP_JOY_GetState();

  switch (joystickState)
  {
  case JOY_UP:
    if (selectedButton > 1)
      selectedButton -= 2; // Bouton au-dessus
    break;
  case JOY_DOWN:
    if (selectedButton < 2)
      selectedButton += 2; // Bouton en dessous
    break;
  case JOY_LEFT:
    if (selectedButton % 2 == 1)
      selectedButton--; // Bouton à gauche
    break;
  case JOY_RIGHT:
    if (selectedButton % 2 == 0)
      selectedButton++; // Bouton à droite
    break;
  case JOY_SEL:
    // Changer la couleur du bouton sélectionné
    buttons[selectedButton].color = LCD_COLOR_GREEN;
    break;
  default:
    break;
  }

  HighlightButton(selectedButton); // Mettre à jour la surbrillance si nécessaire
  HAL_Delay(200);                  // Anti-rebond
}

/* USER CODE END 0 */



/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FSMC_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2S3_Init();
  MX_QUADSPI_Init();
  MX_USART2_UART_Init();
  MX_DFSDM1_Init();
  BSP_LCD_Init();
  BSP_LCD_Clear(LCD_COLOR_WHITE);
  BSP_JOY_Init(JOY_MODE_GPIO);

  /* USER CODE BEGIN 2 */
  InitializeButtons();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  audioReceptionInit();
  UART_init();
  I2C_init();
  I2S_init();
  app_init();
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    UART_periodic();
    I2C_periodic();
    I2S_periodic();
    app_periodic();
    storingAudioIntoBuffer();
    HandleJoystick();
  }
}

/* USER CODE END 3 */

/**
 * @brief System Clock Configuration
 * @retval None
 *
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Macro to configure the PLL multiplication factor
   */
  __HAL_RCC_DFSDM1AUDIO_CONFIG(RCC_DFSDM1AUDIOCLKSOURCE_I2SAPB1);

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
}

/**
 * @brief Peripherals Common Clock Configuration
 * @retval None
 */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
   */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_PLLI2S | RCC_PERIPHCLK_I2S_APB1 | RCC_PERIPHCLK_DFSDM1 | RCC_PERIPHCLK_DFSDM1_AUDIO;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 344;
  PeriphClkInitStruct.PLLI2S.PLLI2SM = 8;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 7;
  PeriphClkInitStruct.PLLI2S.PLLI2SQ = 7;
  PeriphClkInitStruct.Dfsdm1ClockSelection = RCC_DFSDM1CLKSOURCE_APB2;
  PeriphClkInitStruct.Dfsdm1AudioClockSelection = RCC_DFSDM1AUDIOCLKSOURCE_I2SAPB1;
  PeriphClkInitStruct.PLLI2SSelection = RCC_PLLI2SCLKSOURCE_PLLSRC;
  PeriphClkInitStruct.I2sApb1ClockSelection = RCC_I2SAPB1CLKSOURCE_PLLI2S;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}

// new function here
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  if (hdfsdm_filter == &hdfsdm1_filter0)
  {
    DmaLeftRecHalfBuffCplt = 1;
  }
  else
  {
    DmaRightRecHalfBuffCplt = 1;
  }
}
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  if (hdfsdm_filter == &hdfsdm1_filter1)
  {
    DmaLeftRecBuffCplt = 1;
  }
  else
  {
    DmaRightRecBuffCplt = 1;
  }
}
static void Playback_Init(void)
{
  uint16_t buffer_fake[16] = {0x00};

  /* Initialize I2S */
  __HAL_I2S_RESET_HANDLE_STATE(&haudio_i2s);

  /* Configure the I2S peripheral */
  haudio_i2s.Instance = SPI3;
  /* Disable I2S block */
  __HAL_I2S_DISABLE(&haudio_i2s);

  /* I2S peripheral configuration */
  haudio_i2s.Init.AudioFreq = I2S_AUDIOFREQ_16K;
  haudio_i2s.Init.ClockSource = I2S_CLOCK_PLL;
  haudio_i2s.Init.CPOL = I2S_CPOL_LOW;
  haudio_i2s.Init.DataFormat = I2S_DATAFORMAT_16B;
  haudio_i2s.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  haudio_i2s.Init.Mode = I2S_MODE_MASTER_TX;
  haudio_i2s.Init.Standard = I2S_STANDARD_PHILIPS;
  haudio_i2s.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;

  /* Init the I2S */
  HAL_I2S_Init(&haudio_i2s);

  /* Enable I2S block */
  __HAL_I2S_ENABLE(&haudio_i2s);

  /* I2C Init */
  AUDIO_IO_Init();

  /* Initialize audio driver */
  if (WM8994_ID != wm8994_drv.ReadID(AUDIO_I2C_ADDRESS))
  {
    Error_Handler();
  }

  audio_drv = &wm8994_drv;
  audio_drv->Reset(AUDIO_I2C_ADDRESS);
  /* Send fake I2S data in order to generate MCLK needed by WM8994 to set its registers
   * MCLK is generated only when a data stream is sent on I2S */
  HAL_I2S_Transmit_DMA(&haudio_i2s, buffer_fake, 16);

  if (0 != audio_drv->Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, 90, AUDIO_FREQUENCY_16K))
  {
    Error_Handler();
  }
  /* Stop sending fake I2S data */
  HAL_I2S_DMAStop(&haudio_i2s);
}

void audioReceptionInit(void)
{

  Playback_Init();
  BSP_LED_Init(LED3);
  /* Start DFSDM conversions */
  if (HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0, RightRecBuff, 2048))
  {
    Error_Handler();
  }
  if (HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter1, LeftRecBuff, 2048))
  {
    Error_Handler();
  }
}
void storingAudioIntoBuffer(void)
{

  if ((DmaLeftRecHalfBuffCplt == 1) && (DmaRightRecHalfBuffCplt == 1))
  {
    /* Store values on Play buff */
    for (i = 0; i < 1024; i++)
    {
      PlayBuff[2 * i] = SaturaLH((LeftRecBuff[i] >> 8), -32768, 32767);
      PlayBuff[(2 * i) + 1] = SaturaLH((RightRecBuff[i] >> 8), -32768, 32767);
    }
    if (PlaybackStarted == 0)
    {
      if (0 != audio_drv->Play(AUDIO_I2C_ADDRESS, (uint16_t *)&PlayBuff[0], 4096))
      {
        Error_Handler();
      }
      if (HAL_OK != HAL_I2S_Transmit_DMA(&haudio_i2s, (uint16_t *)&PlayBuff[0], 4096))
      {
        Error_Handler();
      }
      PlaybackStarted = 1;
    }
    DmaLeftRecHalfBuffCplt = 0;
    DmaRightRecHalfBuffCplt = 0;
  }
  if ((DmaLeftRecBuffCplt == 1) && (DmaRightRecBuffCplt == 1))
  {
    /* Store values on Play buff */
    for (i = 1024; i < 2048; i++)
    {
      PlayBuff[2 * i] = SaturaLH((LeftRecBuff[i] >> 8), -32768, 32767);
      PlayBuff[(2 * i) + 1] = SaturaLH((RightRecBuff[i] >> 8), -32768, 32767);
    }
    DmaLeftRecBuffCplt = 0;
    DmaRightRecBuffCplt = 0;
  }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
