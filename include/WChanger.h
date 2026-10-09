#pragma once

#include <Arduino.h>
#include <lvgl.h>

class WChanger
{
    private:
        lv_obj_t* sun;
        lv_obj_t* cloud;
        enum State { STATE_NONE, STATE_SUN, STATE_CLOUD } state = STATE_NONE;

    public:
        WChanger(lv_obj_t* Nsun, lv_obj_t* Ncloud)
        {
            sun = Nsun;
            cloud = Ncloud;
        }
        void showSun()
        {
            if (state == STATE_SUN) return;

            if (cloud) lv_obj_add_flag(cloud, LV_OBJ_FLAG_HIDDEN);
            if (sun) lv_obj_clear_flag(sun, LV_OBJ_FLAG_HIDDEN);
            state = STATE_SUN;
        }
        void showCloud()
        {
            if (state == STATE_CLOUD) return;

            if (sun) lv_obj_add_flag(sun, LV_OBJ_FLAG_HIDDEN);
            if (cloud) lv_obj_clear_flag(cloud, LV_OBJ_FLAG_HIDDEN);
            state = STATE_CLOUD;
        }
};