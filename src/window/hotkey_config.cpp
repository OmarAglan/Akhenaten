#include "config.h"

#include "core/string.h"
#include "building/building_type.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/elements/generic_button.h"
#include "graphics/elements/lang_text.h"
#include "graphics/elements/panel.h"
#include "graphics/elements/scrollbar.h"
#include "graphics/image_groups.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "graphics/view/view.h"
#include "config/hotkeys.h"
#include "translation/translation.h"
#include "window/hotkey_editor.h"
#include "game/game.h"

#define HOTKEY_HEADER -1
#define TR_NONE -1
#define GROUP_BUILDINGS 28

#define NUM_VISIBLE_OPTIONS 14
#define NUM_BOTTOM_BUTTONS 3

static void on_scroll(void);

static void button_hotkey(int row, int is_alternative);
static void button_reset_defaults(int param1, int param2);
static void button_close(int save, int param2);

static scrollbar_t g_hotkey_window_scrollbar = {{580, 72}, 352, on_scroll};

struct hotkey_widget {
    int action;
    int name_translation;
    int name_text_group;
    int name_text_id;
    void (*close_callback)(void);
};

static hotkey_widget hotkey_widgets[] = {
  {HOTKEY_HEADER, TR_HOTKEY_HEADER_ARROWS},
  {HOTKEY_ARROW_UP, TR_HOTKEY_ARROW_UP},
  {HOTKEY_ARROW_DOWN, TR_HOTKEY_ARROW_DOWN},
  {HOTKEY_ARROW_LEFT, TR_HOTKEY_ARROW_LEFT},
  {HOTKEY_ARROW_RIGHT, TR_HOTKEY_ARROW_RIGHT},
  {HOTKEY_HEADER, TR_HOTKEY_HEADER_GLOBAL},
  {HOTKEY_TOGGLE_FULLSCREEN, TR_HOTKEY_TOGGLE_FULLSCREEN},
  {HOTKEY_CENTER_WINDOW, TR_HOTKEY_CENTER_WINDOW},
  //{HOTKEY_RESIZE_TO_640, TR_HOTKEY_RESIZE_TO_640},
  //{HOTKEY_RESIZE_TO_800, TR_HOTKEY_RESIZE_TO_800},
  //{HOTKEY_RESIZE_TO_1024, TR_HOTKEY_RESIZE_TO_1024},
  {HOTKEY_SAVE_SCREENSHOT, TR_HOTKEY_SAVE_SCREENSHOT},
  {HOTKEY_SAVE_CITY_SCREENSHOT, TR_HOTKEY_SAVE_CITY_SCREENSHOT},
  {HOTKEY_LOAD_FILE, TR_HOTKEY_LOAD_FILE},
  {HOTKEY_SAVE_FILE, TR_HOTKEY_SAVE_FILE},
  {HOTKEY_HEADER, TR_HOTKEY_HEADER_CITY},
  {HOTKEY_INCREASE_GAME_SPEED, TR_HOTKEY_INCREASE_GAME_SPEED},
  {HOTKEY_DECREASE_GAME_SPEED, TR_HOTKEY_DECREASE_GAME_SPEED},
  {HOTKEY_TOGGLE_PAUSE, TR_HOTKEY_TOGGLE_PAUSE},
  {HOTKEY_CYCLE_LEGION, TR_HOTKEY_CYCLE_LEGION},
  {HOTKEY_ROTATE_MAP_LEFT, TR_HOTKEY_ROTATE_MAP_LEFT},
  {HOTKEY_ROTATE_MAP_RIGHT, TR_HOTKEY_ROTATE_MAP_RIGHT},
  {HOTKEY_ROTATE_BUILDING, TR_HOTKEY_ROTATE_BUILDING},
  {HOTKEY_HEADER, TR_HOTKEY_HEADER_BUILD},
  {HOTKEY_BUILD_CLEAR_LAND, TR_NONE, 68, 21},
  {HOTKEY_BUILD_VACANT_HOUSE, TR_NONE, 67, 7},
  {HOTKEY_BUILD_ROAD, TR_NONE, GROUP_BUILDINGS, BUILDING_ROAD},
  {HOTKEY_BUILD_PLAZA, TR_NONE, GROUP_BUILDINGS, BUILDING_PLAZA},
  {HOTKEY_BUILD_GARDENS, TR_NONE, GROUP_BUILDINGS, BUILDING_GARDENS},
  {HOTKEY_BUILD_PREFECTURE, TR_NONE, GROUP_BUILDINGS, BUILDING_POLICE_STATION},
  {HOTKEY_BUILD_ENGINEERS_POST, TR_NONE, GROUP_BUILDINGS, BUILDING_ARCHITECT_POST},
  {HOTKEY_BUILD_DOCTOR, TR_NONE, GROUP_BUILDINGS, BUILDING_APOTHECARY},
  {HOTKEY_BUILD_GRANARY, TR_NONE, GROUP_BUILDINGS, BUILDING_GRANARY},
  {HOTKEY_BUILD_STORAGE_YARD, TR_NONE, GROUP_BUILDINGS, BUILDING_STORAGE_YARD},
  {HOTKEY_BUILD_MARKET, TR_NONE, GROUP_BUILDINGS, BUILDING_BAZAAR},
  {HOTKEY_BUILD_WALL, TR_NONE, GROUP_BUILDINGS, BUILDING_MUD_WALL},
  {HOTKEY_BUILD_GATEHOUSE, TR_NONE, GROUP_BUILDINGS, BUILDING_MUD_GATEHOUSE},
  {HOTKEY_BUILD_RESERVOIR, TR_NONE, GROUP_BUILDINGS, BUILDING_WATER_LIFT},
  {HOTKEY_BUILD_AQUEDUCT, TR_NONE, GROUP_BUILDINGS, BUILDING_IRRIGATION_DITCH},
  {HOTKEY_BUILD_FOUNTAIN, TR_NONE, GROUP_BUILDINGS, BUILDING_MENU_BEAUTIFICATION},
  {HOTKEY_BUILD_ROADBLOCK, TR_NONE, GROUP_BUILDINGS, BUILDING_ROADBLOCK},
  {HOTKEY_HEADER, TR_HOTKEY_HEADER_ADVISORS},
  {HOTKEY_SHOW_ADVISOR_LABOR, TR_HOTKEY_SHOW_ADVISOR_LABOR},
  {HOTKEY_SHOW_ADVISOR_MILITARY, TR_HOTKEY_SHOW_ADVISOR_MILITARY},
  {HOTKEY_SHOW_ADVISOR_KINGDOME, TR_HOTKEY_SHOW_ADVISOR_IMPERIAL},
  {HOTKEY_SHOW_ADVISOR_RATINGS, TR_HOTKEY_SHOW_ADVISOR_RATINGS},
  {HOTKEY_SHOW_ADVISOR_TRADE, TR_HOTKEY_SHOW_ADVISOR_TRADE},
  {HOTKEY_SHOW_ADVISOR_POPULATION, TR_HOTKEY_SHOW_ADVISOR_POPULATION},
  {HOTKEY_SHOW_ADVISOR_HEALTH, TR_HOTKEY_SHOW_ADVISOR_HEALTH},
  {HOTKEY_SHOW_ADVISOR_EDUCATION, TR_HOTKEY_SHOW_ADVISOR_EDUCATION},
  {HOTKEY_SHOW_ADVISOR_ENTERTAINMENT, TR_HOTKEY_SHOW_ADVISOR_ENTERTAINMENT},
  {HOTKEY_SHOW_ADVISOR_RELIGION, TR_HOTKEY_SHOW_ADVISOR_RELIGION},
  {HOTKEY_SHOW_ADVISOR_FINANCIAL, TR_HOTKEY_SHOW_ADVISOR_FINANCIAL},
  {HOTKEY_SHOW_ADVISOR_CHIEF, TR_HOTKEY_SHOW_ADVISOR_CHIEF},
  {HOTKEY_SHOW_ADVISOR_HOUSING, TR_HOTKEY_SHOW_ADVISOR_HOUSING},
  {HOTKEY_HEADER, TR_HOTKEY_HEADER_OVERLAYS},
  {HOTKEY_TOGGLE_OVERLAY, TR_HOTKEY_TOGGLE_OVERLAY},
  {HOTKEY_SHOW_OVERLAY_WATER, TR_HOTKEY_SHOW_OVERLAY_WATER},
  {HOTKEY_SHOW_OVERLAY_FIRE, TR_HOTKEY_SHOW_OVERLAY_FIRE},
  {HOTKEY_SHOW_OVERLAY_DAMAGE, TR_HOTKEY_SHOW_OVERLAY_DAMAGE},
  {HOTKEY_SHOW_OVERLAY_CRIME, TR_HOTKEY_SHOW_OVERLAY_CRIME},
  {HOTKEY_SHOW_OVERLAY_PROBLEMS, TR_HOTKEY_SHOW_OVERLAY_PROBLEMS},
  {HOTKEY_HEADER, TR_HOTKEY_HEADER_BOOKMARKS},
  {HOTKEY_GO_TO_BOOKMARK_1, TR_HOTKEY_GO_TO_BOOKMARK_1},
  {HOTKEY_GO_TO_BOOKMARK_2, TR_HOTKEY_GO_TO_BOOKMARK_2},
  {HOTKEY_GO_TO_BOOKMARK_3, TR_HOTKEY_GO_TO_BOOKMARK_3},
  {HOTKEY_GO_TO_BOOKMARK_4, TR_HOTKEY_GO_TO_BOOKMARK_4},
  {HOTKEY_SET_BOOKMARK_1, TR_HOTKEY_SET_BOOKMARK_1},
  {HOTKEY_SET_BOOKMARK_2, TR_HOTKEY_SET_BOOKMARK_2},
  {HOTKEY_SET_BOOKMARK_3, TR_HOTKEY_SET_BOOKMARK_3},
  {HOTKEY_SET_BOOKMARK_4, TR_HOTKEY_SET_BOOKMARK_4},
  {HOTKEY_HEADER, TR_HOTKEY_HEADER_EDITOR},
  {HOTKEY_EDITOR_TOGGLE_BATTLE_INFO, TR_HOTKEY_EDITOR_TOGGLE_BATTLE_INFO},
};

