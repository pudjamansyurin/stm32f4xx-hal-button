# STM32F4xx HAL Button Module

## How to use the module

```c
#include "stm32f4xx-hal-button-module/button.h"

static void ButtonCallback(void)
{
  /* Button2 was pressed */
}

int main(void)
{
  struct ButtonStruct hbtn1;
  struct ButtonStruct hbtn2;
  
  /* Initialize the Button for port C13 as Normal mode */
  BTN_Init(&hbtn1, GPIOC, GPIO_PIN_13, NULL);
  
  /* Initialize the Button for port C14 as EXTI mode */
  BTN_Init(&hbtn2, GPIOC, GPIO_PIN_14, ButtonCallback);
  
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