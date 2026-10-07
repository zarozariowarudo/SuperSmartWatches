#include <Arduino.h>
#include <lvgl.h>

// Подключаем твой конфигурационный файл дисплея
#include "gfx_conf.h"

#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Раскомментируй, если используешь демки LVGL, или подключи свой UI (например, из SquareLine)

#include "ui/ui.h"

#define screenWidth   480
#define screenHeight  320

const String Internet = "TP-Link_1B4F";
const String password = "33946597";

const String weatherURL = "https://api.open-meteo.com/v1/forecast?latitude=50.00&longitude=36.25&current=temperature_2m,relative_humidity_2m,wind_speed_10m&hourly=precipitation_probability&timezone=Europe%2FKyiv&forecast_days=4&wind_speed_unit=ms";

LGFX tft;

static lv_disp_draw_buf_t draw_buf;
// Выделяем буферы памяти для отрисовки
static lv_color_t disp_draw_buf1[screenWidth * screenHeight / 10];
static lv_color_t disp_draw_buf2[screenWidth * screenHeight / 10];
static lv_disp_drv_t disp_drv;

/* Функция вывода графики на экран */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    // Используем быструю отправку по DMA
    tft.pushImageDMA(area->x1, area->y1, w, h, (lgfx::rgb565_t*)&color_p->full);
    lv_disp_flush_ready(disp);
}

/* Функция чтения касаний тачскрина */
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t touchX, touchY;

    tft.waitDMA();

    bool touched = tft.getTouch(&touchX, &touchY);
    
    if(!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        data->state = LV_INDEV_STATE_PR;

        /* Установка координат касания */
        data->point.x = touchX;
        data->point.y = touchY;
    }

    // Serial.printf("TOUCH PRESSED -> Raw X: %d, Raw Y: %d | Sent X: %d, Sent Y: %d\n", 
    //                   touchX, touchY, data->point.x, data->point.y);
}

uint16_t calData[8] = {405, 3823, 319, 255, 3592, 3808, 3591, 318};

void setup()
{
    Serial.begin(115200);
    Serial.println("LVGL initialization start...");

    // Подготовка дисплея
    tft.begin();
    tft.setRotation(3); // Поворот в альбомную ориентацию
    tft.fillScreen(TFT_BLACK);

    tft.setTouchCalibrate(calData);

    lv_init();

    // Инициализация буфера отрисовки
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf1, disp_draw_buf2, screenWidth * screenHeight / 10);
    
    // Инициализация драйвера дисплея
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Инициализация устройства ввода (тачскрина)
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // Запуск демо-виджетов или твоего собственного интерфейса
    ui_init();

    WiFi.begin(Internet, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    configTime(3*3600, 0, "pool.ntp.org");
  
    Serial.println("Setup done");
}

void update_clock() {
    static uint32_t last_check_Time  = 0;

    if (millis() - last_check_Time > 1000) {
        last_check_Time = millis();
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            char timeStr[8];
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
            
            // Записываем время в виджет Label
            lv_label_set_text(ui_TimeLabel, timeStr); 
        }
    }
}



void loop()
{
    lv_timer_handler(); 
    update_clock();
    delay(5);
}