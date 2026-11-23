#include "dashboard_ui.h"

#include <Arduino.h>
#include <cmath>
#include <cstdio>
#include <lvgl.h>

namespace {
constexpr int PAGE_Y = 58;
constexpr int PAGE_H = 366;
constexpr int NAV_Y = 424;

constexpr uint32_t BG = 0x061015;
constexpr uint32_t PANEL = 0x0D1B22;
constexpr uint32_t PANEL_2 = 0x11262D;
constexpr uint32_t BORDER = 0x23434B;
constexpr uint32_t TEXT = 0xF1F8F5;
constexpr uint32_t MUTED = 0x8CA7AD;
constexpr uint32_t GREEN = 0x75F04B;
constexpr uint32_t CYAN = 0x32DDE1;
constexpr uint32_t YELLOW = 0xFFD84A;
constexpr uint32_t ORANGE = 0xFFB44A;

lv_obj_t *root;
lv_obj_t *pages[3];
lv_obj_t *nav_buttons[3];
lv_obj_t *nav_labels[3];
lv_obj_t *header_clock;

lv_obj_t *o_speed;
lv_obj_t *o_range;
lv_obj_t *o_batt_power;
lv_obj_t *o_soc;
lv_obj_t *o_energy;
lv_obj_t *o_pack_detail;
lv_obj_t *o_soc_bar;
lv_obj_t *o_solar;
lv_obj_t *o_flow_batt;
lv_obj_t *o_motor;
lv_obj_t *o_regen;
lv_obj_t *flow_dots[6];

lv_obj_t *s_soc;
lv_obj_t *s_soc_arc;
lv_obj_t *s_voltage;
lv_obj_t *s_current;
lv_obj_t *s_temp;
lv_obj_t *s_energy;
lv_obj_t *s_soh_bar;
lv_obj_t *s_motor_power;
lv_obj_t *s_motor_rpm;
lv_obj_t *s_motor_temp;
lv_obj_t *s_solar_power;
lv_obj_t *s_solar_voltage;
lv_obj_t *s_solar_current;
lv_obj_t *s_cell_delta;
lv_obj_t *s_torque;
lv_obj_t *s_range;
lv_obj_t *s_solar_temp;

lv_obj_t *v_cart_a_soc;
lv_obj_t *v_cart_b_soc;
lv_obj_t *v_cart_a_bar;
lv_obj_t *v_cart_b_bar;
lv_obj_t *v_direction;
lv_obj_t *v_direction_detail;
lv_obj_t *v_flow_dots[4];

int active_page = 0;
bool swap_v2v_demo = false;

lv_obj_t *make_obj(lv_obj_t *parent, int x, int y, int w, int h,
                   uint32_t fill, uint32_t border = BORDER, int radius = 12) {
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_color(obj, lv_color_hex(fill), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(border), 0);
    lv_obj_set_style_border_width(obj, border == fill ? 0 : 1, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y,
                const lv_font_t *font = &lv_font_montserrat_14,
                uint32_t color = TEXT) {
    lv_obj_t *obj = lv_label_create(parent);
    lv_label_set_text(obj, text);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
    return obj;
}

lv_obj_t *bar(lv_obj_t *parent, int x, int y, int w, int h, uint32_t color) {
    lv_obj_t *obj = lv_bar_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_bar_set_range(obj, 0, 100);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x1D3538), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_radius(obj, h / 2, 0);
    lv_obj_set_style_radius(obj, h / 2, LV_PART_INDICATOR);
    return obj;
}

lv_obj_t *line(lv_obj_t *parent, const lv_point_t *points, uint16_t count,
               uint32_t color = GREEN, int width = 3) {
    lv_obj_t *obj = lv_line_create(parent);
    lv_line_set_points(obj, points, count);
    lv_obj_set_style_line_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_line_width(obj, width, 0);
    lv_obj_set_style_line_rounded(obj, true, 0);
    return obj;
}

lv_obj_t *flow_dot(lv_obj_t *parent, uint32_t color = GREEN) {
    lv_obj_t *dot = make_obj(parent, 0, 0, 9, 9, color, color, 5);
    lv_obj_set_style_shadow_color(dot, lv_color_hex(color), 0);
    lv_obj_set_style_shadow_width(dot, 10, 0);
    lv_obj_set_style_shadow_opa(dot, LV_OPA_50, 0);
    return dot;
}

void text_value(lv_obj_t *obj, const char *fmt, float value) {
    char buffer[50];
    snprintf(buffer, sizeof(buffer), fmt, value);
    lv_label_set_text(obj, buffer);
}

void int_value(lv_obj_t *obj, const char *fmt, int value) {
    char buffer[50];
    snprintf(buffer, sizeof(buffer), fmt, value);
    lv_label_set_text(obj, buffer);
}

void make_metric_card(lv_obj_t *parent, int y, const char *title,
                      const char *unit, uint32_t accent, lv_obj_t **value_out) {
    lv_obj_t *card = make_obj(parent, 12, y, 146, 102, PANEL);
    make_obj(card, 0, 0, 4, 102, accent, accent, 2);
    label(card, title, 16, 12, &lv_font_montserrat_14, MUTED);
    *value_out = label(card, "0.0", 15, 36, &lv_font_montserrat_32, TEXT);
    label(card, unit, 102, 53, &lv_font_montserrat_12, MUTED);
}

void show_page(int index) {
    active_page = index;
    for (int i = 0; i < 3; ++i) {
        if (i == index) {
            lv_obj_clear_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_bg_color(nav_buttons[i], lv_color_hex(0x174534), 0);
            lv_obj_set_style_border_color(nav_buttons[i], lv_color_hex(GREEN), 0);
            lv_obj_set_style_text_color(nav_labels[i], lv_color_hex(GREEN), 0);
        } else {
            lv_obj_add_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_bg_color(nav_buttons[i], lv_color_hex(BG), 0);
            lv_obj_set_style_border_color(nav_buttons[i], lv_color_hex(BORDER), 0);
            lv_obj_set_style_text_color(nav_labels[i], lv_color_hex(MUTED), 0);
        }
    }
}

void nav_event(lv_event_t *event) {
    show_page(static_cast<int>(reinterpret_cast<intptr_t>(lv_event_get_user_data(event))));
}

void swap_v2v_event(lv_event_t *) {
    swap_v2v_demo = !swap_v2v_demo;
}

void build_header() {
    lv_obj_t *header = make_obj(root, 0, 0, 800, PAGE_Y, BG, BG, 0);
    make_obj(header, 12, 12, 34, 32, 0x123526, GREEN, 8);
    label(header, "EV", 16, 17, &lv_font_montserrat_16, GREEN);
    label(header, "ELECTRIC GOLF CART", 58, 5, &lv_font_montserrat_20, TEXT);
    label(header, "VEHICLE MONITOR", 59, 31, &lv_font_montserrat_12, MUTED);
    lv_obj_t *badge = make_obj(header, 570, 11, 104, 32, 0x17362B, 0x2E6745, 16);
    label(badge, "DEMO MODE", 12, 8, &lv_font_montserrat_12, GREEN);
    header_clock = label(header, "10:24 AM", 689, 15, &lv_font_montserrat_16, TEXT);

    static lv_point_t divider_points[] = {{12, 56}, {788, 56}};
    line(header, divider_points, 2, BORDER, 1);
}

void build_nav() {
    lv_obj_t *nav = make_obj(root, 0, NAV_Y, 800, 56, 0x09171C, BORDER, 0);
    const char *names[] = {"OVERVIEW", "SYSTEMS", "V2V LAB"};
    const int widths[] = {248, 248, 264};
    int x = 8;
    for (int i = 0; i < 3; ++i) {
        nav_buttons[i] = lv_btn_create(nav);
        lv_obj_set_pos(nav_buttons[i], x, 6);
        lv_obj_set_size(nav_buttons[i], widths[i], 42);
        lv_obj_set_style_radius(nav_buttons[i], 10, 0);
        lv_obj_set_style_border_width(nav_buttons[i], 1, 0);
        lv_obj_set_style_shadow_width(nav_buttons[i], 0, 0);
        lv_obj_add_event_cb(nav_buttons[i], nav_event, LV_EVENT_CLICKED,
                            reinterpret_cast<void *>(static_cast<intptr_t>(i)));
        nav_labels[i] = label(nav_buttons[i], names[i], 0, 0, &lv_font_montserrat_16);
        lv_obj_center(nav_labels[i]);
        x += widths[i] + 7;
    }
}

void build_overview() {
    lv_obj_t *page = pages[0];
    make_metric_card(page, 16, "SPEED", "km/h", CYAN, &o_speed);
    make_metric_card(page, 126, "EST. RANGE", "km", GREEN, &o_range);
    make_metric_card(page, 236, "BATTERY", "kW", YELLOW, &o_batt_power);

    lv_obj_t *main = make_obj(page, 168, 16, 620, 330, PANEL);
    label(main, "LIVE POWER FLOW", 18, 12, &lv_font_montserrat_14, MUTED);
    lv_obj_t *badge = make_obj(main, 473, 8, 129, 26, 0x163B2C, 0x275E3E, 13);
    label(badge, "ALL NORMAL", 19, 5, &lv_font_montserrat_12, GREEN);

    label(main, "BATTERY (SOC)", 18, 54, &lv_font_montserrat_14, MUTED);
    o_soc = label(main, "78%", 17, 75, &lv_font_montserrat_48, GREEN);
    o_energy = label(main, "7.8 / 10.0 kWh", 19, 137, &lv_font_montserrat_16, TEXT);
    o_soc_bar = bar(main, 19, 169, 170, 16, GREEN);
    o_pack_detail = label(main, "51.8 V  |  32.6 C", 19, 200,
                          &lv_font_montserrat_14, MUTED);
    label(main, "DISCHARGING", 19, 233, &lv_font_montserrat_14, GREEN);
    label(main, "10.0 kWh traction pack", 19, 261, &lv_font_montserrat_12, MUTED);
    label(main, "SOH 96%", 19, 282, &lv_font_montserrat_12, MUTED);

    static lv_point_t separator[] = {{207, 48}, {207, 304}};
    line(main, separator, 2, BORDER, 1);

    // Paths are drawn before nodes and moving dots so they remain unobtrusive.
    static lv_point_t solar_path[] = {{341, 131}, {341, 191}};
    static lv_point_t motor_path[] = {{356, 231}, {449, 231}};
    static lv_point_t regen_path[] = {{449, 274}, {356, 274}};
    line(main, solar_path, 2, GREEN, 4);
    line(main, motor_path, 2, GREEN, 4);
    line(main, regen_path, 2, CYAN, 2);

    lv_obj_t *solar = make_obj(main, 286, 48, 110, 84, PANEL_2, 0x74682A, 10);
    label(solar, "SOLAR", 26, 8, &lv_font_montserrat_12, YELLOW);
    o_solar = label(solar, "1.2 kW", 13, 35, &lv_font_montserrat_24, TEXT);

    lv_obj_t *pack = make_obj(main, 231, 190, 126, 86, PANEL_2, 0x357449, 10);
    label(pack, "PACK", 41, 9, &lv_font_montserrat_12, GREEN);
    o_flow_batt = label(pack, "2.4 kW", 19, 37, &lv_font_montserrat_24, TEXT);

    lv_obj_t *motor = make_obj(main, 449, 190, 146, 86, PANEL_2, 0x267078, 10);
    label(motor, "MOTOR", 46, 9, &lv_font_montserrat_12, CYAN);
    o_motor = label(motor, "2.8 kW", 31, 37, &lv_font_montserrat_24, TEXT);

    o_regen = label(main, "REGEN 0.4 kW", 389, 289, &lv_font_montserrat_12, CYAN);
    for (auto &dot : flow_dots) {
        dot = flow_dot(main);
    }
}

lv_obj_t *stat_value(lv_obj_t *parent, const char *title, int x, int y,
                     const char *initial, const lv_font_t *font = &lv_font_montserrat_20) {
    label(parent, title, x, y, &lv_font_montserrat_12, MUTED);
    return label(parent, initial, x, y + 20, font, TEXT);
}

void build_systems() {
    lv_obj_t *page = pages[1];
    lv_obj_t *battery = make_obj(page, 12, 16, 380, 330, PANEL);
    label(battery, "TRACTION BATTERY", 18, 12, &lv_font_montserrat_16, TEXT);
    lv_obj_t *ok = make_obj(battery, 277, 10, 85, 26, 0x153B29, 0x2B613D, 13);
    label(ok, "BMS OK", 16, 5, &lv_font_montserrat_12, GREEN);

    s_soc_arc = lv_arc_create(battery);
    lv_obj_set_pos(s_soc_arc, 17, 55);
    lv_obj_set_size(s_soc_arc, 150, 150);
    lv_arc_set_rotation(s_soc_arc, 135);
    lv_arc_set_bg_angles(s_soc_arc, 0, 270);
    lv_arc_set_range(s_soc_arc, 0, 100);
    lv_obj_remove_style(s_soc_arc, nullptr, LV_PART_KNOB);
    lv_obj_clear_flag(s_soc_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_color(s_soc_arc, lv_color_hex(0x25413B), LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_soc_arc, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_soc_arc, lv_color_hex(GREEN), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(s_soc_arc, 14, LV_PART_INDICATOR);
    s_soc = label(battery, "78%", 43, 108, &lv_font_montserrat_32, GREEN);
    label(battery, "CHARGE", 62, 145, &lv_font_montserrat_12, MUTED);

    s_voltage = stat_value(battery, "PACK VOLTAGE", 192, 57, "51.8 V");
    s_current = stat_value(battery, "PACK CURRENT", 192, 112, "-46.3 A");
    s_temp = stat_value(battery, "BATTERY TEMP", 192, 167, "32.6 C");
    s_energy = stat_value(battery, "ENERGY REMAINING", 192, 222, "7.8 kWh");
    label(battery, "STATE OF HEALTH  96%", 20, 234, &lv_font_montserrat_12, MUTED);
    s_soh_bar = bar(battery, 20, 260, 150, 12, GREEN);
    label(battery, "Positive current = charging", 20, 292, &lv_font_montserrat_12, MUTED);

    lv_obj_t *motor = make_obj(page, 404, 16, 180, 158, PANEL);
    label(motor, "MOTOR", 16, 12, &lv_font_montserrat_16, CYAN);
    s_motor_power = label(motor, "2.8 kW", 16, 41, &lv_font_montserrat_28, TEXT);
    s_motor_rpm = label(motor, "3,480 rpm", 16, 81, &lv_font_montserrat_16, MUTED);
    s_motor_temp = label(motor, "44.8 C", 16, 116, &lv_font_montserrat_14, MUTED);

    lv_obj_t *solar = make_obj(page, 596, 16, 192, 158, PANEL);
    label(solar, "SOLAR / MPPT", 16, 12, &lv_font_montserrat_16, YELLOW);
    s_solar_power = label(solar, "1.2 kW", 16, 41, &lv_font_montserrat_28, TEXT);
    s_solar_voltage = label(solar, "36.8 V", 16, 81, &lv_font_montserrat_16, MUTED);
    s_solar_current = label(solar, "32.6 A", 106, 81, &lv_font_montserrat_16, MUTED);
    s_solar_temp = label(solar, "Panel 41.3 C", 16, 116, &lv_font_montserrat_14, MUTED);

    lv_obj_t *details = make_obj(page, 404, 186, 384, 160, PANEL);
    label(details, "KEY DETAILS", 16, 12, &lv_font_montserrat_14, MUTED);
    label(details, "Cell delta", 16, 44, &lv_font_montserrat_14, MUTED);
    s_cell_delta = label(details, "27 mV", 160, 42, &lv_font_montserrat_18, TEXT);
    label(details, "Motor torque", 16, 77, &lv_font_montserrat_14, MUTED);
    s_torque = label(details, "27.5 Nm", 160, 75, &lv_font_montserrat_18, TEXT);
    label(details, "Estimated range", 16, 110, &lv_font_montserrat_14, MUTED);
    s_range = label(details, "42 km", 160, 108, &lv_font_montserrat_18, TEXT);
    label(details, "DEMO VALUES", 254, 126, &lv_font_montserrat_12, GREEN);
}

lv_obj_t *make_v2v_cart(lv_obj_t *parent, int x, const char *name,
                       lv_obj_t **soc_out, lv_obj_t **bar_out, uint32_t accent) {
    lv_obj_t *cart = make_obj(parent, x, 93, 214, 202, PANEL_2, accent, 14);
    label(cart, name, 18, 15, &lv_font_montserrat_20, TEXT);
    label(cart, "TRACTION PACK", 18, 46, &lv_font_montserrat_12, MUTED);
    *soc_out = label(cart, "0%", 18, 69, &lv_font_montserrat_40, accent);
    *bar_out = bar(cart, 18, 127, 178, 16, accent);
    label(cart, "SIMULATED BMS", 18, 163, &lv_font_montserrat_12, MUTED);
    return cart;
}

void build_v2v() {
    lv_obj_t *page = pages[2];
    lv_obj_t *banner = make_obj(page, 12, 16, 776, 55, 0x382B14, 0x8A6928, 10);
    label(banner, "WORK IN PROGRESS - V2V CHARGING", 18, 8,
          &lv_font_montserrat_18, YELLOW);
    label(banner, "Concept screen only - no charging outputs or hardware are controlled.",
          18, 32, &lv_font_montserrat_12, TEXT);

    make_v2v_cart(page, 24, "CART A", &v_cart_a_soc, &v_cart_a_bar, GREEN);
    make_v2v_cart(page, 562, "CART B", &v_cart_b_soc, &v_cart_b_bar, CYAN);

    label(page, "RECOMMENDED ENERGY PATH", 265, 94, &lv_font_montserrat_12, MUTED);
    v_direction = label(page, "CART A  >  CART B", 272, 122,
                        &lv_font_montserrat_20, GREEN);
    static lv_point_t transfer_path[] = {{251, 179}, {548, 179}};
    line(page, transfer_path, 2, GREEN, 5);
    for (auto &dot : v_flow_dots) {
        dot = flow_dot(page, GREEN);
    }
    v_direction_detail = label(page, "Higher SOC donates to lower SOC", 290, 204,
                               &lv_font_montserrat_14, MUTED);
    label(page, "0.0 kW - NOT CONNECTED", 298, 235, &lv_font_montserrat_14, ORANGE);

    lv_obj_t *swap = lv_btn_create(page);
    lv_obj_set_pos(swap, 310, 267);
    lv_obj_set_size(swap, 180, 40);
    lv_obj_set_style_bg_color(swap, lv_color_hex(0x203A3D), 0);
    lv_obj_set_style_border_color(swap, lv_color_hex(CYAN), 0);
    lv_obj_set_style_border_width(swap, 1, 0);
    lv_obj_set_style_radius(swap, 10, 0);
    lv_obj_add_event_cb(swap, swap_v2v_event, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *swap_label = label(swap, "SWAP DEMO SOC", 0, 0, &lv_font_montserrat_14);
    lv_obj_center(swap_label);

    label(page, "Future hardware: isolated bidirectional DC/DC + compatible BMS + precharge + interlocks.",
          85, 323, &lv_font_montserrat_12, MUTED);
}

void update_flow(uint32_t now_ms, bool reverse_v2v) {
    const float phase = (now_ms % 1800U) / 1800.0f;
    for (int i = 0; i < 2; ++i) {
        const float p = fmodf(phase + i * 0.5f, 1.0f);
        lv_obj_set_pos(flow_dots[i], 337, static_cast<int>(136 + p * 49));
    }
    for (int i = 0; i < 2; ++i) {
        const float p = fmodf(phase + i * 0.5f, 1.0f);
        lv_obj_set_pos(flow_dots[i + 2], static_cast<int>(361 + p * 78), 227);
    }
    for (int i = 0; i < 2; ++i) {
        const float p = fmodf(phase + i * 0.5f, 1.0f);
        lv_obj_set_pos(flow_dots[i + 4], static_cast<int>(438 - p * 75), 270);
        lv_obj_set_style_bg_color(flow_dots[i + 4], lv_color_hex(CYAN), 0);
    }

    for (int i = 0; i < 4; ++i) {
        const float p = fmodf(phase + i * 0.25f, 1.0f);
        const int x = reverse_v2v ? 525 - static_cast<int>(p * 250)
                                  : 258 + static_cast<int>(p * 250);
        lv_obj_set_pos(v_flow_dots[i], x, 175);
    }
}
}  // namespace

void dashboard_ui_init() {
    lv_theme_default_init(nullptr, lv_color_hex(GREEN), lv_color_hex(CYAN),
                          true, &lv_font_montserrat_14);
    root = lv_obj_create(nullptr);
    lv_obj_set_size(root, 800, 480);
    lv_obj_set_style_bg_color(root, lv_color_hex(BG), 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_radius(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_scr_load(root);

    for (int i = 0; i < 3; ++i) {
        pages[i] = make_obj(root, 0, PAGE_Y, 800, PAGE_H, BG, BG, 0);
    }
    build_header();
    build_overview();
    build_systems();
    build_v2v();
    build_nav();
    show_page(0);
}

void dashboard_ui_update(const TelemetryData &d, uint32_t now_ms) {
    const uint32_t demo_minutes = 10U * 60U + 24U + now_ms / 60000U;
    const int hour_24 = static_cast<int>((demo_minutes / 60U) % 24U);
    int hour_12 = hour_24 % 12;
    if (hour_12 == 0) {
        hour_12 = 12;
    }
    char clock_text[18];
    snprintf(clock_text, sizeof(clock_text), "%d:%02d %s", hour_12,
             static_cast<int>(demo_minutes % 60U), hour_24 >= 12 ? "PM" : "AM");
    lv_label_set_text(header_clock, clock_text);

    text_value(o_speed, "%.1f", d.speed_kmh);
    int_value(o_range, "%d", static_cast<int>(lroundf(d.estimated_range_km)));
    text_value(o_batt_power, "%.1f", fabsf(d.battery_power_kw));
    int_value(o_soc, "%d%%", static_cast<int>(lroundf(d.battery_soc_pct)));
    char detail[64];
    snprintf(detail, sizeof(detail), "%.1f / %.1f kWh",
             d.battery_soc_pct * d.battery_capacity_kwh / 100.0f,
             d.battery_capacity_kwh);
    lv_label_set_text(o_energy, detail);
    snprintf(detail, sizeof(detail), "%.1f V  |  %.1f C", d.pack_voltage_v, d.battery_temp_c);
    lv_label_set_text(o_pack_detail, detail);
    lv_bar_set_value(o_soc_bar, static_cast<int>(lroundf(d.battery_soc_pct)), LV_ANIM_OFF);
    text_value(o_solar, "%.1f kW", d.solar_power_kw);
    text_value(o_flow_batt, "%.1f kW", fabsf(d.battery_power_kw));
    text_value(o_motor, "%.1f kW", d.motor_power_kw);

    snprintf(detail, sizeof(detail), "REGEN %.1f kW", d.regen_power_kw);
    lv_label_set_text(o_regen, detail);

    lv_arc_set_value(s_soc_arc, static_cast<int>(lroundf(d.battery_soc_pct)));
    int_value(s_soc, "%d%%", static_cast<int>(lroundf(d.battery_soc_pct)));
    text_value(s_voltage, "%.1f V", d.pack_voltage_v);
    text_value(s_current, "%.1f A", d.pack_current_a);
    text_value(s_temp, "%.1f C", d.battery_temp_c);
    text_value(s_energy, "%.1f kWh", d.battery_soc_pct * d.battery_capacity_kwh / 100.0f);
    lv_bar_set_value(s_soh_bar, static_cast<int>(lroundf(d.battery_soh_pct)), LV_ANIM_OFF);
    text_value(s_motor_power, "%.1f kW", d.motor_power_kw);
    int_value(s_motor_rpm, "%d rpm", static_cast<int>(lroundf(d.motor_rpm)));
    text_value(s_motor_temp, "%.1f C", d.motor_temp_c);
    text_value(s_solar_power, "%.1f kW", d.solar_power_kw);
    text_value(s_solar_voltage, "%.1f V", d.solar_voltage_v);
    text_value(s_solar_current, "%.1f A", d.solar_current_a);
    text_value(s_solar_temp, "Panel %.1f C", d.solar_temp_c);
    int_value(s_cell_delta, "%d mV",
              static_cast<int>(lroundf((d.cell_max_v - d.cell_min_v) * 1000.0f)));
    text_value(s_torque, "%.1f Nm", d.motor_torque_nm);
    int_value(s_range, "%d km", static_cast<int>(lroundf(d.estimated_range_km)));

    const float cart_a = swap_v2v_demo ? d.other_cart_soc_pct : d.battery_soc_pct;
    const float cart_b = swap_v2v_demo ? d.battery_soc_pct : d.other_cart_soc_pct;
    const bool reverse = cart_b > cart_a;
    int_value(v_cart_a_soc, "%d%%", static_cast<int>(lroundf(cart_a)));
    int_value(v_cart_b_soc, "%d%%", static_cast<int>(lroundf(cart_b)));
    lv_bar_set_value(v_cart_a_bar, static_cast<int>(lroundf(cart_a)), LV_ANIM_OFF);
    lv_bar_set_value(v_cart_b_bar, static_cast<int>(lroundf(cart_b)), LV_ANIM_OFF);
    lv_label_set_text(v_direction, reverse ? "CART A  <  CART B" : "CART A  >  CART B");
    if (fabsf(cart_a - cart_b) < 5.0f) {
        lv_label_set_text(v_direction_detail, "SOC gap small - hold / no transfer");
    } else {
        lv_label_set_text(v_direction_detail, "Higher SOC donates to lower SOC");
    }
    update_flow(now_ms, reverse);
}

