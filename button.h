/*
 * button.h
 *
 *  Created on: Feb 12, 2022
 *      Author: pudja
 */

#ifndef INC_MODULES_BUTTON_H_
#define INC_MODULES_BUTTON_H_

#include "stm32f4xx_hal.h"

/* Public macros */
#define DELAY_MS(__X__)        HAL_Delay(__X__)

/* Public types */
struct Button {
  GPIO_TypeDef *port;
  uint16_t pin;
  HAL_LockTypeDef Lock;
  void (*callback)(struct Button*);
};

/* Public function declarations */
HAL_StatusTypeDef BTN_Init(struct Button *btn,
                           GPIO_TypeDef *port,
                           uint16_t pin,
                           void (*cb)(void));
HAL_StatusTypeDef BTN_DeInit(struct Button *btn, uint8_t disable);
HAL_StatusTypeDef BTN_Suspend(struct Button *btn, uint8_t on);
GPIO_PinState BTN_GetState(struct Button *btn);

/* Interrupt Request Handler */
void BTN_IRQHandler(void);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#endif /* INC_MODULES_BUTTON_H_ */
