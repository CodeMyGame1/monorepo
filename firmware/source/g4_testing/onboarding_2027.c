#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == TEST_ONBOARDING_2027)

#include <stdint.h>
#include "common/rtos/rtos.h"

#include "common/phal_G4/gpio/gpio.h"
#include "common/phal_G4/rcc/rcc.h"
#include "common/utils/countof.h"
#include "main.h"

// CAN imports
#include "can_library/can_common.h"
#include "common/phal_G4/pin_defs/g474ret6.h"
#include "can_library/generated/ONBOARDING.h"


// PA5
#define LED_USER_PORT GPIOA
#define LED_USER_PIN 5

PHAL_GPIO_InitConfig_t gpio_config[] = {
    // Green User LED
    PHAL_GPIO_INIT_OUTPUT(LED_USER_PORT, LED_USER_PIN, GPIO_OUTPUT_LOW_SPEED),
    
    // CAN: Using PA11 (RX) and PA12 (TX)
    PHAL_PIN_DEFS_FDCAN2_RX_PB12,
    PHAL_PIN_DEFS_FDCAN2_TX_PB13,
};

void HardFault_Handler();

uint16_t counter = 0;

void blinkGreenLED() {
    PHAL_GPIO_write(LED_USER_PORT, LED_USER_PIN, true);
    RTOS_delay_ms(200);
    PHAL_GPIO_write(LED_USER_PORT, LED_USER_PIN, false);
    RTOS_delay_ms(800);

    // CAN comms.
    counter++;
    CAN_SEND_heartbeat_onboarding(counter, 7);
}

// Thread defines.
RTOS_DEFINE_TASK(blinkGreenLED, 1000, TASK_PRIORITY_NORMAL, STACK_256);
DEFINE_CAN_TASKS();

int main() {
    // NUCLEO-G474RE does not have an external oscillator so use the internal 16MHz HSI clock
    PHAL_RCC_init(PHAL_RCC_HSI_16MHZ);

    if (!PHAL_GPIO_init(gpio_config, countof(gpio_config))) {
        HardFault_Handler();
    }

    // Setup CAN + FDCAN1 peripheral.
    PHAL_FDCAN_init(FDCAN2, CCAN_BAUD_RATE);
    CAN_init();
    START_CAN_TASKS()

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
