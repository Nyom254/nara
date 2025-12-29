#include "ui.h"

/* Globals */
static lv_obj_t * label_battery;
static lv_obj_t * label_wifi;

static lv_obj_t * label_temp;
static lv_obj_t * label_hum;
static lv_obj_t * label_ph;
static lv_obj_t * label_ec;
static lv_obj_t * label_n;
static lv_obj_t * label_p;
static lv_obj_t * label_k;

/* Thresholds */
#define TEMP_LOW   18.0f
#define TEMP_HIGH  35.0f
#define PH_LOW     5.5f
#define PH_HIGH    7.5f


/* Status bar */
static void create_status_bar(lv_obj_t * parent)
{
    lv_obj_t * bar = lv_obj_create(parent);
    lv_obj_set_width(bar, LV_PCT(100));
    lv_obj_set_height(bar, LV_PCT(13));
    lv_obj_set_style_pad_hor(bar, 8, 0);
    lv_obj_set_style_pad_ver(bar, 4, 0);

    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        bar,
        LV_FLEX_ALIGN_SPACE_BETWEEN,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );

    /* WIFI ICON */
    label_wifi = lv_label_create(bar);
    lv_obj_set_style_text_font(
        label_wifi,
        &lv_font_montserrat_20,
        0
    );
    lv_label_set_text(label_wifi, LV_SYMBOL_CLOSE);

    /* BATTERY ICON */
    label_battery = lv_label_create(bar);
    lv_obj_set_style_text_font(
        label_battery,
        &lv_font_montserrat_20,
        0
    );
    lv_label_set_text(label_battery, LV_SYMBOL_BATTERY_FULL " 100%");
}


