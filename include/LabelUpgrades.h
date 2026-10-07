#pragma once
#include <Arduino.h>
#include <lvgl.h>

class UpgradeLabel
{
    private:
        String previous = "";
    public:
        void update(const String &Ndata, lv_obj_t* label)
        {
            if (!label) return;

            if (previous != Ndata)
            {
                lv_label_set_text(label, Ndata.c_str());
                previous = Ndata;
            }
        }
};