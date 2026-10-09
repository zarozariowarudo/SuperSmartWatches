#pragma once
#include <Arduino.h>
#include <lvgl.h>

class UpgradeLabel
{
    private:
        String previous = "";
        lv_obj_t* label;
    public:
        UpgradeLabel(lv_obj_t* Nlabel)
        {
            label = Nlabel;
        }
        void update(const String &Ndata)
        {
            if (!label) return;

            if (previous != Ndata)
            {
                lv_label_set_text(label, Ndata.c_str());
                previous = Ndata;
            }
        }
};