#define HOTKEY_X_OFFSET_1 290
#define HOTKEY_X_OFFSET_2 430
#define HOTKEY_BTN_WIDTH 140
#define HOTKEY_BTN_HEIGHT 22

static generic_button hotkey_buttons[] = {
  {HOTKEY_X_OFFSET_1, 80 + 24 * 0, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 0, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 0, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 0, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 1, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 1, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 1, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 1, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 2, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 2, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 2, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 2, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 3, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 3, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 3, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 3, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 4, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 4, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 4, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 4, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 5, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 5, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 5, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 5, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 6, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 6, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 6, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 6, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 7, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 7, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 7, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 7, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 8, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 8, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 8, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 8, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 9, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 9, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 9, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 9, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 10, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 10, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 10, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 10, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 11, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 11, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 11, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 11, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 12, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 12, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 12, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 12, 1},
  {HOTKEY_X_OFFSET_1, 80 + 24 * 13, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 13, 0},
  {HOTKEY_X_OFFSET_2, 80 + 24 * 13, HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT, button_hotkey, button_none, 13, 1},
};

static generic_button bottom_buttons[] = {
  {240, 430, 160, 30, button_reset_defaults, button_none},
  {410, 430, 100, 30, button_close, button_none, 0},
  {520, 430, 100, 30, button_close, button_none, 1},
};

