# STM32F4xx HAL Button Module

## How to use the module

#### **`main.c`**

```c
#include "stm32f4xx-hal-button/button.h"

static void ButtonCallback(void)
{
  /* Button2 was pressed */
}

int main(void)
{
  struct ButtonStruct hbtn1;
  struct ButtonStruct hbtn2;

  /* Initialize the Button for port A5 as Normal mode */
  BTN_Init(&hbtn1, GPIOA, 5, NULL);

  /* Initialize the Button for port C13 as EXTI mode */
  BTN_Init(&hbtn2, GPIOC, 13, ButtonCallback);

  /* Super loop */
  while(1) {
    /* Polling for Button1 */
    if (BTN_GetState(&hbtn1) == GPIO_PIN_SET) {
      /* Button1 is pressed */
    }

    HAL_Delay(500);
  }

  /* DeInit the Buttons */
  BTN_DeInit(&hbtn1);
  BTN_DeInit(&hbtn2;
}
```

#### **`stm32f4xx_it.c`**

```c
/* USER CODE BEGIN Includes */
#include "stm32f4xx-hal-button-module/button.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 1 */
/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  BTN_IRQHandler();
}
/* USER CODE END 1 */
```