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
// #include "stm32412g_discovery_io.h"
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

JOYState_TypeDef lastJoystickState = JOY_NONE; // Dernier état du joystick
uint32_t lastDebounceTime = 0;                 // Horodatage du dernier changement
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define SaturaLH(N, L, H) (((N) < (L)) ? (L) : (((N) > (H)) ? (H) : (N)))
#define SIZE_ECHO 3
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern DFSDM_Filter_HandleTypeDef hdfsdm1_filter0;   // handle fitre passe bas DFSDM
extern DFSDM_Filter_HandleTypeDef hdfsdm1_filter1;   // handle fitre passe bas DFSDM
extern DFSDM_Channel_HandleTypeDef hdfsdm1_channel0; // handle channel passe bas DFSDM
extern DFSDM_Channel_HandleTypeDef hdfsdm1_channel3; // handle channel passe bas DFSDM
DMA_HandleTypeDef hLeftDma;                          // handle DMA géré logiciel
DMA_HandleTypeDef hRightDma;                         // handle DMA géré logiciel
I2S_HandleTypeDef haudio_i2s;                        // handle I2S géré logiciel
DMA_HandleTypeDef hdma_i2s_tx;                       // handle I2S géré logiciel
AUDIO_DrvTypeDef *audio_drv;
int32_t LeftRecBuff[2048];   // buffer d'entree gauche
int32_t RightRecBuff[2048];  // buffer d'entree droite
int16_t DelayBuff[20][4096]; // delayed buffer combining left and right
uint16_t idxarray = 0;
int16_t PlayBuff[4096];               // buffer de sortie
uint32_t DmaLeftRecHalfBuffCplt = 0;  // flag de buffer
uint32_t DmaLeftRecBuffCplt = 0;      // flag de buffer
uint32_t DmaRightRecHalfBuffCplt = 0; // flag de buffer
uint32_t DmaRightRecBuffCplt = 0;     // flag de buffer
uint32_t PlaybackStarted = 0;         // flag de buffer
Button buttons[4];                    // Tableau pour les 4 boutons
uint8_t selectedButton = 0;           // Indice du bouton sélectionné
uint8_t previousSelectedButton = 0;   // Variable pour suivre l'état précédent de la sélection
uint8_t EffetNB = 0;                  // numéro d'effet
uint16_t istart = 1024;               // début milieu de buffer
uint16_t istop = 2048;                // fin buffer
int16_t size_echo = 0;                // taille de l'écho

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */
static void Playback_Init(void);
void InitializeButtons();            // initialise bouton de l'écran
void HighlightButton(uint8_t index); // change couleur écran
void HandleJoystick();               // gère le bouton
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

  // Dessiner les boutons avec leurs labels
  for (uint8_t i = 0; i < 4; i++)
  {
    BSP_LCD_SetTextColor(buttons[i].color);
    BSP_LCD_FillRect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height);

    // Afficher le label centré sur chaque bouton
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