static int bottom_button_texts[] = {TR_BUTTON_RESET_DEFAULTS, TR_BUTTON_CANCEL, TR_BUTTON_OK};

struct hotkeys_window_data_t {
    int focus_button;
    int bottom_focus_button;
    hotkey_mapping mappings[HOTKEY_MAX_ITEMS];
    void (*close_callback)(void);
};

hotkeys_window_data_t g_hotkeys_window_data;

static void init(void (*close_callback)(void)) {
    auto& data = g_hotkeys_window_data;
    data.close_callback = close_callback;

    g_hotkey_window_scrollbar.init(0, std::size(hotkey_widgets) - NUM_VISIBLE_OPTIONS);

    for (int i = 0; i < HOTKEY_MAX_ITEMS; i++) {
        hotkey_mapping empty("", KEY_NONE, KEY_MOD_NONE, (e_hotkey_action)i);

        const hotkey_mapping* mapping = game_hotkeys::hotkey_for_action((e_hotkey_action)i);
        data.mappings[i] = mapping ? *mapping : empty;
    }
}

static void draw_background(int) {
    painter ctx = game.painter();
    auto& data = g_hotkeys_window_data;
    graphics_clear_screen();

    ImageDraw::img_background(ctx, image_id_from_group(PACK_UNLOADED, 8));
    graphics_set_to_dialog();
    outer_panel_draw(vec2i{0, 0}, 40, 30);

    text_draw_centered(translation_for(TR_HOTKEY_TITLE), 16, 16, 608, FONT_LARGE_BLACK_ON_LIGHT, 0);

    text_draw_centered(translation_for(TR_HOTKEY_LABEL), HOTKEY_X_OFFSET_1, 55, HOTKEY_BTN_WIDTH, FONT_NORMAL_BLACK_ON_LIGHT, 0);
    text_draw_centered(translation_for(TR_HOTKEY_ALTERNATIVE_LABEL), HOTKEY_X_OFFSET_2, 55, HOTKEY_BTN_WIDTH, FONT_NORMAL_BLACK_ON_LIGHT,0);

    inner_panel_draw({ 20, 72 }, { 35, 22 });
    int y_base = 80;
    for (int i = 0; i < NUM_VISIBLE_OPTIONS; i++) {
        hotkey_widget* widget = &hotkey_widgets[i + g_hotkey_window_scrollbar.scroll_position];
        int text_offset = y_base + 6 + 24 * i;
        if (widget->action == HOTKEY_HEADER)
            text_draw(translation_for(widget->name_translation), 32, text_offset, FONT_NORMAL_WHITE_ON_DARK, 0);
        else {
            if (widget->name_translation != TR_NONE) {
                text_draw(translation_for(widget->name_translation), 32, text_offset, FONT_NORMAL_BLACK_ON_DARK, 0);
            } else {
                lang_text_draw(widget->name_text_group, widget->name_text_id, 32, text_offset, FONT_NORMAL_BLACK_ON_DARK);
            }

            const hotkey_mapping* mapping = &data.mappings[widget->action];
            if (mapping->state.key) {
                const uint8_t* keyname = key_combination_display_name(mapping->state.key, mapping->state.modifiers);
                graphics_set_clip_rectangle({HOTKEY_X_OFFSET_1, text_offset}, {HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT});
                text_draw_centered(keyname, HOTKEY_X_OFFSET_1 + 3, text_offset, HOTKEY_BTN_WIDTH - 6, FONT_NORMAL_WHITE_ON_DARK, 0);
                graphics_reset_clip_rectangle();
            }

            if (mapping->alt.key) {
                graphics_set_clip_rectangle({HOTKEY_X_OFFSET_2, text_offset}, {HOTKEY_BTN_WIDTH, HOTKEY_BTN_HEIGHT});
                const uint8_t* keyname = key_combination_display_name(mapping->alt.key, mapping->alt.modifiers);
                text_draw_centered(keyname, HOTKEY_X_OFFSET_2 + 3, text_offset, HOTKEY_BTN_WIDTH - 6, FONT_NORMAL_WHITE_ON_DARK, 0);
                graphics_reset_clip_rectangle();
            }
        }
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        text_draw_centered(translation_for(bottom_button_texts[i]),
                           bottom_buttons[i].x,
                           bottom_buttons[i].y + 9,
                           bottom_buttons[i].width,
                           FONT_NORMAL_BLACK_ON_LIGHT,
                           0);
    }

    graphics_reset_dialog();
}

