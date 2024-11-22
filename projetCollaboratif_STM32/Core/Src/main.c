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
#include "i2c.h"
#include "i2s.h"
#include "quadspi.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../../Drivers/BSP/STM32412G-Discovery/stm32412g_discovery.h"
#include "../../../Drivers/BSP/STM32412G-Discovery/stm32412g_discovery_lcd.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    uint16_t x;            // Position X
    uint16_t y;            // Position Y
    uint16_t width;        // Largeur
    uint16_t height;       // Hauteur
    uint32_t color;        // Couleur par défaut
    uint32_t selectedColor; // Couleur en surbrillance
    const char *label;     // Texte du bouton
} Button;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
Button buttons[4];          // Tableau pour les 4 boutons
uint8_t selectedButton = 0; // Indice du bouton sélectionné
uint8_t previousSelectedButton = 0; // Variable pour suivre l'état précédent de la sélection
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void InitializeButtons();
void HighlightButton(uint8_t index);
void HandleJoystick();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief Initialisation des boutons à l'écran LCD
 */
void InitializeButtons() {
    // Définir les positions, dimensions, couleurs et labels des boutons
    buttons[0] = (Button){20, 60, 80, 40, LCD_COLOR_BLUE, LCD_COLOR_RED, "Effet 1"};
    buttons[1] = (Button){140, 60, 80, 40, LCD_COLOR_BLUE, LCD_COLOR_RED, "Effet 2"};
    buttons[2] = (Button){20, 140, 80, 40, LCD_COLOR_BLUE, LCD_COLOR_RED, "Effet 3"};
    buttons[3] = (Button){140, 140, 80, 40, LCD_COLOR_BLUE, LCD_COLOR_RED, "Effet 4"};

    // Dessiner les boutons et afficher leurs labels
    for (uint8_t i = 0; i < 4; i++) {
        BSP_LCD_SetTextColor(buttons[i].color);
        BSP_LCD_FillRect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height);

        // Afficher le label centré sur le bouton
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE); // Couleur du texte
        BSP_LCD_SetBackColor(buttons[i].color); // Fond du texte
        BSP_LCD_DisplayStringAt(
            buttons[i].x + (buttons[i].width / 2) - (strlen(buttons[i].label) * 4), // Ajuster le texte à la taille
            buttons[i].y + (buttons[i].height / 2) - 8, // Centrer verticalement
            (uint8_t *)buttons[i].label,
            LEFT_MODE
        );
    }
    previousSelectedButton = selectedButton; // Initialiser l'état précédent
    HighlightButton(0); // Mettre le premier bouton en surbrillance
}

/**
 * @brief Met en surbrillance un bouton spécifique
 * @param index L'indice du bouton à surligner
 */
void HighlightButton(uint8_t index) {
    // Si le bouton sélectionné n'a pas changé, ne pas redessiner
    if (index == previousSelectedButton) {
        return;
    }

    // Dessiner les boutons sans leur label
    for (uint8_t i = 0; i < 4; i++) {
        if (i == index) {
            BSP_LCD_SetTextColor(buttons[i].selectedColor); // Choisir la couleur de surbrillance
        } else {
            BSP_LCD_SetTextColor(buttons[i].color); // Couleur par défaut
        }
        BSP_LCD_FillRect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height);
    }

    // Réafficher les labels (en maintenant la couleur du texte)
    for (uint8_t i = 0; i < 4; i++) {
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE); // Couleur du texte
        BSP_LCD_SetBackColor(i == index ? buttons[i].selectedColor : buttons[i].color); // Fond du texte
        BSP_LCD_DisplayStringAt(
            buttons[i].x + (buttons[i].width / 2) - (strlen(buttons[i].label) * 4), // Ajuster le texte à la taille
            buttons[i].y + (buttons[i].height / 2) - 8, // Centrer verticalement
            (uint8_t *)buttons[i].label,
            LEFT_MODE
        );
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
void HandleJoystick() {
    JOYState_TypeDef joystickState = BSP_JOY_GetState();

    switch (joystickState) {
        case JOY_UP:
            if (selectedButton > 1) selectedButton -= 2; // Bouton au-dessus
            break;
        case JOY_DOWN:
            if (selectedButton < 2) selectedButton += 2; // Bouton en dessous
            break;
        case JOY_LEFT:
            if (selectedButton % 2 == 1) selectedButton--; // Bouton à gauche
            break;
        case JOY_RIGHT:
            if (selectedButton % 2 == 0) selectedButton++; // Bouton à droite
            break;
        case JOY_SEL:
            // Réinitialiser les boutons et activer le bouton sélectionné
            ResetButtons();
            break;
        default:
            break;
    }

    HighlightButton(selectedButton); // Mettre à jour la surbrillance si nécessaire
    HAL_Delay(200); // Anti-rebond
}

/* USER CODE END 0 */



/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FSMC_Init();
  BSP_LCD_Init();
  BSP_LCD_Clear(LCD_COLOR_WHITE);
  BSP_JOY_Init(JOY_MODE_GPIO);

  /* USER CODE BEGIN 2 */
  InitializeButtons();
  /* USER CODE END 2 */

  /* Infinite loop */
  while (1) {
    /* USER CODE BEGIN WHILE */
    HandleJoystick();
    /* USER CODE END WHILE */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 72;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 3;
    RCC_OscInitStruct.PLL.PLLR = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN Error_Handler */
void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
/* USER CODE END Error_Handler */

