
#include "lvgl_init.h"
#include "lvgl.h"
#include "pinetime_board.h"
#include "spi_master2.h"
#include "nrf_gpio.h"
#include "st7789.h"
#include "cst816.h"

#define DISP_HOR_RES 240
#define DISP_VER_RES 240

static void disp_flush(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);
static void touchpad_read(lv_indev_drv_t* drv, lv_indev_data_t* data);

void lvgl_init(void) {

    st7789_init();

    lv_init();

    static lv_disp_draw_buf_t draw_buf_dsc;
    static lv_color_t buf_2_1[DISP_HOR_RES * 4];
    //static lv_color_t buf_2_2[DISP_HOR_RES * 4];
    lv_disp_draw_buf_init(&draw_buf_dsc, buf_2_1, NULL, DISP_HOR_RES * 4);   // Initialize the display buffer

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);    // Basic initialization

    // Set the resolution of the display
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;

    // Used to copy the buffer's content to the display
    disp_drv.flush_cb = disp_flush;

    // Set a display buffer
    disp_drv.draw_buf = &draw_buf_dsc;

    // Finally register the driver
    lv_disp_drv_register(&disp_drv);

    //

    static lv_indev_drv_t indev_drv;
    // Initialize your touchpad if you have
    cst816Init();

    // Register a touchpad input device
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);

}

static void disp_flush(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p) {

    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    st7789_set_window(area->x1, area->y1, w, h);

    st7789_send(0, (uint8_t*)(color_p), (w * h * 2));

    lv_disp_flush_ready(disp_drv);
}


static void touchpad_read(lv_indev_drv_t* drv, lv_indev_data_t* data) {

    cst816Get();

    data->state = (tsData.event == 2) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;

    /*Set the last pressed coordinates*/
    data->point.x = tsData.xpos;
    data->point.y = tsData.ypos;

}