/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

extern osThreadId usbTaskHandle;
extern osThreadId keyTaskHandle;
extern osThreadId i2cTaskHandle;
extern osThreadId appTaskHandle;
extern osThreadId i2cOledTaskHandle;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ENC_UPDATE_TIM_IRQn TIM7_IRQn
#define ENC_MOTOR2_TIM TIM1
#define ENC_MOTOR1_TIM TIM2
#define ENC_NOB1_TIM TIM3
#define ENC_NOB2_TIM TIM4
#define ENC_UPDATE_TIM TIM7
#define PWM_TIM TIM9
#define MOTOR2_SWT1_Pin LL_GPIO_PIN_0
#define MOTOR2_SWT1_GPIO_Port GPIOC
#define MOTOR1_SWT1_Pin LL_GPIO_PIN_1
#define MOTOR1_SWT1_GPIO_Port GPIOC
#define MOTOR2_SWT2_Pin LL_GPIO_PIN_2
#define MOTOR2_SWT2_GPIO_Port GPIOC
#define MOTOR1_SWT2_Pin LL_GPIO_PIN_3
#define MOTOR1_SWT2_GPIO_Port GPIOC
#define USR_BTN_Pin LL_GPIO_PIN_0
#define USR_BTN_GPIO_Port GPIOA
#define ENABLE_A_Pin LL_GPIO_PIN_2
#define ENABLE_A_GPIO_Port GPIOA
#define ENABLE_B_Pin LL_GPIO_PIN_3
#define ENABLE_B_GPIO_Port GPIOA
#define SPI1_NSS_Pin LL_GPIO_PIN_4
#define SPI1_NSS_GPIO_Port GPIOA
#define SPI1_NSS_EXTI_IRQn EXTI4_IRQn
#define MOTOR1_LED_Pin LL_GPIO_PIN_4
#define MOTOR1_LED_GPIO_Port GPIOC
#define MOTOR2_LED_Pin LL_GPIO_PIN_5
#define MOTOR2_LED_GPIO_Port GPIOC
#define INA1_Pin LL_GPIO_PIN_6
#define INA1_GPIO_Port GPIOC
#define INA2_Pin LL_GPIO_PIN_7
#define INA2_GPIO_Port GPIOC
#define INB1_Pin LL_GPIO_PIN_8
#define INB1_GPIO_Port GPIOC
#define INB2_Pin LL_GPIO_PIN_9
#define INB2_GPIO_Port GPIOC
#define ENC_MOTOR2_A_Pin LL_GPIO_PIN_8
#define ENC_MOTOR2_A_GPIO_Port GPIOA
#define ENC_MOTOR2_B_Pin LL_GPIO_PIN_9
#define ENC_MOTOR2_B_GPIO_Port GPIOA
#define ENC_MOTOR1_A_Pin LL_GPIO_PIN_15
#define ENC_MOTOR1_A_GPIO_Port GPIOA
#define TSD_ARM_Pin LL_GPIO_PIN_11
#define TSD_ARM_GPIO_Port GPIOC
#define LED_Pin LL_GPIO_PIN_2
#define LED_GPIO_Port GPIOD
#define ENC_MOTOR1_B_Pin LL_GPIO_PIN_3
#define ENC_MOTOR1_B_GPIO_Port GPIOB
#define ENC_NOB1_A_Pin LL_GPIO_PIN_4
#define ENC_NOB1_A_GPIO_Port GPIOB
#define ENC_NOB1_B_Pin LL_GPIO_PIN_5
#define ENC_NOB1_B_GPIO_Port GPIOB
#define ENC_NOB2_A_Pin LL_GPIO_PIN_6
#define ENC_NOB2_A_GPIO_Port GPIOB
#define ENC_NOB2_B_Pin LL_GPIO_PIN_7
#define ENC_NOB2_B_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
