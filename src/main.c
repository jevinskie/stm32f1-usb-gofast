#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include <zephyr/arch/cache.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/timing/timing.h>

LOG_MODULE_REGISTER(main);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE     DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

typedef void (*spinner_fptr_t)(uint32_t n);

__attribute__((noinline)) void spinner(uint32_t n) {
    do {
        __asm__ __volatile__(
            "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tno"
            "p\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop"
            "\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n"
            "\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
            "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tno"
            "p\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop"
            "\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n"
            "\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
            "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tno"
            "p\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop"
            "\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n"
            "\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
            "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tno"
            "p\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop"
            "\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n"
            "\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
            "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tno"
            "p\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop"
            "\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n"
            "\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
            "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t" ::
                : "memory");
        n--;
    } while (n != 0);
}

static uint16_t code_sram[300];

int main(void) {
    int ret;
    bool led_state = true;

    LOG_INF("entered main.");

    if (!gpio_is_ready_dt(&led)) {
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    void *const code_start_flash = (void *const)((uintptr_t)&spinner & (UINTPTR_MAX ^ 1));
    void *const code_end_flash   = (void *const)((uintptr_t)code_start_flash + sizeof(code_sram));

    void *code_start_sram = &code_sram;
    void *code_end_sram   = (void *)((uintptr_t)code_start_sram + sizeof(code_sram));

    printk("copying code from [%p, %p) to [%p, %p)\n", code_start_flash, code_end_flash,
           code_start_sram, code_end_sram);

    memcpy(code_sram, code_start_flash, sizeof(code_sram));
#if defined(CONFIG_ICACHE)
    arch_icache_flush_all();
    arch_icache_invd_all();
#endif
    spinner_fptr_t spinner_sram = (spinner_fptr_t)((uintptr_t)code_start_sram | 1);

    printk("code copy done, spinner_sram: %p\n", spinner_sram);

    timing_init();
    timing_start();

    printk("printk float: %f\n", 1.2 + ret);

    timing_t start_flash = timing_counter_get();
    spinner(4 * 1024 * 1024);
    timing_t end_flash          = timing_counter_get();
    const uint64_t cycles_flash = timing_cycles_get(&start_flash, &end_flash);
    printk("cycles_flash: %" PRIu64 "\n", cycles_flash);

    timing_t start_sram = timing_counter_get();
    spinner_sram(4 * 1024 * 1024);
    timing_t end_sram          = timing_counter_get();
    const uint64_t cycles_sram = timing_cycles_get(&start_sram, &end_sram);
    printk("cycles_sram: %" PRIu64 "\n", cycles_sram);

    timing_stop();

    while (1) {
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0) {
            return 0;
        }

        led_state = !led_state;
        printf("LED state: %s\n", led_state ? "ON" : "OFF");
        k_msleep(SLEEP_TIME_MS);
    }
    return 0;
}