static void draw_foreground(int) {
    auto& data = g_hotkeys_window_data;
    graphics_set_to_dialog();

    scrollbar_draw(vec2i{0, 0}, &g_hotkey_window_scrollbar);

    for (int i = 0; i < NUM_VISIBLE_OPTIONS; i++) {
        hotkey_widget* widget = &hotkey_widgets[i + g_hotkey_window_scrollbar.scroll_position];
        if (widget->action != HOTKEY_HEADER) {
            generic_button* btn = &hotkey_buttons[2 * i];
            button_border_draw(btn->x, btn->y, btn->width, btn->height, data.focus_button == 1 + 2 * i);
            btn++;
            button_border_draw(btn->x, btn->y, btn->width, btn->height, data.focus_button == 2 + 2 * i);
        }
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        button_border_draw(bottom_buttons[i].x,
                           bottom_buttons[i].y,
                           bottom_buttons[i].width,
                           bottom_buttons[i].height,
                           data.bottom_focus_button == i + 1);
    }
    graphics_reset_dialog();
}

static void handle_input(const mouse* m, const hotkeys* h) {
    auto& data = g_hotkeys_window_data;
    const mouse* m_dialog = mouse_in_dialog(m);
    if (scrollbar_handle_mouse(vec2i{0, 0}, &g_hotkey_window_scrollbar, m_dialog)) {
        return;
    }

    int mouse_button = 0;
    mouse_button |= generic_buttons_handle_mouse(m_dialog, {0, 0}, hotkey_buttons, NUM_VISIBLE_OPTIONS * 2, &data.focus_button);
    mouse_button |= generic_buttons_handle_mouse(m_dialog, {0, 0}, bottom_buttons, NUM_BOTTOM_BUTTONS, &data.bottom_focus_button);

    if (!mouse_button && (m->right.went_up || h->escape_pressed)) {
        data.close_callback();
        window_go_back();
    }
}

