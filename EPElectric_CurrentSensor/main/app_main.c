
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
#include "sntp.h"
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

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "mbedtls/md5.h"


#define onboardLED 2
#define GPIO_BUTTON_0 5
#define GPIO_BUTTON_1 22
#define GPIO_BUTTON_2 23


#define TAG "app"
#define GPIO_OUTPUT_SEL  ((1ULL<<GPIO_BUTTON_0) | (1ULL<<GPIO_BUTTON_1) |  (1ULL<<GPIO_BUTTON_2))

static float conversion;
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
#define EXAMPLE_ESP_WIFI_SSID "EPElectric"
#define EXAMPLE_ESP_WIFI_PASS "EPElectric"
#define EXAMPLE_MAX_STA_CONN 5
static xQueueHandle duty_queue = NULL;

static EventGroupHandle_t s_wifi_event_group;

#define GPIO_OUTPUT_SEL  ((1ULL<<GPIO_BUTTON_0) | (1ULL<<GPIO_BUTTON_1) |  (1ULL<<GPIO_BUTTON_2))
#define GPIO_INPUT_IO_0     CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1     CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0



#define I2C_MASTER_NUM           I2C_NUM_0
#define I2C_MASTER_TX_BUF_LEN    0                     // disabled
#define I2C_MASTER_RX_BUF_LEN    0                     // disabled
#define I2C_MASTER_FREQ_HZ       100000
#define I2C_MASTER_SDA_IO        CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_SCL_IO        CONFIG_I2C_MASTER_SCL
static float percent;

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_txt_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\n";
static char http_index_hml[] =  R"=====(
<!DOCTYPE html>
<html>
    <head>
        <meta
            charset="UTF-8"
            name="viewport"
            content="width = device-width initial-scale = 1.0"
        />
        <title>EP ELECTRIC EVTOG </title>
    <body style="background-color: #6495ED ;">
        <div class="header"><h1>EP Electric EV2G</h1></div>
        <input
            class="button"
            id="btn0"
            type="button"
            value="Grid/Charge"
            onclick="sendRequestLed()"
        />
        <input
            class="button"
            id="btn1"
            type="button"
            value="Battery %"
            onclick="sendRequestData()"
        />
         <input
            class="button"
            id="btn2"
            type="button"
            value="First HouseGrid "
            onclick="TurnOnGridFirstHouse()"
        />
         <input
            class="button"
            id="btn3"
            type="button"
            value="Second HouseGrid "
            onclick="TurnOnGridSecondHouse()"
        />
        <div class="sensorVal">
            <p>Sensor Value:</p>
            <p id="sen"></p>
        </div>
        <style>
            * {
                margin: 0;
                padding: 1;
            }
            body {
                background-color: #d4dce2;
            }
            .button {
                border: none;
  color: white;
  padding: 20px 30px;
  width: 38%;
  height: 20%;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 26px;
  margin: 30px 20px;
  transition-duration: 0.4s;
  cursor: pointer;
  background-color: #e27217;
}
            .header {
                width: 100%;
                height: 150px;
                color: white;
                background-color: #6495ED;
                padding: 1;
                text-align: center;
            }
            .header h1 {
                color: white;
                vertical-align: center;
                font-size: 52px;
            }
            .btn {
                margin: 0;
                margin-top: 0.5%;
                background-color: #fb9541;
                width: 100%;
                border: none;
                color: white;
                text-align: center;
                text-decoration: none;
                font-size: 16px;
    
            }
            .btn:hover {
                cursor: pointer;
                background-color: #e27217;
            }
            .sensorVal {
                margin: 0;
                margin-top: 0.5%;
                width: 100%;
                height: 700px;
                color: white;
                background-color: #6495ED;
                padding: 0;
                text-align: center;
            }
            .sensorVal p {
                color: white;
                vertical-align: center;
                font-size: 38px;
            }
        </style>
        <script>
            function changeButton(value) { 
                var btn = document.getElementById("btn0"); 
                if(value === "0"){ 
                    btn.value = "Powering Grid"; 
                } else { 
                    btn.value = "Charging Battery"; 
                } 
            } 
              function SecondchangeButton(value) { 
                var btn = document.getElementById("btn2"); 
                if(value === "2"){ 
                    btn.value = "House 1 Off"; 
                } else { 
                    btn.value = "House 1 On"; 
                } 
            } 
                function ThirdchangeButton(value) { 
                            var btn = document.getElementById("btn3"); 
                            if(value === "3"){ 
                                btn.value = "House 2 Off"; 
                            } else { 
                                btn.value = "House 2 On"; 
                            } 
                        } 
                        
            
            function sendRequestLed(){ 
                var http = new XMLHttpRequest(); 
                http.onreadystatechange = (()=>{
                    if(http.readyState === 4){ 
                        if(http.status === 200){ 
                            changeButton(http.responseText); 
                        } 
                    } 
                });
               http.open("GET", "0", true); 
               http.send(); 
            } 
            
            function sendRequestData(){ 
                var http = new XMLHttpRequest();
                http.onreadystatechange = (()=>{ 
                    if(http.readyState === 4){ 
                        if(http.status === 200){
                            document.getElementById("sen").innerHTML = http.responseText; 
                        } 
                    } 
                }); 
                http.open("GET", "1", true);
                http.send(); } 
                
                 function TurnOnGridFirstHouse(){ 
                var http = new XMLHttpRequest(); 
                http.onreadystatechange = (()=>{
                    if(http.readyState === 4){ 
                        if(http.status === 200){ 
                            SecondchangeButton(http.responseText); 
                        } 
                    } 
                });
               http.open("GET", "2", true); 
               http.send(); 
            } 
            function TurnOnGridSecondHouse(){ 
                var http = new XMLHttpRequest(); 
                http.onreadystatechange = (()=>{
                    if(http.readyState === 4){ 
                        if(http.status === 200){ 
                            ThirdchangeButton(http.responseText); 
                        } 
                    } 
                });
               http.open("GET", "3", true); 
               http.send(); 
            } 
        </script>
    </body>
