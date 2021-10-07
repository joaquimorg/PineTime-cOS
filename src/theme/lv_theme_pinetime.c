#include "lvgl.h"
#include "lv_gc.h"
#include "lv_theme_pinetime.h"

#define COLOR_WHITE   lv_color_white()
#define COLOR_LIGHT   lv_palette_lighten(LV_PALETTE_GREY, 3)
#define COLOR_MID     lv_palette_lighten(LV_PALETTE_GREY, 1)
#define COLOR_DARK    lv_palette_main(LV_PALETTE_GREY)
#define COLOR_DIM     lv_palette_darken(LV_PALETTE_GREY, 2)
#define PAD_DEF       LV_DPX(5)

typedef struct {
    lv_style_t scr;
    lv_style_t light;
    lv_style_t dark;
    lv_style_t scrollbar;
    lv_style_t pressed;
    lv_style_t disabled;
    lv_style_t pad_zero;
} my_theme_styles_t;


static void style_init_reset(lv_style_t * style);
static void theme_apply(struct _lv_theme_t *, lv_obj_t *);

static my_theme_styles_t * styles;
static bool inited;


static lv_color_t dark_color_filter_cb(const lv_color_filter_dsc_t * f, lv_color_t c, lv_opa_t opa)
{
    LV_UNUSED(f);
    return lv_color_darken(c, opa);
}


static lv_color_t grey_filter_cb(const lv_color_filter_dsc_t * f, lv_color_t color, lv_opa_t opa)
{
    LV_UNUSED(f);
    return lv_color_mix(lv_color_white(), color, opa);
}

static void style_init(void)
{
    style_init_reset(&styles->scrollbar);
    lv_style_set_bg_opa(&styles->scrollbar, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->scrollbar, lv_palette_darken(LV_PALETTE_GREY, 2));    
    lv_style_set_width(&styles->scrollbar,  PAD_DEF);

    style_init_reset(&styles->scr);
    lv_style_set_bg_opa(&styles->scr, LV_OPA_COVER);
    //lv_style_set_bg_color(&styles->scr, COLOR_WHITE);
    lv_style_set_bg_color(&styles->scr, lv_color_make(0x0, 0x0, 0x0));
    lv_style_set_text_color(&styles->scr, COLOR_WHITE);
    lv_style_set_pad_row(&styles->scr, PAD_DEF / 2);
    lv_style_set_pad_column(&styles->scr, PAD_DEF / 2);

    style_init_reset(&styles->light);
    lv_style_set_bg_opa(&styles->light, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->light, COLOR_LIGHT);
    lv_style_set_border_color(&styles->light, COLOR_MID);
    lv_style_set_border_width(&styles->light, 1);
    lv_style_set_pad_all(&styles->light, PAD_DEF);
    lv_style_set_pad_gap(&styles->light, PAD_DEF / 2);
    lv_style_set_line_width(&styles->light, LV_DPX(2));
    lv_style_set_line_color(&styles->light, COLOR_MID);
    lv_style_set_arc_width(&styles->light, LV_DPX(2));
    lv_style_set_arc_color(&styles->light, COLOR_MID);

    style_init_reset(&styles->dark);
    lv_style_set_radius(&styles->dark, 12);
    lv_style_set_text_color(&styles->dark, lv_color_make(0x0, 0x0, 0x0));
    lv_style_set_bg_opa(&styles->dark, LV_OPA_COVER);
    lv_style_set_bg_color(&styles->dark, COLOR_DARK);
    lv_style_set_border_color(&styles->dark, COLOR_DIM);
    lv_style_set_border_width(&styles->dark, 1);
    lv_style_set_pad_all(&styles->dark, 8);
    lv_style_set_pad_gap(&styles->dark, 8);
    lv_style_set_line_width(&styles->dark, LV_DPX(2));
    lv_style_set_line_color(&styles->dark, COLOR_DIM);
    lv_style_set_arc_width(&styles->dark, LV_DPX(2));
    lv_style_set_arc_color(&styles->dark, COLOR_DIM);

    static lv_color_filter_dsc_t dark_filter;
    lv_color_filter_dsc_init(&dark_filter, dark_color_filter_cb);

    style_init_reset(&styles->pressed);
    lv_style_set_color_filter_dsc(&styles->pressed, &dark_filter);
    lv_style_set_color_filter_opa(&styles->pressed, 35);

    static lv_color_filter_dsc_t grey_filter;
    lv_color_filter_dsc_init(&grey_filter, grey_filter_cb);

    style_init_reset(&styles->disabled);
    lv_style_set_color_filter_dsc(&styles->disabled, &grey_filter);
    lv_style_set_color_filter_opa(&styles->disabled, LV_OPA_70);

    style_init_reset(&styles->pad_zero);
    lv_style_set_pad_all(&styles->pad_zero, 0);
    lv_style_set_pad_gap(&styles->pad_zero, 0);

}

