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

#include <board.h>
#include <string.h>
#include <stdio.h>

#include "drv_gpio.h"
//#include "sensor_dallas_dht11.h"

#include "DHT11.h"
#include "ssd1306.h"
#include "showChinese.h"

//#if defined(BSP_USING_DHT11) && defined( PKG_USING_DHT11 )
//extern int rt_hw_dht11_port(void);
//extern int dht11_read_temp_sample(void);
//#endif

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

static rt_thread_t test_thread;
void test_thread_entry(void* parameter) {
    uint8* data;

    char buffer[100];
    static uint8_t offset = 0;

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    LOG_E("Results\n\r");

    while(1) {
        /* Place your application code here. */
        rt_enter_critical();
        data = DHT_Read();
        rt_exit_critical();
        if(data[0] != ERROR){
            LOG_E(" Humidity: %d \n\r Temperature: %d \n\r", data[0], data[2]);
        }else{
            LOG_E(" Error reading sensor!\n\r");
        }

        offset = 5;
        ssd1306_Fill(Black);
        ssd1306_SetCursor(offset, 10);
        ssd1306_WriteString("RT-Thread x Infineon", Font_6x8, White);

        OLED_DrawHz(offset, 20, 0, White);
        offset+=16;
        OLED_DrawHz(offset, 20, 1, White);
        offset+=16;
        OLED_DrawHz(offset, 20, 2, White);
        offset+=16;
        OLED_DrawHz(offset, 20, 3, White);
        offset+=16;
        OLED_DrawHz(offset, 20, 4, White);
        offset+=16;
        OLED_DrawHz(offset, 20, 5, White);


        ssd1306_SetCursor(5, 40);
        rt_sprintf(buffer, "Humidity: %d", data[0]);
        ssd1306_WriteString(buffer, Font_7x10, White);
    
        ssd1306_SetCursor(5, 50);
        rt_sprintf(buffer, "Temperature: %d", data[2]);
        ssd1306_WriteString(buffer, Font_7x10, White);

        ssd1306_UpdateScreen();



        rt_thread_mdelay(1000);
    }
}


int main(void)
{
//#if defined(BSP_USING_DHT11) && defined( PKG_USING_DHT11 )
//    dht11_read_temp_sample();
//#endif
    ssd1306_Init();

    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LED_PIN, PIN_HIGH);

    button_sem = rt_sem_create("button_press", 0, RT_IPC_FLAG_FIFO);

    button_thread = rt_thread_create("button", button_thread_entry, RT_NULL, 512, 10, 10);
    rt_thread_startup(button_thread);

    led_thread = rt_thread_create("led", led_thread_entry, RT_NULL, 512, 10, 10);
    rt_thread_startup(led_thread);

    test_thread = rt_thread_create("test", test_thread_entry, RT_NULL, 1024, 10, 5);
    rt_thread_startup(test_thread);
}
