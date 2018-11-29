#include "common.h"
#include "console.h"
#include "monitor.h"

/* The VGA controller also has some ports on the main I/O bus, which you can use to send it specific instructions. (Among others) it has a control register at 0x3D4 and a data register at 0x3D5. We will use these to instruct the controller to update it's cursor position (the flashy underbar thing that tells you where your next character will go).
设置0x3D4来向显示器发送命令
设置0x3D5来告诉显示器内容
*/
static const uint8_t VGA_CTRL_PORT = 0x3D4;
static const uint8_t VGA_CTRL_VAL = 0x3D5;

static uint16_t *video_memory = (uint16_t *)0xB8000;

static const uint16_t SCREEN_ROW_SIZE = 25;
static const uint16_t SCREEN_COL_SIZE = 80;
// static const uint16_t SCREEN_SIZE = SCREEN_COL_SIZE * SCREEN_ROW_SIZE;

static const uint8_t SPACE = 0x20;
static const uint8_t BACK_SPACE = 0x08;
static const uint8_t TABLE = 0x09;    // '\t'
static const uint8_t NEW_LINE = 0x0A; // '\n'
static const uint8_t RETURN = 0x0D;   // '\r'

static const uint16_t TAB_SIZE = 4;

static uint16_t cursor_location = 0;

static uint16_t cursor_x() { return cursor_location % 80; }
static uint16_t cursor_y() { return cursor_location / 80; }


static void set_cursor(int16_t row, int16_t col)
{
        cursor_location = row * SCREEN_COL_SIZE + col;
}
// static uint16_t get_cursor_location()
// {
//     return (cursor_y() * 80 + cursor_x());
// }

static void move_cursor()
{
        // uint16_t cursor_location = get_cursor_location();

        outb(VGA_CTRL_PORT, 14);
        outb(VGA_CTRL_VAL, cursor_location >> 8);
        outb(VGA_CTRL_PORT, 15);
        outb(VGA_CTRL_VAL, (uint8_t)(cursor_location & 255));
}

static void scroll()
{
        uint8_t attribute = (rc_black << 4) | rc_white;
        uint16_t blank = SPACE | ((uint16_t)attribute << 8);

        if (cursor_y() >= 25){
                // 每行内容上移
                for (int i = 0; i < SCREEN_COL_SIZE * (SCREEN_ROW_SIZE - 1); i++) {
                        video_memory[i] = video_memory[i + SCREEN_COL_SIZE];
                }
                uint16_t last_row = SCREEN_ROW_SIZE - 1;
                for (int i = last_row * SCREEN_COL_SIZE; i < SCREEN_COL_SIZE * SCREEN_ROW_SIZE; i++){
                        video_memory[i] = blank;
                }
                // cursor_y = 24;
                // cursor_location -= SCREEN_COL_SIZE;
                set_cursor(SCREEN_ROW_SIZE - 1, 0);
        }
}

static void cursor_back()
{
        if (cursor_location)
                cursor_location--;
}

static void cursor_next()
{
        cursor_location++;
        scroll();
}


void monitor_put(char c)
{
        real_color_t background_color = rc_black;
        real_color_t foreground_color = rc_white;
        uint16_t attribute = (background_color << 4) | foreground_color;
        if (c == BACK_SPACE) {
                cursor_back();
        } else if (c == TABLE) {
                cursor_location = (cursor_location + TAB_SIZE) / TAB_SIZE * TAB_SIZE;
        } else if (c == RETURN) {
                set_cursor(cursor_y(), 0);
        } else if (c == NEW_LINE) {
                set_cursor(cursor_y() + 1, 0);
        } else {
                video_memory[cursor_location] = (attribute << 8) | c;
        }
        cursor_next();
        move_cursor();
};

void monitor_clear()
{
        uint8_t attribute = (rc_black << 4) | rc_white;
        uint16_t blank = SPACE | ((uint16_t)attribute << 8);
        memset(video_memory, blank, SCREEN_COL_SIZE * SCREEN_ROW_SIZE);
        set_cursor(0,0);
        move_cursor();
}

void monitor_write(char *c)
{
        int i = 0;
        while (c[i]) {
                monitor_put(c[i++]);
        }
}