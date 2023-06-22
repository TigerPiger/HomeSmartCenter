/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-29     Rbb666       first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include "drv_gpio.h"
#include "sensor_dallas_dht11.h"

#if defined(BSP_USING_DHT11) && defined( PKG_USING_DHT11 )
extern int rt_hw_dht11_port(void);
extern int dht11_read_temp_sample(void);
#endif

#define LED_PIN     GET_PIN(0, 1)
#define BUTTON_PIN  GET_PIN(6, 2) // MCU_USER_BTN

static rt_sem_t button_sem = RT_NULL;

static rt_thread_t button_thread;
void button_thread_entry(void* parameter) {
    rt_pin_mode(BUTTON_PIN, PIN_MODE_INPUT_PULLUP);

    while (1) {
        if (rt_pin_read(BUTTON_PIN) == PIN_LOW) {
            rt_thread_delay(5);
            if (rt_pin_read(BUTTON_PIN) == PIN_LOW)
                rt_sem_release(button_sem);
            while(rt_pin_read(BUTTON_PIN) == PIN_LOW);
        }
        rt_thread_delay(10);
    }

}

static rt_thread_t led_thread;
void led_thread_entry(void* parameter) {
    static rt_base_t pin_value = PIN_LOW;

    while(1) {
        rt_sem_take(button_sem, RT_WAITING_FOREVER);
        rt_pin_write(LED_PIN, pin_value);
        pin_value = (pin_value == PIN_LOW) ? PIN_HIGH : PIN_LOW;
    }
}

int main(void)
{
#if defined(BSP_USING_DHT11) && defined( PKG_USING_DHT11 )
    dht11_read_temp_sample();
#endif

    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LED_PIN, PIN_HIGH);

    button_sem = rt_sem_create("button_press", 0, RT_IPC_FLAG_FIFO);

    button_thread = rt_thread_create("button", button_thread_entry, RT_NULL, 512, 10, 5);
    rt_thread_startup(button_thread);

    led_thread = rt_thread_create("led", led_thread_entry, RT_NULL, 512, 10, 5);
    rt_thread_startup(led_thread);
}
