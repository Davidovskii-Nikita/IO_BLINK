#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include <stdbool.h>
#include <cJSON.h>


int LEN_PINOUT = 0;// Колличество пар вход-выход в массиве (колличество его элементов).

/**
 * Описывает пару вход-выход, определяет состояние пары. 
 * 
 */
typedef struct pin_out
{
    int input_adr; /*!< Адресс входа. */
    int output_adr; /*!< Адрес выхода. */
    bool bin_state /*!< Состояние выхода. */
} pin_out;

int old_inputs_value = 0; // Хранит прошлое значение "контрольной суммы"


/**
 * @brief Создает и принтует JSON строку.
 * 
 * @param p указатель на массив структур.
 * @return string указатель на строку в куче, которую необходимо освободить
 * 
 */

char *create_monitor_with_helpers(struct pin_out *p)
{
    char *string = NULL;
    cJSON *pinouts = NULL;

    cJSON *monitor = cJSON_CreateObject();
    pinouts = cJSON_AddArrayToObject(monitor, "pinout");

    for (int i=0 ; i < LEN_PINOUT;i++)
    {
        cJSON *pinout = cJSON_CreateObject();

        cJSON_AddNumberToObject(pinout,"input",p->input_adr);

        cJSON_AddNumberToObject(pinout,"output",p->output_adr);

        cJSON_AddNumberToObject(pinout,"state",p->bin_state);

        cJSON_AddItemToArray(pinouts, pinout);
        p +=1;
    }

    string = cJSON_Print(monitor);
    printf(string);
    if (string == NULL)
    {
        printf("Failed to print monitor.\n");
    }
    return string;
}

/**
 * @brief Определяет выходы, устанавливает их логическое состояние.
 * 
 * @param p указатель на массив структур.
 * @param inputs_value "контрольная сумма".
 * 
 */
void define_and_set_output(struct pin_out *p, int inputs_value)
{
    for (int i = 0; i < LEN_PINOUT; i++)
    {
         gpio_set_direction(p->output_adr, GPIO_MODE_OUTPUT); 
        if (inputs_value % 2)
        {
            p->bin_state = true;
            gpio_set_level(p->output_adr, 1);
        }
        else
        {
            p->bin_state = false;
            gpio_set_level(p->output_adr, 0);
        }
        inputs_value = inputs_value / 2;
        p -= 1;
    }
}

/**
 *@brief Определяет состояния входов, опрашивает их. Переводит стостояния входов в 10-чное число inputs_value ("контрольная сумма")
 *
 *@param p указатель на массив структур.
 *       num_of_element Колличество пар вход-выход в массиве (колличество его элементов).
*/
void define_and_get_input(struct pin_out *p , int num_of_element)
{
    extern int LEN_PINOUT;
    LEN_PINOUT = num_of_element;

    int inputs_value = 0;
    for (int i = 0; i < LEN_PINOUT; i++) 
    {   
        gpio_set_direction(p->input_adr, GPIO_MODE_INPUT);
        p->bin_state  = gpio_get_level(p->input_adr);
        inputs_value = inputs_value * 2 + p->bin_state;
        p += 1;
    }
    if (old_inputs_value != inputs_value)
    {
        define_and_set_output(p -= 1, inputs_value);
        free(create_monitor_with_helpers(p -= LEN_PINOUT-1));
    }
    old_inputs_value = inputs_value;
}


void app_main(void){

    pin_out test_pin[] = 
    {
    {2, 14, false},
    {13, 25, false},
    {4, 16, false},
    {9, 5, false}

    };

    int number_of_elements = 4;

    while(true){
        define_and_get_input(&test_pin, number_of_elements );
        vTaskDelay(10);
    }


}