/*
 * button.c
 *
 *  Created on: Feb 12, 2022
 *      Author: pudja
 */
#include "./button.h"

#define BTN_PIN_CNT				(16)

/* Private variables */
static void (*Listeners[BTN_PIN_CNT])(void) = {NULL };

/* Private function declarations */
static inline void PortEnableClock(GPIO_TypeDef *port);
static inline IRQn_Type PinGetIrqNumber(uint16_t pin);

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
  PortEnableClock(port);

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
    IRQn = PinGetIrqNumber(btn->pin);

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
 * @note Button DeInit does not disable the GPIO clock
 * @param btn Pointer to Button handle
 * @return HAL Status
 */
HAL_StatusTypeDef BTN_DeInit(struct Button *btn)
{
  __HAL_LOCK(btn);

  /* DeInit the GPIO pin */
  if (Listeners[btn->pin] != NULL) {
    HAL_NVIC_DisableIRQ(PinGetIrqNumber(btn->pin));
    /* Remove current handle from list */
    Listeners[btn->pin] = NULL;
  }
  HAL_GPIO_DeInit(btn->port, btn->pin);

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

/* Private function definitions */
/**
 * @brief Get related IRQ number based on GPIO pin
 * @param pin The GPIO pin
 * @return The IRQ number
 */
static inline IRQn_Type PinGetIrqNumber(uint16_t pin)
{
  IRQn_Type IRQn;

  /* Select appropriate EXTI pin IRQ number */
  if (pin >= GPIO_PIN_10)
    IRQn = EXTI15_10_IRQn;
  else if (pin >= GPIO_PIN_5)
    IRQn = EXTI9_5_IRQn;
  else if (pin == GPIO_PIN_4)
    IRQn = EXTI4_IRQn;
  else if (pin == GPIO_PIN_3)
    IRQn = EXTI3_IRQn;
  else if (pin == GPIO_PIN_2)
    IRQn = EXTI2_IRQn;
  else if (pin == GPIO_PIN_1)
    IRQn = EXTI1_IRQn;
  else if (pin == GPIO_PIN_0)
    IRQn = EXTI0_IRQn;

  return (IRQn);
}

/**
 * @brief Enable GPIO clock
 * @param port Port to be enabled
 */
static inline void PortEnableClock(GPIO_TypeDef *port)
{
  assert_param(IS_GPIO_ALL_INSTANCE(port));

  /* Enable appropriate GPIO clock */
  if (port == GPIOH)
    __HAL_RCC_GPIOH_CLK_ENABLE();
  else if (port == GPIOG)
    __HAL_RCC_GPIOG_CLK_ENABLE();
  else if (port == GPIOF)
    __HAL_RCC_GPIOF_CLK_ENABLE();
  else if (port == GPIOE)
    __HAL_RCC_GPIOE_CLK_ENABLE();
  else if (port == GPIOD)
    __HAL_RCC_GPIOD_CLK_ENABLE();
  else if (port == GPIOC)
    __HAL_RCC_GPIOC_CLK_ENABLE();
  else if (port == GPIOB)
    __HAL_RCC_GPIOB_CLK_ENABLE();
  else if (port == GPIOA)
    __HAL_RCC_GPIOA_CLK_ENABLE();
}
