#include <stdio.h>

#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"


#include "mrubyc.h"


#include "driver/uart.h"
#include "driver/gpio.h"


#include "models/led.h"
#include "models/ledto.h"
#include "models/ledtoo.h"
#include "loops/master.h"
#include "loops/masto.h"

// #include "models/[replace with your file].h"
// #include "loops/[replace with your file].h"

#define MEMORY_SIZE (1024*40)

static uint8_t memory_pool[MEMORY_SIZE];

//================================================================
/*! DEBUG PRINT
*/
void chip_info() {
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
}

static void c_debugprint(struct VM *vm, mrbc_value v[], int argc){
  for( int i = 0; i < 79; i++ ) { console_putchar('='); }
  console_putchar('\n');
  chip_info();
  int total, used, free, fragment;
  mrbc_alloc_statistics( &total, &used, &free, &fragment );
  console_printf("Memory total:%d, used:%d, free:%d, fragment:%d\n", total, used, free, fragment );
  unsigned char *key = GET_STRING_ARG(1);
  unsigned char *value = GET_STRING_ARG(2);
  console_printf("%s:%s\n", key, value );
  heap_caps_print_heap_info(MALLOC_CAP_8BIT);
  heap_caps_print_heap_info(MALLOC_CAP_32BIT);
  for( int i = 0; i < 79; i++ ) { console_putchar('='); }
  console_putchar('\n');
}

//#####################################################################//
// GPIO //
static void c_gpio_init_output(mrb_vm *vm, mrb_value *v, int argc) {
  int pin = GET_INT_ARG(1);
  console_printf("init pin %d\n", pin);
  gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

static void c_gpio_set_level(mrb_vm *vm, mrb_value *v, int argc){
  int pin = GET_INT_ARG(1);
  int level = GET_INT_ARG(2);
  gpio_set_level(pin, level);
}
//#####################################################################//
// UART //
#define MY_UART_TXD  (17)
#define MY_UART_RXD  (16)

#define ECHO_TEST_TXD  (17)
#define ECHO_TEST_RXD  (16)
#define BUF_SIZE (1024)

//static int uart_num = UART_NUM_2;
const uart_port_t uart_num = UART_NUM_2;

static void c_get_data(struct VM *vm, mrbc_value v[], int argc){
  
  /*uint8_t command[] = {
    0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79
  };*/
  //uart_write_bytes(uart_num, (const char*)command, 9);
  // Read data from UART.
  //uint8_t data[25];
  
  uint8_t* data = (uint8_t*) malloc(BUF_SIZE+1);
  float length = 0;
  ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, (size_t*)&length));
  
  length = uart_read_bytes(uart_num, data, BUF_SIZE, 1000 / portTICK_RATE_MS);
  
  
  int i;
  mrb_value array = mrbc_array_new( vm, 9 );
  for( i = 0; i < 9; i++ ) {
   //puts("TEST: #{i}");
   //putchar(i);
   
   mrb_value value = mrb_fixnum_value(data[i]);
   //putchar(i);
   mrbc_array_set( &array, i, &value );
  }
 /* if (length > 0) {
    mrb_value data = mrb_str_new(vm, (const char *)data, length);
  }*/
  SET_RETURN(array);
 
}



//#####################################################################//


void app_main(void) {
  // UART //
   uart_config_t uart_config = {
    .baud_rate = 9600,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
  };
  // Configure UART parameters
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  // Set UART pins(TX: IO16 (UART2 default), RX: IO17 (UART2 default), RTS: IO18, CTS: IO19)
  ESP_ERROR_CHECK(uart_set_pin(uart_num, MY_UART_TXD, MY_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  // Setup UART buffered IO with event queue
  const int uart_buffer_size = (1024 * 2);
  QueueHandle_t uart_queue;
  // Install UART driver using an event queue here
  ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

  nvs_flash_init();
  mrbc_init(memory_pool, MEMORY_SIZE);

  mrbc_define_method(0, mrbc_class_object, "debugprint", c_debugprint);

  mrbc_init(memory_pool, MEMORY_SIZE);
  mrbc_define_method(0, mrbc_class_object, "gpio_init_output", c_gpio_init_output);
  mrbc_define_method(0, mrbc_class_object, "gpio_set_level", c_gpio_set_level);
  mrbc_define_method(0, mrbc_class_object, "get_data", c_get_data);
  //mrbc_define_method(0, mrbc_class_object, "get_gps", read_gps_data);
  
  
  mrbc_create_task( led, 0 );
  mrbc_create_task( ledto, 0 );
  mrbc_create_task( ledtoo, 0 );
  mrbc_create_task( master, 0 );
  mrbc_create_task( masto, 0 );
  

  mrbc_run();
}
