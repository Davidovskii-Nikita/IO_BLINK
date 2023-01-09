#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include <stdbool.h>
#include <cJSON.h>

#define BIT_MASK(val, mask) ((val) & (1 << (mask)))// макрос возвращает бит mask в значении val

#define LEN_PINOUT 4 // Колличество пар вход-выход в массиве (колличество его элементов).

/**
 * Описывает пару вход-выход.
 *
 */
static struct Pin_out_adress
{
    uint32_t input_adr[LEN_PINOUT];   /*!< Адресс входа. */
    uint32_t output_adr[LEN_PINOUT]; /*!< Адрес выхода. */
}Pin_out_adr={{0,1,2,3},{4,5,6,8}};

/**
 * Хранит биты состояний вход-выход.
 *
 */
struct Pin_out_register{
    uint32_t pin_in;
    uint32_t pins_out_reg;
}Pin_out_reg;


int old_inputs_value = 0; // Хранит прошлое значение "контрольной суммы"

/**
 * @brief Инициализирует выходы/выходы.
 *
 * @return esp_err_t 1 - успех.
 *
 */
uint32_t GPIO_Init()
{
    for (uint32_t i = 0; i < LEN_PINOUT; i++){
        if (gpio_set_direction(Pin_out_adr.output_adr[i], GPIO_MODE_OUTPUT)){
            return 0;
        }
        if (gpio_set_direction(Pin_out_adr.input_adr[i], GPIO_MODE_INPUT)){
            return 0;
        }
    }
    return 1;
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
    pinouts = cJSON_AddArrayToObject(monitor, "pinout");

    for (uint32_t i = 0; i < LEN_PINOUT; i++){
        cJSON *pinout = cJSON_CreateObject();

        cJSON_AddNumberToObject(pinout,"input",Pin_out_adr.input_adr[i]);

        cJSON_AddNumberToObject(pinout,"output",Pin_out_adr.output_adr[i]);

        cJSON_AddNumberToObject(pinout,"state",0);

        cJSON_AddItemToArray(pinouts, pinout);
    }

    string = cJSON_Print(monitor);
    printf(string);
    if (string == NULL){
        printf("Failed to print monitor.\n");
    }
    return string;
}

/**
 * @brief Устанавливает логическое состояние выходов.
 *
 */
uint32_t set_output()
{
    for (uint32_t i = 0; i < LEN_PINOUT; i++){
        if(BIT_MASK(Pin_out_reg.pins_out_reg,Pin_out_adr.input_adr[i])){
            if(gpio_set_level(Pin_out_adr.output_adr[i],1)== -1){
                return 1;
            }
        }
        else{
            if(gpio_set_level(Pin_out_adr.output_adr[i],0)== -1){
                return 1;
            }
        }
    }
    return 0;
}

/**
 *@brief Опрашивает  состояния входов. Сохраняет их в int переменной.
 *
 */
uint32_t get_input()
{
    uint32_t input_bit = 0;
    uint32_t sum_val = 0;
    for (uint32_t i = 0; i < LEN_PINOUT; i++){
        if(gpio_get_level(Pin_out_adr.input_adr[i])!=-1){
            input_bit = 1<<Pin_out_adr.input_adr[i];
            sum_val = sum_val + input_bit;
        }
        else{
            return 1;
        }
    }
    Pin_out_reg.pin_in = sum_val;

    if (Pin_out_reg.pin_in != Pin_out_reg.pins_out_reg){
            if (set_output()){  
            printf("Output error!");
            for (int i = 10; i >= 0; i--) {
                printf("Restarting in %d seconds...\n", i);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            printf("Restarting now.\n");;
            esp_restart();
            }
        free(create_monitor_with_helpers());
    }
    Pin_out_reg.pins_out_reg = sum_val;
    
    return 0;
}


void app_main(void)
{
    if (GPIO_Init()){  
        printf("Initialization error!");

        for (int i = 10; i >= 0; i--) {
            printf("Restarting in %d seconds...\n", i);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        printf("Restarting now.\n");;
        esp_restart();
    }
    else{
        printf("Initialization was successful!");
    }

    while (true){
            
        if (get_input()){  
            printf("Input error!");
            for (int i = 10; i >= 0; i--) {
                printf("Restarting in %d seconds...\n", i);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            printf("Restarting now.\n");;
            esp_restart();
        }
        vTaskDelay(10);
    }
}