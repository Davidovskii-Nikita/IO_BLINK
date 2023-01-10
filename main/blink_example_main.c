/**
 * @file blink_example_main.c
 * @author Davidvoskii Nikita (Davidovskiinikita@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include <stdbool.h>
#include <cJSON.h>

#define BIT_MASK(val, mask) ((val) & (1 << (mask)))// макрос возвращает бит mask в значении val
#define LEN_PINOUT 4 // Колличество пар вход-выход в массивах (колличество его элементов).

/**
 * Описывает пару вход-выход.
 *
 */
static struct 
{
    uint32_t input_adr[LEN_PINOUT];   /*!< Адресс входа. */
    uint32_t output_adr[LEN_PINOUT]; /*!< Адрес выхода. */
}Pin_out_adr={{2,13,4,9},{14,25,16,5}};

/**
 * Хранит биты состояний вход-выход.
 *
 */
struct {
    uint32_t pin_in;
    uint32_t pins_out_reg;
}Pin_out_reg;

/**
 * @brief Инициализирует выходы/выходы.
 *
 */
void GPIO_Init()
{   
    for (uint32_t i = 0; i < LEN_PINOUT; i++){
        ESP_ERROR_CHECK(gpio_set_direction(Pin_out_adr.output_adr[i], GPIO_MODE_OUTPUT));
        ESP_ERROR_CHECK(gpio_set_direction(Pin_out_adr.input_adr[i], GPIO_MODE_INPUT));
    }
}

/**
 * @brief Создает и принтует JSON строку.
 *
 * @return string указатель на строку в куче, которую необходимо освободить
 *
 */
char *create_monitor_with_helpers()
{
    char *string = NULL;
    cJSON *pinouts = NULL;

    cJSON *monitor = cJSON_CreateObject();
    pinouts = cJSON_AddArrayToObject(monitor, "pinout_v2");

    for (uint32_t i = 0; i < LEN_PINOUT; i++){
        cJSON *pinout = cJSON_CreateObject();

        cJSON_AddNumberToObject(pinout,"input",Pin_out_adr.input_adr[i]);

        cJSON_AddNumberToObject(pinout,"output",Pin_out_adr.output_adr[i]);

        if(BIT_MASK(Pin_out_reg.pin_in,Pin_out_adr.input_adr[i])){
            cJSON_AddNumberToObject(pinout,"state",1);
        }
        else{
            cJSON_AddNumberToObject(pinout,"state",0);
        }

        cJSON_AddItemToArray(pinouts, pinout);
    }

    string = cJSON_Print(monitor);
    printf(string);// Заменить на sprintf!!!
    if (string == NULL){
        printf("Failed to print monitor.\n");
    }
    return string;
}

/**
 * @brief Устанавливает логическое состояние выходов.
 * 
 * @return uint32_t ESP_OK (0) = успех
 */
uint32_t set_output()
{
    esp_err_t ret = ESP_OK;

    for (uint32_t i = 0; i < LEN_PINOUT; i++){
        if(BIT_MASK(Pin_out_reg.pin_in,Pin_out_adr.input_adr[i])){
            ESP_ERROR_CHECK(gpio_set_level(Pin_out_adr.output_adr[i],1));
        }
        else{
            ESP_ERROR_CHECK(gpio_set_level(Pin_out_adr.output_adr[i],0));
        }
    }

    return ret;
}

/**
 * @brief Опрашивает входы, контролирует изменения.
 * 
 * @return uint32_t ESP_OK - успех, ESP_FAIL - ошибка.
 */
uint32_t get_input()
{
    uint32_t input_bit = 0;
    uint32_t sum_val = 0;

    for (uint32_t i = 0; i < LEN_PINOUT; i++){
        if(gpio_get_level(Pin_out_adr.input_adr[i])){
            input_bit = 1<<Pin_out_adr.input_adr[i];
            sum_val = sum_val + input_bit;
        }
    }
    Pin_out_reg.pin_in = sum_val;

    if (Pin_out_reg.pin_in != Pin_out_reg.pins_out_reg){
        if (!(set_output())){  
            free(create_monitor_with_helpers());
        }
        else{
            return ESP_FAIL;
        }
    }
    Pin_out_reg.pins_out_reg = sum_val;

    return ESP_OK;
}


void app_main(void)
{
    GPIO_Init();

    while (true){ 
        ESP_ERROR_CHECK(get_input()); 
        
        vTaskDelay(10);
    }
}