void HighlightButton(uint8_t index)
{
  // Réinitialiser tous les boutons à leur couleur par défaut
  for (uint8_t i = 0; i < 4; i++)
  {
    BSP_LCD_SetTextColor(i == index ? buttons[i].selectedColor : buttons[i].color);
    BSP_LCD_FillRect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height);

    // Réafficher les labels avec les couleurs mises à jour
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

void ResetButtons()
{
  // Met à jour les couleurs : sélectionné en vert, les autres en bleu
  for (uint8_t i = 0; i < 4; i++)
  {
    if (i == selectedButton)
    {
      buttons[i].color = LCD_COLOR_GREEN;
      buttons[i].selectedColor = LCD_COLOR_GREEN;
      EffetNB = i;
    }
    else
    {
      buttons[i].color = LCD_COLOR_BLUE;
      buttons[i].selectedColor = LCD_COLOR_RED;
    }
  }

  // Redessiner tous les boutons avec le bouton sélectionné en vert
  HighlightButton(selectedButton);
}

void HandleJoystick()
{
  JOYState_TypeDef joystickState = BSP_JOY_GetState();
  uint32_t currentTime = HAL_GetTick(); // Obtenir le temps système actuel

  if (currentTime > lastDebounceTime + DEBONCE_DELAY) // anti rebond pour empecher glitch lors du maintient
  {
    switch (joystickState)
    {
    case JOY_UP:
      if (selectedButton > 1)
        selectedButton -= 2; // Bouton au-dessus

      lastDebounceTime = currentTime;
      break;
    case JOY_DOWN:
      if (selectedButton < 2)
        selectedButton += 2; // Bouton en dessous

      lastDebounceTime = currentTime;
      break;
    case JOY_LEFT:
      if (selectedButton % 2 == 1)
        selectedButton--; // Bouton à gauche

      lastDebounceTime = currentTime;
      break;
    case JOY_RIGHT:
      if (selectedButton % 2 == 0)
        selectedButton++; // Bouton à droite

      lastDebounceTime = currentTime;
      break;
    case JOY_SEL:
      ResetButtons(); // Appliquer la sélection

      lastDebounceTime = currentTime;
      return; // Pas besoin de surbrillance supplémentaire

    default:
      return;
    }

    HighlightButton(selectedButton); // Mettre à jour la surbrillance
  }
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

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

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
  audioReceptionInit(); // init des fonctions audios
  UART_init();          // init des autres fichiers
  I2C_init();
  I2S_init();
  app_init();
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    UART_periodic(); // boucle des autres fichier
    I2C_periodic();
    I2S_periodic();
    app_periodic();
    // storingAudioIntoBuffer(); //retour sans delai
    HandleJoystick(); // gestion du bouton
    echobox(EffetNB); // création de l'echo
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

  /** Configure the main internal regulator output voltage
   */
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

  /** Initializes the CPU, AHB and APB buses clocks
   */
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
int __io_putchar(int ch) // gestion printf
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
    for (uint16_t i = 0; i < 1024; i++)
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

    for (uint16_t i = 1024; i < 2048; i++)
    {
      PlayBuff[2 * i] = SaturaLH((LeftRecBuff[i] >> 8), -32768, 32767);
      PlayBuff[(2 * i) + 1] = SaturaLH((RightRecBuff[i] >> 8), -32768, 32767);
    }

    DmaLeftRecBuffCplt = 0;
    DmaRightRecBuffCplt = 0;
  }
}
void proccessEcho(uint16_t idx_start, uint16_t idx_stop, int32_t *p_in_left, int32_t *p_in_right, int16_t *p_out, int16_t size_echo)
{
  uint16_t i = 0;                            // gestion de l'emplacement des l'echo
  int16_t idx_m1 = idxarray - size_echo;     // gestion de l'emplacement des l'echo
  int16_t idx_m2 = idxarray - size_echo * 2; // gestion de l'emplacement des l'echo
  int16_t idx_m3 = idxarray - size_echo * 3; // gestion de l'emplacement des l'echo
  if (idx_m1 < 0)                            // permettant d'empecher les index negatifs
  {
    idx_m1 += 20;
  }
  if (idx_m2 < 0)
  {
    idx_m2 += 20;
  }
  if (idx_m3 < 0)
  {
    idx_m3 += 20;
  }
  for (i = idx_start; i < idx_stop; i++) // for pour remplir les buffers (du start au stop)
  {
    DelayBuff[idxarray][2 * i] = SaturaLH((p_in_left[i] >> 8), -32768, 32767);
    DelayBuff[idxarray][(2 * i) + 1] = SaturaLH((p_in_right[i] >> 8), -32768, 32767);
    p_out[2 * i] = DelayBuff[idxarray][2 * i] + DelayBuff[idx_m1][2 * i] / 2 + DelayBuff[idx_m2][2 * i] / 3 + DelayBuff[idx_m3][2 * i] / 4;
    p_out[(2 * i) + 1] = DelayBuff[idxarray][(2 * i) + 1] + DelayBuff[idx_m1][(2 * i) + 1] / 2 + DelayBuff[idx_m2][(2 * i) + 1] / 3 + DelayBuff[idx_m3][(2 * i) + 1] / 4;
  }
  if (i == 2048) // incrementation de la ligne si i= 2048
  {
    idxarray++;
  }
  if (idxarray == 20) // remise a zero
  {
    idxarray = 0;
  }
}
void echobox(uint8_t EffetNB)
{
  switch (EffetNB)
  {
  case 0:
    size_echo = 1;
    break;
  case 1:
    size_echo = 3;
    break;
  case 2:
    size_echo = 5;
    break;
  case 3:
    size_echo = 6;
    break;
  default:
    break;
  }

  if ((DmaLeftRecHalfBuffCplt == 1) && (DmaRightRecHalfBuffCplt == 1))
  {
    /* Store values on Play buff */

    proccessEcho(0, 1024, LeftRecBuff, RightRecBuff, PlayBuff, size_echo);

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

    proccessEcho(1024, 2048, LeftRecBuff, RightRecBuff, PlayBuff, size_echo);

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
