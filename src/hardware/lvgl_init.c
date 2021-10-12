
#include "lvgl_init.h"
#include "lvgl.h"
#include "pinetime_board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "spi_master2.h"
#include "nrf_gpio.h"
#include "st7789.h"
#include "cst816.h"
#include "sys.h"
#include "lv_theme_pinetime.h"

#define DISP_HOR_RES 240
#define DISP_VER_RES 240


static uint16_t totalNbLines = 320;
static uint16_t visibleNbLines = 240;

uint16_t writeOffset = 0;
uint16_t scrollOffset = 0;


static void touchpad_read(lv_indev_drv_t* drv, lv_indev_data_t* data);
static void disp_flush(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);

void lvgl_init(void) {

    writeOffset = 0;
    scrollOffset = 0;

    st7789_init();

    lv_init();

    static lv_disp_draw_buf_t draw_buf_dsc;
    static lv_color_t buf_2_1[DISP_HOR_RES * 4];
    static lv_color_t buf_2_2[DISP_HOR_RES * 4];
    lv_disp_draw_buf_init(&draw_buf_dsc, buf_2_1, buf_2_2, DISP_HOR_RES * 4);   // Initialize the display buffer

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

    /*Initialize the new theme from the current theme*/
    lv_theme_t * th_act = lv_disp_get_theme(lv_disp_get_default());
    static lv_theme_t th_pinetime;
    th_pinetime = *th_act;

    /*Set the parent theme ans the style applay callback for the new theme*/
    lv_theme_set_parent(&th_pinetime, th_act);

    lv_theme_pinetime_init(&th_pinetime);

    /*Assign the new theme the the current display*/
    lv_disp_set_theme(lv_disp_get_default(), &th_pinetime);

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

static void touchpad_read(lv_indev_drv_t* drv, lv_indev_data_t* data) {

    cst816Get();

    bool touched = (tsData.event == 2);

    if (!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = tsData.xpos;
        data->point.y = tsData.ypos;
    }

}

static void disp_flush(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p) {


    uint16_t y1, y2, width, height = 0;

    if( (pinetimecos.refreshDirection == Down) && (area->y2 == visibleNbLines - 1)) {
        writeOffset = ((writeOffset + totalNbLines) - visibleNbLines) % totalNbLines;
    } else if( (pinetimecos.refreshDirection == Up) && (area->y1 == 0) ) {
        writeOffset = (writeOffset + visibleNbLines) % totalNbLines;
    }

    y1 = (area->y1 + writeOffset) % totalNbLines;
    y2 = (area->y2 + writeOffset) % totalNbLines;

    width = (area->x2 - area->x1) + 1;
    height = (area->y2 - area->y1) + 1;

    if ( pinetimecos.refreshDirection == Down ) {
        if(area->y2 < visibleNbLines - 1) {
            uint16_t toScroll = 0;
                if(area->y1 == 0) {
                toScroll = height * 2;
                pinetimecos.refreshDirection = None;
                lv_disp_set_direction(lv_disp_get_default(), 0);
            } else {
                toScroll = height;
            }

            if(scrollOffset >= toScroll)
                scrollOffset -= toScroll;
            else {
                toScroll -= scrollOffset;
                scrollOffset = (totalNbLines) - toScroll;
            }
            st7789_vertical_scroll_start_address(scrollOffset);
        }
    } else if(pinetimecos.refreshDirection == Up) {

        if(area->y1 > 0) {
            if(area->y2 == visibleNbLines - 1) {
                scrollOffset += (height * 2);
                pinetimecos.refreshDirection = None;
                lv_disp_set_direction(lv_disp_get_default(), 0);
            } else {
                scrollOffset += height;
            }
            scrollOffset = scrollOffset % totalNbLines;
            st7789_vertical_scroll_start_address(scrollOffset);
        }
    }

    if (y2 < y1) {
        height = totalNbLines - y1;

        if ( height > 0 ) {
            st7789_set_window(area->x1, y1, width, height);
            st7789_send(0, (uint8_t*)(color_p), (width * height * 2));        
        }
        
        uint16_t pixOffset = width * height;

        height = y2 + 1;

        st7789_set_window(area->x1, 0, width, height);
        st7789_send(0, (uint8_t*)(color_p + pixOffset), (width * height * 2));

    } else {
        
        st7789_set_window(area->x1, y1, width, height);
        st7789_send(0, (uint8_t*)(color_p), (width * height * 2));

    }

    lv_disp_flush_ready(disp_drv);
}
