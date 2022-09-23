
/**
 * @file app_main.c
 * @brief Example application for the LCD1602 16x2 Character Dot Matrix LCD display via I2C backpack..
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "rom/uart.h"

#include "smbus.h"
#include "i2c-lcd1602.h"
#include <string.h>
#include <math.h>
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/adc.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"

#define onboardLED 2

#define TAG "app"
#define GPIO_OUTPUT_SEL  ((1ULL<<GPIO_BUTTON_0) | (1ULL<<GPIO_BUTTON_1) |  (1ULL<<GPIO_BUTTON_2))

float conversion;
static xQueueHandle duty_queue = NULL;
// LCD1602
#define LCD_NUM_ROWS               2
#define LCD_NUM_COLUMNS            32
#define LCD_NUM_VISIBLE_COLUMNS    16

// LCD2004
//#define LCD_NUM_ROWS               4
//#define LCD_NUM_COLUMNS            40
//#define LCD_NUM_VISIBLE_COLUMNS    20

// Undefine USE_STDIN if no stdin is available (e.g. no USB UART) - a fixed delay will occur instead of a wait for a keypress.
#define USE_STDIN  1
//#undef USE_STDIN


#define I2C_MASTER_NUM           I2C_NUM_0
#define I2C_MASTER_TX_BUF_LEN    0                     // disabled
#define I2C_MASTER_RX_BUF_LEN    0                     // disabled
#define I2C_MASTER_FREQ_HZ       100000
#define I2C_MASTER_SDA_IO        CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_SCL_IO        CONFIG_I2C_MASTER_SCL
#define Mateo 0
static void i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;  // GY-2561 provides 10kΩ pullups
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;  // GY-2561 provides 10kΩ pullups
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_MASTER_RX_BUF_LEN,
                       I2C_MASTER_TX_BUF_LEN, 0);
}

// uart_rx_one_char_block() causes a watchdog trigger, so use the non-blocking
// uart_rx_one_char() and delay briefly to reset the watchdog.


void CurrentSensorTask(void * pvParameter)
{

   //   float current_rate [2001] ;
    float conversion;
    float voltage;
  //  float AverageRateCurrent=0;
  //  int icon = 0;

while(1){
   vTaskDelay(100/portTICK_PERIOD_MS);
   int adc = adc1_get_raw(ADC1_CHANNEL_6); //Get the ADC reading
     voltage = (adc * 3.3)/4095; 
 conversion = (voltage-2.5)/0.066;
    if (conversion < 0.0) {
      conversion = 0.0;
    }
    
xQueueSendToBack(duty_queue,&conversion,(TickType_t)10);
}

}


void setADC()
{
    adc1_config_width(ADC_WIDTH_BIT_12);                         //Width of 12 bits, so we range from 0 to 4096
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);  //Attenuation to be able to read higher voltages

       adc1_config_width(ADC_WIDTH_BIT_12);                         //Width of 12 bits, so we range from 0 to 4096
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);  //Attenuation to be able to read higher voltages

       adc1_config_width(ADC_WIDTH_BIT_12);                         //Width of 12 bits, so we range from 0 to 4096
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);  //Attenuation to be able to read higher voltages

       adc1_config_width(ADC_WIDTH_BIT_12);                         //Width of 12 bits, so we range from 0 to 4096
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);  //Attenuation to be able to read higher voltages
}


void lcd1602_task(void * pvParameter)
{
    // Set up I2C
    i2c_master_init();
    i2c_port_t i2c_num = I2C_MASTER_NUM;
    uint8_t address = CONFIG_LCD1602_I2C_ADDRESS;

    // Set up the SMBus
    smbus_info_t * smbus_info = smbus_malloc();
    ESP_ERROR_CHECK(smbus_init(smbus_info, i2c_num, address));
    ESP_ERROR_CHECK(smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS));

    // Set up the LCD1602 device with backlight off
    i2c_lcd1602_info_t * lcd_info = i2c_lcd1602_malloc();
    ESP_ERROR_CHECK(i2c_lcd1602_init(lcd_info, smbus_info, true,
                                     LCD_NUM_ROWS, LCD_NUM_COLUMNS, LCD_NUM_VISIBLE_COLUMNS));

    ESP_ERROR_CHECK(i2c_lcd1602_reset(lcd_info));

    // turn on backlight
    ESP_LOGI(TAG, "backlight on");
    i2c_lcd1602_set_backlight(lcd_info, true);

    ESP_LOGI(TAG, "cursor on");
 
    i2c_lcd1602_set_cursor(lcd_info, true);
    ESP_LOGI(TAG, "display special characters");
   

    ESP_LOGI(TAG, "display all characters (loop)");
    i2c_lcd1602_clear(lcd_info);
    i2c_lcd1602_set_cursor(lcd_info, false);
  
        char ans[50];
    
        i2c_lcd1602_set_auto_scroll(lcd_info,false);
  while (1)
  {
    vTaskDelay(500/portTICK_PERIOD_MS);
   
    //settings for  lcd info
  printf("Update Screen\n");
        i2c_lcd1602_home(lcd_info);
  //transfor double into strings
        sprintf(ans, "I|Sensor = %.3f", conversion);
       
        i2c_lcd1602_write_string(lcd_info,ans);
//wait 100 tick
 

  }
}

void setGPIO()
{
    gpio_pad_select_gpio(onboardLED);
    gpio_set_direction(onboardLED, GPIO_MODE_OUTPUT);
}

void task(void * pvParameter) // testing led light and parallel workloads
{
  int adc;
  float voltage;
  float Read[100];
  int Addition = 0;
  while(1){
        vTaskDelay(10 / portTICK_RATE_MS);
Addition++;
  adc = adc1_get_raw(ADC1_CHANNEL_6); //Get the ADC reading
     voltage = (adc * 3.3)/4095; 

     if(Addition<100){
Read[Addition]= voltage;
     }
     else if (Addition== 100){
      Addition=0;
      GPIO.out ^= BIT2;
      for(int JRE=0;JRE <100; JRE++){
        conversion = conversion+ Read[JRE];
     
      }
      conversion = conversion / 100;
      conversion = (conversion-2.5) * .066;
     }


//printf("Current Sensor Voltage: %.2f\n", conversion);
  }
}

void app_main()
{
    setGPIO();
    setADC();
   // xTaskCreatePinnedToCore(&lcd1602_task, "lcd1602_task", 4096, NULL, 10, NULL,NULL,NULL);
     //  xTaskCreatePinnedToCore(&CurrentSensorTask, "CurrentSensor_task", 4096, NULL, 5, NULL,0);
    xTaskCreatePinnedToCore(&lcd1602_task, "lcd1602_task", 4096, NULL, 5, NULL,0);
   xTaskCreatePinnedToCore(&task, "LEd", 4096, NULL, 10, NULL,1);



}
