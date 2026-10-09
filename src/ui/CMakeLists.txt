SET(SOURCES ui_Screen1.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_thermomether_png.c
    ui_img_forecast_icon_png.c
    ui_img_sunicon_png.c
    ui_img_cloudicon_png.c)

add_library(ui ${SOURCES})
