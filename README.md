| Supported Targets | ESP32 SOLO1 V1 | 
| ----------------- | -------------- |

# Example

Этот пример создан с использованием фреймфорка ESP-IDF, в редакторе VScode (Ubuntu 22.04).
Для прошивания чипа использовался USB-TTL переходник (ch341 driver). Для корректной работы следует удалить компонент brltty.


## How to Use Example

Перед сборкой проекта необходимо настроить `idf.py menuconfig` для работы на 1 ядре с 4 мб памяти.

### Hardware Required

В качестве примера создается статическая структура массивов:
```
static struct 
{
    uint32_t input_adr[LEN_PINOUT];   /*!< Адресс входа. */
    uint32_t output_adr[LEN_PINOUT]; /*!< Адрес выхода. */
}Pin_out_adr={{2,13,4,9},{14,25,16,5}};
```

|  uint32_t input_adr | uint32_t output_adr |
| ------------------- | ------------------- |
|      GPIO2          |      GPIO14         |
|      GPIO13         |      GPIO25         |
|      GPIO4          |      GPIO16         |
|      GPIO9          |      GPIO5          |

Значения состояния входов и выходов содержится в структуре:
```
struct {
    uint32_t pin_in;
    uint32_t pins_out_reg;
}Pin_out_reg;
```


### Build and Flash

`idf.py -p PORT flash monitor` для сборки и отладки через serial port.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

При изменении сконфигурированных входов, в monitor передается JSON строка типа:

```text
{
	"pinout_v2":	[{
			"input":	2,
			"output":	14,
			"state":	0
		}, {
			"input":	13,
			"output":	25,
			"state":	1
		}, {
			"input":	4,
			"output":	16,
			"state":	0
		}, {
			"input":	9,
			"output":	5,
			"state":	1
		}]

}
```


## Troubleshooting

* Проверить надежность посадки/пайки проводов.
* В случае неккоректной работы драйвера, удалить из системы компонент `brltty`