</html>
)=====";




///////////////////////////////////
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
    i2c_lcd1602_set_cursor(lcd_info, true);
        char ans[50];
    char second[50];
        i2c_lcd1602_set_auto_scroll(lcd_info,false);
  while (1)
  {
 
    vTaskDelay(100/portTICK_PERIOD_MS);
    //settings for  lcd info
  //printf("Update Screen\n");
        i2c_lcd1602_home(lcd_info);
  //transfor double into striVngs
        sprintf(ans, "AC sensor: %.2fA", conversion);
        
       //@23.5= 3.29919 @0.0 = 0 @5.04 = 0.884 @12.00 = 2.254 @17.93 = 3.2991
        i2c_lcd1602_write_string(lcd_info,ans);
         sprintf(ans, "Battery: %.3f V", percent);
       i2c_lcd1602_move_cursor(lcd_info,0,1);
       i2c_lcd1602_write_string(lcd_info,ans);
//wait 100 tick
 

  }
}

void setGPIO()
{   
   gpio_pad_select_gpio(onboardLED);
    gpio_set_direction(onboardLED, GPIO_MODE_OUTPUT);

gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = GPIO_OUTPUT_SEL;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config(&io_conf);

}




void task(void * pvParameter) // testing led light and parallel workloads
{
  int adc;
    float TestCurrent [60];
  float vref = 4096.0;
  float unitvalue  = (5.0/vref) *1000.0;
  float voltage;
  float current;
  float converse=0;
  int Addition = 0;
    float addVoltage=0;
    int VoltageSensor;
    float VSensor = 0;
    int ACvoltage;
    float ACsensorVoltage;
  while(1){
    ACvoltage =  adc1_get_raw(ADC1_CHANNEL_5);
    ACsensorVoltage = (ACvoltage*3.3)/4096;

    //testing voltage  sensor


    ////////Getting  current sensor values/////
  vTaskDelay(100/portTICK_PERIOD_MS);
////v sensor
  VoltageSensor =  adc1_get_raw(ADC1_CHANNEL_6);//get ADC value current sensor p34
    VSensor = (VoltageSensor  * 3.3)/4096;

    addVoltage = addVoltage+VSensor;
/////V sensor

  adc = adc1_get_raw(ADC1_CHANNEL_7); //Get the ADC reading current sensor p35
     voltage = (adc * 3.3)/4096;
     current = (voltage-2.495)/0.066;
    if(current<0.1){
        current=0;
    }

    TestCurrent[Addition]= current;
     converse= converse + current;
     Addition++;

     if(Addition == 60){ //sample and get average  
        //ffind largest number
     // storing the largest number at arr[0]
        for (int i = 0; i < 60; ++i) {
        if (TestCurrent[0] < TestCurrent[i]) {
            TestCurrent[0] = TestCurrent[i];
        }
        }
     addVoltage = addVoltage/60;
     percent = addVoltage/0.19025;
      conversion = TestCurrent[0];
      converse = conversion;
      Addition = 0;
      addVoltage= 0;
  //  printf("Current Sensor Voltage: %.2f\nRaw Value: %d \n Current Value: %.3f\n \nVoltage Sensor %.3fV\n ", voltage,adc,conversion,percent);
     }

     /////////Current sensor value
     
     
     
     }

  }



