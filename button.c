/*
 * button.c
 *
 *  Created on: Feb 12, 2022
 *      Author: pudja
 */
#include "./button.h"
#include "stm32f4xx-hal-common/common.h"

/* Private variables */
static void (*Listeners[GPIO_PIN_CNT])(void) = {NULL };

/* Public function definitions */
/**
 * @brief Configures Button GPIO.
 * @note  Ignore cb parameter for normal input mode
 * @param btn Pointer to Button handle
 * @param port The GPIO port used
 * @param pin_num The GPIO pin number
 * @param cb The callback for EXTI mode
 * @return HAL Status
 */
HAL_StatusTypeDef BTN_Init(struct Button *btn,
                           GPIO_TypeDef *port,
                           uint8_t pin_num,
                           void (*cb)(void))
{
  IRQn_Type IRQn;

  /* Check the structure handle allocation */
  if (btn == NULL)
    return HAL_ERROR;
  if (pin_num >= GPIO_PIN_CNT) {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_GPIO_ALL_INSTANCE(port));
  assert_param(IS_GPIO_PIN(GPIO_PIN(pin_num)));

  /* Initialize properties */
  btn->Lock = HAL_UNLOCKED;
  btn->port = port;
  btn->pin_num = pin_num;

  /* Enable the GPIO Clock */
  CMN_PortEnableClock(port);

  /* Configure the GPIO pin */
  if (cb == NULL) {
    /* Configure Button pin as normal input */
    btn->init.Mode = GPIO_MODE_INPUT;
    btn->init.Pull = GPIO_PULLDOWN;
    btn->init.Speed = GPIO_SPEED_FAST;
  } else {
    /* Configure Button pin as input with External interrupt */
    btn->init.Pull = GPIO_NOPULL;
    btn->init.Mode = GPIO_MODE_IT_FALLING;
  }
  btn->init.Pin = GPIO_PIN(btn->pin_num);
  HAL_GPIO_Init(btn->port, &btn->init);

  /* Enable Interrupt in EXTI mode */
  if (cb != NULL) {
    IRQn = CMN_PinGetIrqNumber(btn->pin_num);

    /* Enable and set Button EXTI Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(IRQn, 0x0F, 0x00);
    HAL_NVIC_EnableIRQ(IRQn);

    /* Added callback, called when interrupt raised */
    Listeners[btn->pin_num] = cb;
  }

  return (HAL_OK);
}

/**
 * @brief DeInit Buttons.
 * @note The clock port is not disabled by default
 * @param btn Pointer to Button handle
 * @return HAL Status
 */
HAL_StatusTypeDef BTN_DeInit(struct Button *btn)
{
  __HAL_LOCK(btn);

  /* Disable interrupt pin */
  if (Listeners[btn->pin_num] != NULL) {
    HAL_NVIC_DisableIRQ(CMN_PinGetIrqNumber(btn->pin_num));
    /* Remove current handle from list */
    Listeners[btn->pin_num] = NULL;
  }

  /* DeInit the GPIO pin */
  HAL_GPIO_DeInit(btn->port, GPIO_PIN(btn->pin_num));

  __HAL_UNLOCK(btn);
  return (HAL_OK);
}

/**
 * @brief Configure button suspend mode
 * @note The clock port is also disabled
 * @param btn Pointer to Button handle
 * @param suspend Suspend state
 * @return HAL Status
 */
HAL_StatusTypeDef BTN_Suspend(struct Button *btn, uint8_t suspend)
{
  IRQn_Type irq;

  __HAL_LOCK(btn);

  /* Modify interrupt pin */
  if (Listeners[btn->pin_num] != NULL) {
    irq = CMN_PinGetIrqNumber(btn->pin_num);
    if (suspend) {
      HAL_NVIC_DisableIRQ(irq);
    } else {
      HAL_NVIC_EnableIRQ(irq);
    }
  }

  /* Modify GPIO & clock */
  if (suspend) {
    HAL_GPIO_DeInit(btn->port, GPIO_PIN(btn->pin_num));
    CMN_PortDisableClock(btn->port);
  } else {
    CMN_PortEnableClock(btn->port);
    HAL_GPIO_Init(btn->port, &btn->init);
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
  return (HAL_GPIO_ReadPin(btn->port, GPIO_PIN(btn->pin_num)));
}

/**
 * @brief It should be called on all HAL EXTI IRQ lines handler
 */
void BTN_IRQHandler(void)
{
  uint8_t pin_num;

  for (pin_num = 0; pin_num < GPIO_PIN_CNT; pin_num++) {
    if (Listeners[pin_num] != NULL) {
      HAL_GPIO_EXTI_IRQHandler(GPIO_PIN(pin_num));
    }
  }
}

/**
 * @brief HAL EXTI Interrupt Callback
 * @param GPIO_Pin The interrupted pin
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  void (*Listener)(void) = Listeners[CMN_PinGetNumber(GPIO_Pin)];

  /* Check properties */
  if (Listener != NULL) {
    /* Execute callback */
    Listener();
  }
}
