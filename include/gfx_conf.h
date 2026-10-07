#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9488     _panel_instance;
    lgfx::Bus_SPI           _bus_instance;
    lgfx::Light_PWM         _light_instance;
    lgfx::Touch_XPT2046     _touch_instance;

public:
    LGFX(void) {
        // 1. Настройка шины SPI для экрана
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host   = VSPI_HOST; 
            cfg.spi_mode   = 0;
            cfg.freq_write = 40000000; // 40 МГц
            cfg.freq_read  = 16000000;
            cfg.spi_3wire  = false;
            cfg.use_lock   = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            
            cfg.pin_sclk = 18; // TFT_SCLK
            cfg.pin_mosi = 23; // TFT_MOSI
            cfg.pin_miso = 19; // TFT_MISO
            cfg.pin_dc   = 2;  // TFT_DC

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // 2. Настройка панели ILI9488
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = 15; // TFT_CS
            cfg.pin_rst          = 4;  // TFT_RST
            cfg.pin_busy         = -1;
            cfg.panel_width      = 320;
            cfg.panel_height     = 480;
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;
            cfg.offset_rotation  = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;
            cfg.readable         = true;
            cfg.invert           = false; // Если цвета инвертированы - поставь true
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false; // Для ILI9488 по SPI это важно!
            cfg.bus_shared       = true;

            _panel_instance.config(cfg);
        }

        // 3. Настройка подсветки
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl      = 27; // TFT_BL
            cfg.invert      = false;
            cfg.freq        = 44100;
            cfg.pwm_channel = 7;

            _light_instance.config(cfg);
            _panel_instance.light(&_light_instance);
        }

        // 4. Настройка тачскрина XPT2046
        {
            auto cfg = _touch_instance.config();
            cfg.x_min      = 417;  // Поменяли местами (было 417)
            cfg.x_max      = 3293;   // Поменяли местами (было 3277)
            cfg.y_min      = 398;   
            cfg.y_max      = 3104; 

            cfg.pin_int    = -1;    
            cfg.bus_shared = true;  
            cfg.offset_rotation = 0;

            cfg.spi_host   = VSPI_HOST;
            cfg.freq       = 1000000;   
            cfg.pin_sclk   = 18;
            cfg.pin_mosi   = 23;
            cfg.pin_miso   = 19;
            cfg.pin_cs     = 21; // TOUCH_CS

            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};