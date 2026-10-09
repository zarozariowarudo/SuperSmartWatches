#include <Arduino.h>
#include <lvgl.h>

// Подключаем твой конфигурационный файл дисплея
#include "gfx_conf.h"

#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Раскомментируй, если используешь демки LVGL, или подключи свой UI (например, из SquareLine)

#include "ui/ui.h"

#include "LabelUpgrades.h"
#include "WChanger.h"
#include "DisplayManager.h"

#define screenWidth   480
#define screenHeight  320

#define DHTPIN 14
#define DHTTYPE DHT22

const String Internet = "TP-Link_1B4F";
const String password = "33946597";

const String weatherURL = "https://api.open-meteo.com/v1/forecast?latitude=50.00&longitude=36.25&current=temperature_2m,relative_humidity_2m,wind_speed_10m&hourly=temperature_2m,precipitation_probability&timezone=Europe%2FKyiv&forecast_days=4&wind_speed_unit=ms";

DHT dht(DHTPIN, DHTTYPE);
LGFX tft;

DisplayManager displayMgr(&tft, ui_Screen1, 120000);

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
    
    if(touched)
    {
        if (displayMgr.sleeping())
        {
            displayMgr.wakeUp();
            data->state = LV_INDEV_STATE_REL;
            return;
        }

        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    }   

    else
        data->state = LV_INDEV_STATE_REL;
        
        // Serial.printf("TOUCH PRESSED -> Raw X: %d, Raw Y: %d | Sent X: %d, Sent Y: %d\n", 
        //                   touchX, touchY, data->point.x, data->point.y);
}

uint16_t calData[8] = {405, 3823, 319, 255, 3592, 3808, 3591, 318};

void lvgl_init()
{
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
}
void setup()
{
    Serial.begin(115200);
    Serial.println("LVGL initialization start...");

    // Подготовка дисплея
    tft.begin();
    tft.setRotation(3); // Поворот в альбомную ориентацию
    tft.fillScreen(TFT_BLACK);

    tft.setTouchCalibrate(calData);

    lvgl_init();

    // Запуск демо-виджетов или твоего собственного интерфейса
    ui_init();

    WiFi.begin(Internet, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    configTime(3*3600, 0, "pool.ntp.org");

    displayMgr.begin();
  
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

void getForecastData()
{
    //PART 1 ----------------------------------------------------------------------------------------------------
    static unsigned long previousWeatherTime = 0;
    static unsigned long weatherInterval = 10 * 60 * 1000;

    static UpgradeLabel IntTempData(ui_IntTempData);
    static UpgradeLabel IntHumData(ui_IntHumData);
    static UpgradeLabel IntWindData(ui_IntWindData);

    if (millis() - previousWeatherTime >= weatherInterval || previousWeatherTime == 0)
    {
        previousWeatherTime = millis();
        if (WiFi.status() != WL_CONNECTED) 
        {
            IntTempData.update("WiFi Err");
            previousWeatherTime = millis() - weatherInterval + 10000; // повтор через 10 сек
            return;
        }
        WiFiClientSecure client;
        client.setInsecure();

        HTTPClient http;
        http.begin(client, weatherURL);
        int httpCode = http.GET();

        if (httpCode != HTTP_CODE_OK)
        {
            IntHumData.update("HTTP ERR");
            http.end();
            previousWeatherTime = millis() - weatherInterval + 10000; // повтор через 10 сек
            return;
        }

        String data = http.getString();
        http.end();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, data);
        if (error)
        {
            IntWindData.update("Parse Error");
            previousWeatherTime = millis() - weatherInterval + 10000; // повтор через 10 сек
            return;
        }

        // Переводим текущие данные сразу в переменные
        String temp = String(doc["current"]["temperature_2m"].as<float>(), 1) + "*C";
        String humd = String(doc["current"]["relative_humidity_2m"].as<int>()) + "%";
        String wind = String(doc["current"]["wind_speed_10m"].as<float>(), 1);

        IntTempData.update(temp);
        IntHumData.update(humd);
        IntWindData.update(wind);

        // PART 2: ПРОГНОЗ НА 4 ДНЯ -------------------------------------------------------------
        static UpgradeLabel rainLabels[4] = {
            UpgradeLabel(ui_RainDataToday),
            UpgradeLabel(ui_RainDataTommrw),
            UpgradeLabel(ui_RainDataDayAfter),
            UpgradeLabel(ui_RainDataIn3Days)
        };

        static UpgradeLabel tempLabels[4] = {
            UpgradeLabel(ui_TempDataToday),
            UpgradeLabel(ui_TempDataTommrw),
            UpgradeLabel(ui_TempDataDayAfter),
            UpgradeLabel(ui_TempDataIn3Days)
        };

        static WChanger imgChangers[4] = {
            WChanger(ui_TodaySunIcon, ui_TodayCloudIcon),
            WChanger(ui_TommrwSunIcon, ui_TommrwCloudIcon),
            WChanger(ui_DayAfterSunIcon, ui_DayAfterCloudIcon),
            WChanger(ui_In3DaysSunIcon, ui_In3DaysCloudIcon)
        };

        for (int day = 0; day < 4; day++)
        {
            // 1. Температура в диапазоне времени от 7 до 19 часов (дневной максимум)
            float maxDayTemp = -999.0;
            for (int h = 7; h <= 19; h++)
            {
                int idx = day * 24 + h;
                float t = doc["hourly"]["temperature_2m"][idx].as<float>();
                if (t > maxDayTemp)
                {
                    maxDayTemp = t;
                }
            }

            // Форматируем температуру (округление до целого, например, "22°")
            String tempStr = String((int)round(maxDayTemp)) + "°";
            tempLabels[day].update(tempStr);

            // 2. Максимальная вероятность осадков и время дождя
            int maxProb = 0;
            int maxIndex = day * 24;
            for (int h = 0; h < 24; h++)
            {
                int idx = day * 24 + h;
                int prob = doc["hourly"]["precipitation_probability"][idx];
                if (prob > maxProb)
                {
                    maxProb = prob;
                    maxIndex = idx;
                }
            }

            String rainInfo;
            if (maxProb > 10)
            {
                String timeStr = doc["hourly"]["time"][maxIndex].as<String>();
                int tIndex = timeStr.indexOf('T');
                String exactTime = (tIndex >= 0) ? timeStr.substring(tIndex + 1) : timeStr;
                rainInfo = exactTime + " (" + String(maxProb) + "%)";
                imgChangers[day].showCloud();
            }
            else
            {
                rainInfo = "No rain";
                imgChangers[day].showSun();
            }
            rainLabels[day].update(rainInfo);
        }
    }
}

void getHomeTempData()
{
    static unsigned long lastDHTRead = 0;

    static UpgradeLabel HomeTempData(ui_HomeTempData);
    static UpgradeLabel HomeHumData(ui_HomeHumData);

    if (millis() - lastDHTRead <= 15000) return;

    lastDHTRead = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h)) 
    {
        return;
    }

    String nt = String(t, 1) + "*C";
    String nh = String(h, 0) + "%";

    HomeTempData.update(nt);
    HomeHumData.update(nh);
}

void loop()
{
    lv_timer_handler(); 
    update_clock();
    // getForecastData();
    getHomeTempData();
    displayMgr.update();
    delay(5);
}