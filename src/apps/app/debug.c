#include <stdint.h>
#include <stddef.h>
#include "debug.h"
#include "app.h"
#include "sys.h"
#include "utils.h"
#include "cst816.h"
#include "lvgl.h"
#include "nrf_ble.h"


static const app_spec_t debug_spec;

debug_app_t debug_app = {
    .app = {.spec = &debug_spec }
};

static inline debug_app_t *_from_app(app_t *app) {
    return container_of(app, debug_app_t, app);
}

lv_mem_monitor_t mon;

static void vTaskStats( app_t *app );

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        delete_bonds();
        load_application(Menu, AnimUp);
    }
}

static lv_obj_t *screen_create(debug_app_t *ht, lv_obj_t * parent) {

    lv_obj_t *scr = lv_obj_create(parent);

    lv_obj_clear_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(scr);                            /*Make it transparent*/
    lv_obj_set_size(scr, lv_pct(100), lv_pct(100));
    //lv_obj_set_scroll_snap_y(scr, LV_SCROLL_SNAP_CENTER);    /*Snap the children to the center*/


    //lv_obj_set_style_bg_color(scr, lv_color_hex(0x900000), 0);

    
    lv_mem_monitor(&mon);

    lv_obj_t * lv_demo = lv_label_create(scr);    
    ht->lv_demo = lv_demo;

    lv_label_set_text_static(ht->lv_demo, "Debug..." );
    lv_obj_set_style_text_align(ht->lv_demo, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(ht->lv_demo, lv_color_hex(0xffffff), 0);
    lv_obj_align(ht->lv_demo, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t * label;

    lv_obj_t * btn1 = lv_btn_create(scr);
    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_MID, 0, 0);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Del Bounds");
    lv_obj_center(label);


    /*lv_obj_t * lv_table = lv_table_create(scr);
    ht->lv_table = lv_table;
    lv_obj_align(ht->lv_table, LV_ALIGN_TOP_MID, 0, 0);
    //lv_obj_set_height(ht->lv_table, 100);
    lv_obj_set_width(ht->lv_table, 240);
    lv_table_set_col_cnt(ht->lv_table, 2);*/
    
    return scr;
}

static int init(app_t *app, lv_obj_t * parent) {
    debug_app_t *htapp = _from_app(app);
    htapp->screen = screen_create(htapp, parent);
    //vTaskStats(app);
    return 0;
}


static void vTaskStats( app_t *app ) {

    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;
    char snum[5];

    debug_app_t *ht = _from_app(app);

    uxArraySize = uxTaskGetNumberOfTasks();
   
    pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

    if( pxTaskStatusArray != NULL ) {

      
        uxArraySize = uxTaskGetSystemState( pxTaskStatusArray,
                                 uxArraySize,
                                 NULL );
      
        for( x = 0; x < uxArraySize; x++ )
        {
            
            lv_table_set_cell_value(ht->lv_table, x, 0, pxTaskStatusArray[ x ].pcTaskName);
            sprintf( snum, "%i", pxTaskStatusArray[ x ].usStackHighWaterMark );
            lv_table_set_cell_value(ht->lv_table, x, 1, snum);

        }
     
      /* The array is no longer needed, free the memory it consumes. */
      vPortFree( pxTaskStatusArray );
   }
}

static int update(app_t *app) {

   debug_app_t *ht = _from_app(app);

    lv_mem_monitor(&mon);    

    lv_label_set_text_fmt(ht->lv_demo, "%1i.%02i volts %d%%\n%d %% used %d%% frag.\nerror : 0x%08x\n%s\n%x %x\n%s", 
        pinetimecos.batteryVoltage / 1000, 
        pinetimecos.batteryVoltage % 1000 / 10, 
        pinetimecos.batteryPercentRemaining, 
        mon.used_pct, mon.frag_pct,
        pinetimecos.debug,
        pinetimecos.resetReason,
        tsData.gesture, tsData.event,
        pinetimecos.passkey
        );

    //vTaskStats( app );

    return 0;
}

static int gesture(app_t *app, enum appGestures gesture) {
    return 0;
}

static int close(app_t *app) {
    debug_app_t *ht = _from_app(app);
    lv_obj_clean(ht->screen);
    lv_obj_del(ht->screen);    
    ht->screen = NULL;
    return 0;
}

static const app_spec_t debug_spec = {
    .name = "debug",
    .updateInterval = 1000,
    .init = init,
    .update = update,
    .gesture = gesture,
    .close = close,
};