void lv_theme_pinetime_init(lv_theme_t *theme) {

    if(!inited) {
        LV_GC_ROOT(_lv_theme_default_styles) = lv_mem_alloc(sizeof(my_theme_styles_t));
        styles = (my_theme_styles_t *)LV_GC_ROOT(_lv_theme_default_styles);
    }

    theme->font_small = LV_FONT_DEFAULT;
    theme->font_normal = LV_FONT_DEFAULT;
    theme->font_large = LV_FONT_DEFAULT;
    theme->apply_cb = theme_apply;

    style_init();

    inited = true;

}

static void theme_apply(lv_theme_t *theme, lv_obj_t *obj) {
    
    LV_UNUSED(theme);

    if(lv_obj_get_parent(obj) == NULL) {
        lv_obj_add_style(obj, &styles->scr, 0);
        lv_obj_add_style(obj, &styles->scrollbar, LV_PART_SCROLLBAR);
        return;
    }

    if(lv_obj_check_type(obj, &lv_obj_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->scrollbar, LV_PART_SCROLLBAR);
    }
#if LV_USE_BTN
    else if(lv_obj_check_type(obj, &lv_btn_class)) {
        lv_obj_add_style(obj, &styles->dark, 0);
        lv_obj_add_style(obj, &styles->pressed, LV_STATE_PRESSED);
        lv_obj_add_style(obj, &styles->disabled, LV_STATE_DISABLED);
    }
#endif

#if LV_USE_BAR
    else if(lv_obj_check_type(obj, &lv_bar_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->pad_zero, 0);
        lv_obj_add_style(obj, &styles->dark, LV_PART_INDICATOR);
    }
#endif

#if LV_USE_SLIDER
    else if(lv_obj_check_type(obj, &lv_slider_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->pad_zero, 0);
        lv_obj_add_style(obj, &styles->dark, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &styles->dark, LV_PART_KNOB);
    }
#endif

#if LV_USE_CHECKBOX
    else if(lv_obj_check_type(obj, &lv_checkbox_class)) {
        lv_obj_add_style(obj, &styles->light, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &styles->disabled, LV_PART_INDICATOR | LV_STATE_DISABLED);
        lv_obj_add_style(obj, &styles->dark, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &styles->pressed, LV_PART_INDICATOR | LV_STATE_PRESSED);
    }
#endif

#if LV_USE_SWITCH
    else if(lv_obj_check_type(obj, &lv_switch_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->pad_zero, 0);
        lv_obj_add_style(obj, &styles->dark, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &styles->dark, LV_PART_KNOB);
        lv_obj_add_style(obj, &styles->pad_zero, LV_PART_KNOB);
    }
#endif

#if LV_USE_CHART
    else if(lv_obj_check_type(obj, &lv_chart_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->scrollbar, LV_PART_SCROLLBAR);
        lv_obj_add_style(obj, &styles->light, LV_PART_ITEMS);
        lv_obj_add_style(obj, &styles->light, LV_PART_TICKS);
        lv_obj_add_style(obj, &styles->light, LV_PART_CURSOR);
    }
#endif

#if LV_USE_ROLLER
    else if(lv_obj_check_type(obj, &lv_roller_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->dark, LV_PART_SELECTED);
    }
#endif

#if LV_USE_DROPDOWN
    else if(lv_obj_check_type(obj, &lv_dropdown_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->pressed, LV_STATE_PRESSED);
    }
    else if(lv_obj_check_type(obj, &lv_dropdownlist_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->scrollbar, LV_PART_SCROLLBAR);
        lv_obj_add_style(obj, &styles->light, LV_PART_SELECTED);
        lv_obj_add_style(obj, &styles->dark, LV_PART_SELECTED | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &styles->pressed, LV_PART_SELECTED | LV_STATE_PRESSED);
    }
#endif

#if LV_USE_ARC
    else if(lv_obj_check_type(obj, &lv_arc_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->dark, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &styles->pad_zero, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &styles->dark, LV_PART_KNOB);
    }
#endif

#if LV_USE_METER
    else if(lv_obj_check_type(obj, &lv_meter_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
    }
#endif

#if LV_USE_CALENDAR
    else if(lv_obj_check_type(obj, &lv_calendar_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->light, LV_PART_ITEMS | LV_STATE_PRESSED);
        lv_obj_add_style(obj, &styles->pressed, LV_PART_ITEMS | LV_STATE_PRESSED);
        lv_obj_add_style(obj, &styles->disabled, LV_PART_ITEMS | LV_STATE_DISABLED);
    }
#endif

#if LV_USE_LIST
    else if(lv_obj_check_type(obj, &lv_list_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->scrollbar, LV_PART_SCROLLBAR);
        return;
    }
    else if(lv_obj_check_type(obj, &lv_list_text_class)) {

    }
    else if(lv_obj_check_type(obj, &lv_list_btn_class)) {
        lv_obj_add_style(obj, &styles->dark, 0);
        lv_obj_add_style(obj, &styles->pressed, LV_STATE_PRESSED);

    }
#endif

#if LV_USE_COLORWHEEL
    else if(lv_obj_check_type(obj, &lv_colorwheel_class)) {
        lv_obj_add_style(obj, &styles->light, 0);
        lv_obj_add_style(obj, &styles->light, LV_PART_KNOB);
    }
#endif

}


static void style_init_reset(lv_style_t * style)
{
    if(inited) lv_style_reset(style);
    else lv_style_init(style);
}
