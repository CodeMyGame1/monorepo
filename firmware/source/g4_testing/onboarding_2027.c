#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == TEST_ONBOARDING_2027)

#include <stdint.h>
#include "common/rtos/rtos.h"

#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/rcc/rcc.h"
#include "common/utils/countof.h"
#include "main.h"


// PA5
#define LED_USER_PORT GPIOA
#define LED_USER_PIN 5


PHAL_GPIO_InitConfig_t gpio_config[] = {
    // Green User LED
    PHAL_GPIO_INIT_OUTPUT(LED_USER_PORT, LED_USER_PIN, GPIO_OUTPUT_LOW_SPEED),
};

void HardFault_Handler();

void blinkGreenLED() {
    PHAL_GPIO_write(LED_USER_PORT, LED_USER_PIN, true);
    RTOS_delay_ms(200);
    PHAL_GPIO_write(LED_USER_PORT, LED_USER_PIN, false);
    RTOS_delay_ms(800);
}

// Define blink task.
RTOS_DEFINE_TASK(blinkGreenLED, 1000, TASK_PRIORITY_NORMAL, STACK_256);

int main() {
    // NUCLEO-G474RE does not have an external oscillator so use the internal 16MHz HSI clock
    PHAL_RCC_init(PHAL_RCC_HSI_16MHZ);

    if (!PHAL_GPIO_init(gpio_config, countof(gpio_config))) {
        HardFault_Handler();
    }
    
    PHAL_GPIO_write(LED_USER_PORT, LED_USER_PIN, true);

    RTOS_START_TASK(blinkGreenLED);

    vTaskStartScheduler();

    while (1) {
        __NOP();
    }
}


void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_ONBOARDING_2027
