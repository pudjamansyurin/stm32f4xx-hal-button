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
struct ButtonStruct {
  GPIO_TypeDef *port;
  uint16_t pin;
  HAL_LockTypeDef Lock;
  void (*callback)(struct ButtonStruct*);
};

/* Public function declarations */
HAL_StatusTypeDef BTN_Init(struct ButtonStruct *btn,
                           GPIO_TypeDef *port,
                           uint16_t pin,
                           void (*cb)(void));
HAL_StatusTypeDef BTN_DeInit(struct ButtonStruct *btn);
GPIO_PinState BTN_GetState(struct ButtonStruct *btn);

#endif /* INC_MODULES_BUTTON_H_ */
