/*
 * button.c
 *
 *  Created on: Feb 12, 2022
 *      Author: pudja
 */
#include "./button.h"
#include "stm32f4xx-hal-common/common.h"

#define BTN_PIN_CNT				(16)

/* Private variables */
static void (*Listeners[BTN_PIN_CNT])(void) = {NULL };

/* Public function definitions */
/**
 * @brief Configures Button GPIO.
 * @note  Ignore cb parameter for normal input mode
 * @param btn Pointer to Button handle
 * @param port The GPIO port used
 * @param pin The GPIO pin used
 * @param cb The callback for EXTI mode
 * @return HAL Status
 */
HAL_StatusTypeDef BTN_Init(struct Button *btn,
                           GPIO_TypeDef *port,
                           uint16_t pin,
                           void (*cb)(void))
{
  GPIO_InitTypeDef GPIO_InitStruct;
  IRQn_Type IRQn;

  /* Check the structure handle allocation */
  if (btn == NULL)
    return HAL_ERROR;

  /* Check the parameters */
  assert_param(IS_GPIO_ALL_INSTANCE(port));
  assert_param(IS_GPIO_PIN(pin));

  /* Initialize properties */
  btn->Lock = HAL_UNLOCKED;
  btn->port = port;
  btn->pin = pin;

  /* Enable the GPIO Clock */
  CMN_PortEnableClock(port);

  /* Configure the GPIO pin */
  if (cb == NULL) {
    /* Configure Button pin as normal input */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  } else {
    /* Configure Button pin as input with External interrupt */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  }
  GPIO_InitStruct.Pin = btn->pin;
  HAL_GPIO_Init(btn->port, &GPIO_InitStruct);

  /* Enable Interrupt in EXTI mode */
  if (cb != NULL) {
    IRQn = CMN_PinGetIrqNumber(btn->pin);

    /* Enable and set Button EXTI Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(IRQn, 0x0F, 0x00);
    HAL_NVIC_EnableIRQ(IRQn);

    /* Added callback, called when interrupt raised */
    Listeners[btn->pin] = cb;
  }

  return (HAL_OK);
}

/**
 * @brief DeInit Buttons.
 * @note It is optional for disable the GPIO clock
 * @param btn Pointer to Button handle
 * @param disable Disable the port clock
 * @return HAL Status
 */
HAL_StatusTypeDef BTN_DeInit(struct Button *btn, uint8_t disable)
{
  __HAL_LOCK(btn);

  /* Disable interrupt pin */
  if (Listeners[btn->pin] != NULL) {
    HAL_NVIC_DisableIRQ(CMN_PinGetIrqNumber(btn->pin));
    /* Remove current handle from list */
    Listeners[btn->pin] = NULL;
  }

  /* DeInit the GPIO pin */
  HAL_GPIO_DeInit(btn->port, btn->pin);

  /* Disable clock */
  if (disable) {
    CMN_PortDisableClock(btn->port);
  }

  __HAL_UNLOCK(btn);
  return (HAL_OK);
}

/**
 * @brief Configure button suspend mode
 * @param btn Pointer to Button handle
 * @param on Suspend state
 * @return HAL Status
 */
HAL_StatusTypeDef BTN_Suspend(struct Button *btn, uint8_t on)
{
  IRQn_Type irq;

  __HAL_LOCK(btn);

  /* Modify interrupt pin */
  if (Listeners[btn->pin] != NULL) {
    irq = CMN_PinGetIrqNumber(btn->pin);
    if (on) {
      HAL_NVIC_DisableIRQ(irq);
    } else {
      HAL_NVIC_EnableIRQ(irq);
    }
  }

  /* Modify clock */
  if (on) {
    CMN_PortDisableClock(btn->port);
  } else {
    CMN_PortEnableClock(btn->port);
  }

  __HAL_UNLOCK(btn);
  return (HAL_OK);
}

/**
 * @brief Returns the selected Button state.
 * @param btn Pointer to Button handle
 * @return The Button GPIO pin value.
 */
GPIO_PinState BTN_GetState(struct Button *btn)
{
  return (HAL_GPIO_ReadPin(btn->port, btn->pin));
}

/**
 * @brief It should be called on all HAL EXTI IRQ lines handler
 */
void BTN_IRQHandler(void)
{
  uint8_t pin;

  for (pin = 0; pin < BTN_PIN_CNT; pin++) {
    if (Listeners[pin] != NULL) {
      HAL_GPIO_EXTI_IRQHandler(pin);
    }
  }
}

/**
 * @brief HAL EXTI Interrupt Callback
 * @param GPIO_Pin The interrupted pin
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  void (*Listener)(void) = Listeners[GPIO_Pin];

  /* Check properties */
  if (Listener != NULL) {
    /* Execute callback */
    Listener();
  }
}