static void set_hotkey(int action, int index, e_key key, e_key_mode modifiers) {
    auto& data = g_hotkeys_window_data;
    auto &state = index ? data.mappings[action].alt : data.mappings[action].state;

    state.key = key;
    state.modifiers = modifiers;
}

static void button_hotkey(int row, int is_alternative) {
    hotkey_widget* widget = &hotkey_widgets[row + g_hotkey_window_scrollbar.scroll_position];
    if (widget->action == HOTKEY_HEADER) {
        return;
    }

    window_hotkey_editor_show(widget->action, is_alternative, set_hotkey);
}

static void button_reset_defaults(int param1, int param2) {
    auto& data = g_hotkeys_window_data;
    for (int action = 0; action < HOTKEY_MAX_ITEMS; action++) {
        data.mappings[action] = *game_hotkeys::hotkey_default((e_hotkey_action)action);
    }
}

static void on_scroll(void) {

}

static void button_close(int save, int param2) {
    auto& data = g_hotkeys_window_data;
    if (!save) {
        window_go_back();
        return;
    }

    for (int action = 0; action < HOTKEY_MAX_ITEMS; action++) {
        game_hotkeys::set_hotkey(data.mappings[action]);
    }

    game_hotkeys::save();
    game_hotkeys::install();
    window_go_back();
}

void window_hotkey_config_show(void (*close_callback)(void)) {
    window_type window = {
        WINDOW_HOTKEY_CONFIG,
        draw_background, 
        draw_foreground,
        handle_input
    };
    init(close_callback);
    window_show(&window);
}