/* Sensor info panel */
static void create_sensor_info(lv_obj_t * parent)
{
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_width(panel, LV_PCT(100));
    lv_obj_set_height(panel, LV_PCT(75));

    lv_obj_set_style_radius(panel, 12, 0);

    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(panel, 8, 0);
    lv_obj_set_style_pad_all(panel, 0, 0);

    /* 
        Temperature, Humidity, pH, EC grid 
    */
    lv_obj_t * thcp = lv_obj_create(panel);
    lv_obj_set_width(thcp, LV_PCT(100));
    lv_obj_set_style_border_width(thcp, 0, 0);
    lv_obj_set_style_outline_width(thcp, 0, 0);
    lv_obj_set_style_shadow_width(thcp, 0, 0);

    lv_obj_set_layout(thcp, LV_LAYOUT_GRID);
    static lv_coord_t col_dsc[] = { LV_GRID_FR(2), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t row_dsc[] = { LV_GRID_FR(2), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(thcp, col_dsc, row_dsc);

    label_temp = lv_label_create(thcp);
    lv_obj_set_grid_cell(label_temp, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_label_set_text(label_temp, "T: --.- C");
    lv_obj_set_style_text_font(
        label_temp,
        &lv_font_montserrat_20,
        0
    );

    label_hum = lv_label_create(thcp);
    lv_obj_set_grid_cell(label_hum, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_label_set_text(label_hum, "H: --.- %");
    lv_obj_set_style_text_font(
        label_hum,
        &lv_font_montserrat_20,
        0
    );

    label_ph = lv_label_create(thcp);
    lv_obj_set_grid_cell(label_ph, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_label_set_text(label_ph, "pH: --.-");
    lv_obj_set_style_text_font(
        label_ph,
        &lv_font_montserrat_20,
        0
    );
    label_ec = lv_label_create(thcp);
    lv_obj_set_grid_cell(label_ec, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_label_set_text(label_ec, "EC: --.-");
    lv_obj_set_style_text_font(
        label_ec,
        &lv_font_montserrat_20,
        0
    );

    /* 
        NPK Level
    */

    lv_obj_t * npk_cont = lv_obj_create(panel);
    lv_obj_set_width(npk_cont, LV_PCT(100));
    lv_obj_set_height(npk_cont, LV_PCT(35));
    lv_obj_set_style_border_width(npk_cont, 0, 0);
    lv_obj_set_style_outline_width(npk_cont, 0, 0);
    lv_obj_set_style_shadow_width(npk_cont, 0, 0);


    lv_obj_set_flex_flow(npk_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        npk_cont,
        LV_FLEX_ALIGN_SPACE_EVENLY,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );
    lv_obj_set_style_pad_all(npk_cont, 0, 0);
    lv_obj_set_style_pad_gap(npk_cont, 0, 0);

    lv_obj_t * n_cont = lv_obj_create(npk_cont);
    lv_obj_set_height(n_cont, LV_PCT(100));
    lv_obj_clear_flag(n_cont, LV_OBJ_FLAG_SCROLLABLE);
    label_n = lv_label_create(n_cont);
    lv_obj_center(label_n);
    lv_obj_set_height(label_n, LV_PCT(100));
    lv_label_set_text(label_n, "N: --.- mg/L");
    lv_obj_set_style_text_font(
        label_n,
        &lv_font_montserrat_16, 0
    );  

    lv_obj_t * p_cont = lv_obj_create(npk_cont);
    lv_obj_set_height(p_cont, LV_PCT(100));
    lv_obj_clear_flag(p_cont, LV_OBJ_FLAG_SCROLLABLE);
    label_p = lv_label_create(p_cont);
    lv_obj_center(label_p);
    lv_obj_set_height(label_p, LV_PCT(100));
    lv_label_set_text(label_p, "P: --.- mg/L");
    lv_obj_set_style_text_font(
        label_p,
        &lv_font_montserrat_16, 0
    );  
    lv_obj_t * k_cont = lv_obj_create(npk_cont);
    lv_obj_set_height(k_cont, LV_PCT(100));
    lv_obj_clear_flag(k_cont, LV_OBJ_FLAG_SCROLLABLE);
    label_k = lv_label_create(k_cont);
    lv_obj_center(label_k);
    lv_obj_set_height(label_k, LV_PCT(100));
    lv_label_set_text(label_k, "K: --.- mg/L");
    lv_obj_set_style_text_font(
        label_k,
        &lv_font_montserrat_16, 0
    );

}

/* UI init */
void ui_init(void)
{
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_style_pad_all(scr, 0, 0);

    lv_obj_t * root = lv_obj_create(scr);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root, 0, 0);

    create_status_bar(root);
    create_sensor_info(root);
}

/* Updates */
void ui_update_sensor(const sensor_data_t * s)
{
    if (!label_temp) return;

    lv_label_set_text_fmt(label_temp,
        "T: %d.%d C",
        (int)s->temperature,
        (int)(s->temperature * 10) % 10
    );

    lv_label_set_text_fmt(label_hum,
        "H: %d.%d %%",
        (int)s->humidity,
        (int)(s->humidity * 10) % 10
    );

    lv_label_set_text_fmt(label_ph,
        "pH: %d.%d",
        (int)s->pH,
        (int)(s->pH * 10) % 10
    );

    lv_label_set_text_fmt(label_ec,
        "EC: %d.%d",
        (int)s->conductivity,
        (int)(s->conductivity * 10) % 10
    );

    lv_label_set_text_fmt(label_n,
        "N: %d.%d mg/L",
        (int)s->nitrogen,
        (int)(s->nitrogen * 10) % 10
    );

    lv_label_set_text_fmt(label_p,
        "P: %d.%d mg/L",
        (int)s->phosphorus,
        (int)(s->phosphorus * 10) % 10
    );

    lv_label_set_text_fmt(label_k,
        "K: %d.%d mg/L",
        (int)s->potassium,
        (int)(s->potassium * 10) % 10
    );

}


void ui_update_battery(uint8_t percent)
{
    const char * icon =
        percent > 80 ? LV_SYMBOL_BATTERY_FULL :
        percent > 40 ? LV_SYMBOL_BATTERY_2 :
        percent > 20 ? LV_SYMBOL_BATTERY_1 :
                       LV_SYMBOL_BATTERY_EMPTY;

    lv_label_set_text_fmt(label_battery, "%s %d%%", icon, percent);
}


void ui_update_bl(bool connected)
{
    lv_label_set_text(
        label_wifi,
        connected ? LV_SYMBOL_BLUETOOTH : LV_SYMBOL_CLOSE
    );
}

