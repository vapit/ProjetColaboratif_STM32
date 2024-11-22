/**
  ******************************************************************************
<<<<<<<< HEAD:projetCollaboratif_STM32/Core/Inc/dma.h
  * @file    dma.h
  * @brief   This file contains all the function prototypes for
  *          the dma.c file
========
  * @file    cpu_utils.h
  * @author  MCD Application Team
  * @brief   Header for cpu_utils module
>>>>>>>> Yann:projetCollaboratif_STM32/Utilities/CPU/cpu_utils.h
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2014 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
<<<<<<<< HEAD:projetCollaboratif_STM32/Core/Inc/dma.h
#ifndef __DMA_H__
#define __DMA_H__
========
#ifndef _CPU_UTILS_H__
#define _CPU_UTILS_H__
>>>>>>>> Yann:projetCollaboratif_STM32/Utilities/CPU/cpu_utils.h

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

<<<<<<<< HEAD:projetCollaboratif_STM32/Core/Inc/dma.h
/* DMA memory to memory transfer handles -------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_DMA_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */
========
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define CALCULATION_PERIOD    1000

/* Exported functions ------------------------------------------------------- */
uint16_t osGetCPUUsage (void);
>>>>>>>> Yann:projetCollaboratif_STM32/Utilities/CPU/cpu_utils.h

#ifdef __cplusplus
}
#endif

<<<<<<<< HEAD:projetCollaboratif_STM32/Core/Inc/dma.h
#endif /* __DMA_H__ */

========
#endif /* _CPU_UTILS_H__ */
>>>>>>>> Yann:projetCollaboratif_STM32/Utilities/CPU/cpu_utils.h
