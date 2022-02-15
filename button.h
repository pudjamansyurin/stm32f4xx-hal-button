/*
 * button.h
 *
 *  Created on: Feb 12, 2022
 *      Author: pudja
 */

#ifndef INC_MODULES_BUTTON_H_
#define INC_MODULES_BUTTON_H_

#include "stm32f4xx_hal.h"

/* Public types */
struct Button {
  GPIO_TypeDef *port;
  GPIO_InitTypeDef init;
  uint8_t pin_num;
  HAL_LockTypeDef Lock;
};

/* Public function declarations */
HAL_StatusTypeDef BTN_Init(struct Button *btn,
                           GPIO_TypeDef *port,
                           uint8_t pin_num,
                           void (*cb)(void));
HAL_StatusTypeDef BTN_DeInit(struct Button *btn);
HAL_StatusTypeDef BTN_Suspend(struct Button *btn, uint8_t suspend);
GPIO_PinState BTN_GetState(struct Button *btn);

/* Interrupt Request Handler */
void BTN_IRQHandler(void);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#endif /* INC_MODULES_BUTTON_H_ */
