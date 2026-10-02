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

uint16_t blink_count = 0;

// LED time on and time off in milliseconds,
uint16_t timer = 0;
uint16_t freq = 0;

void blinkGreenLED() {
    if (timer > 5) {
        timer = 0;
        blink_count++;
    }

    // Read message from DaqApp, check if it's stale.
    // if ((can_data.response_onboarding.last_rx - xTaskGetTickCount()) > 2500) {
    if (can_data.response_onboarding.is_stale()) {
        // On for 1 * 200 = 200ms, off for remaining 800ms
        freq = 1;
    } else {
        // Round DaqApp input to nearest 200ms.
        freq = (uint16_t) (can_data.response_onboarding.on_amount / 200);
    }

    // Create blinking functionality.
    // TODO: Bug where even at `on_amount` set to 1000ms, LED briefly flashes off.
    if (timer < freq) {
        PHAL_GPIO_write(LED_USER_PORT, LED_USER_PIN, true);
    } else {
        PHAL_GPIO_write(LED_USER_PORT, LED_USER_PIN, false);
    }

    // CAN comms.
    CAN_SEND_heartbeat_onboarding(blink_count, 7);
    
    // Update timer.
    timer++;
}

// Thread defines.
RTOS_DEFINE_TASK(blinkGreenLED, 200, TASK_PRIORITY_NORMAL, STACK_256);
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
