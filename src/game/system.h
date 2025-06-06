#pragma once

#include "input/keys.h"
#include "platform/platform.h"

struct display_size;

/**
 * Center window
 */
void system_center(void);

/**
 * Returns whether the window must always be fullscreen
 * @return true when only fullscreen can be used, false otherwise
 */
int system_is_fullscreen_only(void);

/**
 * Set cursor to the specified cursor in @link input/cursor.h @endlink
 * @param cursor_id Cursor to set
 */
void system_set_cursor(int cursor_id);

/**
 * Get the key corresponding to the symbol in the current layout
 * @param name Name of the key
 * @return Corresponding key, or KEY_NONE if the key does not exist on the layout
 */
e_key system_keyboard_key_for_symbol(pcstr name);

/**
 * Gets the key name for the current keyboard layout
 * @param key Key to get the name for
 * @return Key name, may be empty
 */
const char* system_keyboard_key_name(int key);

/**
 * Gets the key modifier name
 * @param modifier Modifier
 * @return Modifier name, may depend on OS
 */
const char* system_keyboard_key_modifier_name(int modifier);

/**
 * Sets the position/size of the keyboard input box
 * @param x X offset
 * @param y Y offset
 * @param width Width of the box
 * @param height Height of the box
 */
void system_keyboard_set_input_rect(int x, int y, int width, int height);

/**
 * Show the virtual keyboard
 * @param text The text to display on the virtual keyboard
 * @param max_length The maximum length of the text
 */
void system_keyboard_show(const uint8_t* text, int max_length);

/**
 * Hide the virtual keyboard
 */
void system_keyboard_hide(void);

/**
 * Sets mouse to relative mode, where moving the mouse
 * does not move the cursor on the screen
 * @param enabled Boolean: 1 for enable, 0 for disable
 */
void system_mouse_set_relative_mode(int enabled);

/**
 * Gets relative mouse movement when in relative mode
 * @param x X position of the mouse
 * @param y Y position of the mouse
 */
void system_mouse_get_relative_state(int* x, int* y);

/**
 * Moves the mouse cursor
 * @param delta_x The amount to move on the X axis
 * @param delta_y The amount to move on the Y axis
 */
void system_move_mouse_cursor(int delta_x, int delta_y);

/**
 * Sets the mouse position
 * @param x Pointer to X position of the mouse
 * @param y Pointer to Y position of the mouse
 * Note: the function limits x and y to the bounds of the game window
 * if the x and y are less than 0 or greater than the width/height, they will be
 * changed to fit in the window
 */
void system_set_mouse_position(int* x, int* y);

/**
 * Exit the game
 */
void system_exit(void);
