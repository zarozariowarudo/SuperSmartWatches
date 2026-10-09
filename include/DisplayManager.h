#pragma once
#include <Arduino.h>
#include <lvgl.h>
#include <gfx_conf.h>

class DisplayManager
{
    private:
        LGFX* gfx;
        uint32_t timeoutMs;
        bool sleep = false;
        lv_obj_t* mainScreen;
        uint8_t brightness;

    public:
        DisplayManager(LGFX* gfxInst, lv_obj_t* main_scr, 
            uint32_t timeout_ms, uint8_t Brightness=255)
        {
            gfx = gfxInst;
            mainScreen = main_scr;
            timeoutMs = timeout_ms;
            brightness = Brightness;
        }

        void begin()
        {
            gfx->setBrightness(brightness);
        }

        void update()
        {
            if (!sleep && lv_disp_get_inactive_time(NULL) > timeoutMs)
                goToSleep();
        }

        void goToSleep()
        {
            if (sleep) return;

            gfx->setBrightness(0);
            delay(20);
            gfx->sleep();
            sleep = true;
        }
        void wakeUp()
        {
            if (!sleep) return;

            gfx->wakeup();
            delay(100);
            gfx->setBrightness(brightness);
            lv_disp_trig_activity(NULL);

            resetToMainScreen();

            sleep = false;
        }

        void resetToMainScreen()
        {
            lv_obj_add_flag(ui_PanelTherm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ui_PanelForecast, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(ui_PanelMain, LV_OBJ_FLAG_HIDDEN);
        }

        bool sleeping()
        {
            return sleep;
        }
};