void wifi_init_softap()
{
    s_wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(NULL, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}




void onURL(struct netconn *conn, char command)
{

    if (command == '0') //If BTN0 is pressed
    {
        GPIO.out ^= BIT2;
        	
        if((GPIO.out & BIT2) == BIT2) //If the ONBOARD LED is On, send a 1 to the webpage
        {
            gpio_set_level(GPIO_BUTTON_0, 1);
            printf("LED Current level: 1\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "1", 1, NETCONN_NOCOPY);
        }
        else //If the ONBOARD LED is Off, send a 0 to the webpage
        {
            	gpio_set_level(GPIO_BUTTON_0, 0);
            printf("LED Current level: 0\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "0", 1, NETCONN_NOCOPY);
        }
        
    }
    else if (command == '1') //If the get data button is pressed
    {
   

        char ans[50];                                //String that we're going to send back to the webpage
        memset(ans, 0, sizeof(ans));
if(percent ==40){
           sprintf(ans," Battery : %.2f\n Low Power Charge Now\n", percent);
        netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
        netconn_write(conn, ans, sizeof(ans), NETCONN_NOCOPY);
}
else{
        sprintf(ans,"Battery Percent %.2f ", percent);
        netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
        netconn_write(conn, ans, sizeof(ans), NETCONN_NOCOPY);
}
    }

    else if(command == '3'){
           GPIO.out ^= BIT22;
        	
        if((GPIO.out & BIT22) == BIT22){
            gpio_set_level(GPIO_BUTTON_1, 1);
            printf("LED2 Current level: 1\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "2", 1, NETCONN_NOCOPY); 
            }
            else{
            gpio_set_level(GPIO_BUTTON_1, 0);
            printf("LED2 Current level: 0\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "0", 1, NETCONN_NOCOPY);}

        }

    else if(command == '2'){
          
            GPIO.out ^= BIT23;
        	
        if((GPIO.out & BIT23) == BIT23){
            gpio_set_level(GPIO_BUTTON_2, 1);
            printf("LED3 Current level: 1\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "3", 1, NETCONN_NOCOPY); 
            }
            else{
            gpio_set_level(GPIO_BUTTON_2, 0);
            printf("LED3 Current level: 0\n");
            netconn_write(conn, http_txt_hdr, sizeof(http_txt_hdr) - 1, NETCONN_NOCOPY);
            netconn_write(conn, "0", 1, NETCONN_NOCOPY);}

    }

    else
    {   
        netconn_write(conn, http_html_hdr, sizeof(http_html_hdr) - 1, NETCONN_NOCOPY);
        netconn_write(conn, http_index_hml, sizeof(http_index_hml) - 1, NETCONN_NOCOPY);
        
    }
}

static void http_server_netconn_serve(struct netconn *conn)
{
    struct netbuf *inbuf;
    char *buf;
    u16_t buflen;
    err_t err;
    /* Read the data from the port, blocking if nothing yet there.
 We assume the request (the part we care about) is in one netbuf */
    err = netconn_recv(conn, &inbuf);
    if (err == ERR_OK)
    {
        netbuf_data(inbuf, (void **)&buf, &buflen);
        /* Is this an HTTP GET command? (only check the first 5 chars, since
 there are other formats for GET, and we're keeping it very simple )*/
        if (buflen >= 5 &&
            buf[0] == 'G' &&
            buf[1] == 'E' &&
            buf[2] == 'T' &&
            buf[3] == ' ' &&
            buf[4] == '/')
        {
            /* Send the HTML header
 * subtract 1 from the size, since we dont send the \0 in the string
 * NETCONN_NOCOPY: our data is const static, so no need to copy it
 */
            onURL(conn, buf[5]);
        }
    }
    netconn_close(conn);
    netbuf_delete(inbuf);
}


static void http_server(void *pvParameters)
{
    struct netconn *conn, *newconn;
    err_t err;
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, 80);
    netconn_listen(conn);
    do
    {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK)
        {
            http_server_netconn_serve(newconn);
            netconn_delete(newconn);
        }
    } while (err == ERR_OK);
    netconn_close(conn);
    netconn_delete(conn);
}



void Battery(){

    percent = percent/ 13.4;
    percent = percent *100;
}





///////////////////////////main//////////////////////////////////////////////////////
void app_main()
{

ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs_image",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };


    



  nvs_flash_init();
  wifi_init_softap();
  setGPIO();
  setADC();



xTaskCreatePinnedToCore(&http_server, "http_server", 2048, NULL, 5, NULL,0);
xTaskCreatePinnedToCore(&lcd1602_task, "lcd1602_task", 4096, NULL, 5, NULL,0);
xTaskCreatePinnedToCore(&task, "LEd", 4096, NULL, 10, NULL,1);

}
