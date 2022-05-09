/*
 * button.c
 *
 *  Created on: Feb 12, 2022
 *      Author: pudja
 */
#include "./button.h"

/* Private macros */
#define DELAY_MS(__X__)             (HAL_Delay(__X__))
#define GPIO_PIN_CNT                (16)
#define GPIO_PIN(__X__)             (1 << (__X__))
#define GPIO_MASK(__X__)            (((__X__) - GPIOA) / (GPIOB-GPIOA) + 1)
#define RCC_AHB1ENR_GPIOXEN(__X__)  (GPIO_PIN(GPIO_MASK(__X__)))

/* Private variables */
static struct Button *Buttons[GPIO_PIN_CNT];

/* Private function declarations */
static HAL_StatusTypeDef UTIL_PinGetIrqNumber(IRQn_Type *IRQn, uint8_t pin_num);
static void UTIL_PortEnableClock(GPIO_TypeDef *port);
static void UTIL_PortDisableClock(GPIO_TypeDef *port);

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
  if (pin_num >= GPIO_PIN_CNT)
    return HAL_ERROR;

  /* Check the parameters */
  assert_param(IS_GPIO_ALL_INSTANCE(port));
  assert_param(IS_GPIO_PIN(GPIO_PIN(pin_num)));

  /* Initialize properties */
  __HAL_UNLOCK(btn);
  btn->port = port;
  btn->pin_num = pin_num;

  /* Enable the GPIO Clock */
  UTIL_PortEnableClock(port);

  /* Configure the GPIO pin */
  if (cb == NULL)
  {
    /* Configure Button pin as normal input */
    btn->init.Mode = GPIO_MODE_INPUT;
    btn->init.Pull = GPIO_PULLDOWN;
    btn->init.Speed = GPIO_SPEED_FAST;
  }
  else
  {
    /* Configure Button pin as input with External interrupt */
    btn->init.Pull = GPIO_NOPULL;
    btn->init.Mode = GPIO_MODE_IT_FALLING;
  }
  btn->init.Pin = GPIO_PIN(btn->pin_num);
  HAL_GPIO_Init(btn->port, &btn->init);

  /* Enable Interrupt in EXTI mode */
  if (cb != NULL)
  {
    if (UTIL_PinGetIrqNumber(&IRQn, btn->pin_num) == HAL_OK)
    {
      /* Enable and set Button EXTI Interrupt to the lowest priority */
      HAL_NVIC_SetPriority(IRQn, 0x0F, 0x0F);
      HAL_NVIC_EnableIRQ(IRQn);
    }

    /* Added callback, and register */
    btn->callback = cb;

    Buttons[btn->pin_num] = btn;
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
  IRQn_Type IRQn;

  __HAL_LOCK(btn);

  if (Buttons[btn->pin_num] != NULL)
  {
    /* Disable interrupt pin */
    if (UTIL_PinGetIrqNumber(&IRQn, btn->pin_num) == HAL_OK)
      HAL_NVIC_DisableIRQ(IRQn);

    /* Remove current handle from slot */
    Buttons[btn->pin_num] = NULL;
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
HAL_StatusTypeDef BTN_Suspend(struct Button *btn, FunctionalState suspend)
{
  IRQn_Type IRQn;

  __HAL_LOCK(btn);

  /* Modify interrupt pin */
  if (Buttons[btn->pin_num] != NULL)
  {
    if (UTIL_PinGetIrqNumber(&IRQn, btn->pin_num) == HAL_OK)
    {
      if (suspend)
        HAL_NVIC_DisableIRQ(IRQn);
      else
        HAL_NVIC_EnableIRQ(IRQn);
    }
  }

  /* Modify GPIO & clock */
  if (suspend)
  {
    HAL_GPIO_DeInit(btn->port, GPIO_PIN(btn->pin_num));
    UTIL_PortDisableClock(btn->port);
  }
  else
  {
    UTIL_PortEnableClock(btn->port);
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
  struct Button *btn;
  uint16_t GPIO_Pin;
  uint8_t pin_num;

  for (pin_num = 0; pin_num < GPIO_PIN_CNT; pin_num++)
  {
    GPIO_Pin = GPIO_PIN(pin_num);

    /* EXTI line interrupt detected */
    if (RESET != __HAL_GPIO_EXTI_GET_IT(GPIO_Pin))
    {
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);

      /* Get the listener */
      btn = Buttons[pin_num];
      if (btn != NULL)
        btn->callback();
    }
  }
}

/* Private function definitions */
/**
 * @brief Get related IRQ number based on GPIO pin
 * @param IRQn Pointer to destination IRQ buffer
 * @param pin_num The GPIO pin number
 * @return The IRQ number
 */
static HAL_StatusTypeDef UTIL_PinGetIrqNumber(IRQn_Type *IRQn, uint8_t pin_num)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Select appropriate EXTI pin IRQ number */
  if (pin_num >= 10 && pin_num <= 15)
    *IRQn = EXTI15_10_IRQn;
  else if (pin_num >= 5 && pin_num <= 9)
    *IRQn = EXTI9_5_IRQn;
  else if (pin_num == 4)
    *IRQn = EXTI4_IRQn;
  else if (pin_num == 3)
    *IRQn = EXTI3_IRQn;
  else if (pin_num == 2)
    *IRQn = EXTI2_IRQn;
  else if (pin_num == 1)
    *IRQn = EXTI1_IRQn;
  else if (pin_num == 0)
    *IRQn = EXTI0_IRQn;
  else
    status = HAL_ERROR;

  return (status);
}

/**
 * @brief Enable GPIO clock
 * @param port Port to be enabled
 */
static void UTIL_PortEnableClock(GPIO_TypeDef *port)
{
  assert_param(IS_GPIO_ALL_INSTANCE(port));

  /* Enable appropriate GPIO clock */
  __IO uint32_t tmpreg = 0x00U;
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOXEN(port));
  /* Delay after an RCC peripheral clock enabling */
  tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOXEN(port));
  UNUSED(tmpreg);
}

/**
 * @brief Disable GPIO clock
 * @param port Port to be disabled
 */
static void UTIL_PortDisableClock(GPIO_TypeDef *port)
{
  assert_param(IS_GPIO_ALL_INSTANCE(port));

  /* Disable appropriate GPIO clock */
  RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIOXEN(port));
}

