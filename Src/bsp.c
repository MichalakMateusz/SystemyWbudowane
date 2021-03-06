/**
  ******************************************************************************
  * @file    bsp.c 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    18-June-2014
  * @brief   This file provides the demo bsp functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"


/** @addtogroup CORE
  * @{
  */

/** @defgroup BSP
  * @brief BSP routines
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* TSC handler declaration */
/* USB handler declaration */

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes LEDs, Button, EPD and LTS.
  * @param  None 
  * @retval None
  */
void Bsp_Init(void)
{
  /* Configure LED3 and LED4 */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  
  /* Initialize the User button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
  
  /* Initialize the EPD */
  BSP_EPD_Init();
  
  /* Initialize the Touch sensor */
  BSP_TSL_Init();
}

/**
  * @brief  Configure the touch sensor.
  * @param  None 
  * @retval None
  */
void BSP_TSL_Init(void)
{
  /* Configure the TSC peripheral */
  /* All channel, shield and sampling IOs must be declared below */
  /* Initialize the STMTouch driver */
}

/**
  * @brief  Initializes LEDs, Button, and EPD.
  * @param  None 
  * @retval None
  */
void BSP_USB_Init(void)
{
  /* Init Device Library */
  
  /* Register the HID class */
  
  /* Start Device Process */
}

/**
  * @}
  */

/**
  * @}
  *
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
