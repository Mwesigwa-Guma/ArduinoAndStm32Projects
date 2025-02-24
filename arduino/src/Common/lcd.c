#include <util/delay.h>
#include "i2c.h"
#include "lcd.h"

#define LCD_ADDR 0x3F  // I2C address of the LCD (Change to 0x27 if needed)
#define LCD_BACKLIGHT 0x08 // Backlight control bit
#define ENABLE 0x04  // Enable bit for sending commands
#define LCD_CMD 0    // Command mode
#define LCD_DATA 1   // Data mode

static uint8_t cursor_row = 0;
static uint8_t cursor_col = 0;

// Function to send data/commands to LCD via I2C (4-bit mode)
void lcd_send(uint8_t value, uint8_t mode) {
    lcd_write_nibble(value & 0xF0, mode); // Send high nibble
    lcd_write_nibble((value << 4) & 0xF0, mode); // Send low nibble
}

// Function to write 4 bits to LCD with backlight enabled
void lcd_write_nibble(uint8_t nibble, uint8_t mode) {
    uint8_t data = nibble | LCD_BACKLIGHT | (mode == LCD_DATA ? 1 : 0);
    lcd_enable_pulse(data);
}

// Function to generate an enable pulse to latch data into LCD
void lcd_enable_pulse(uint8_t data) {
    i2c_start();
    i2c_write(LCD_ADDR << 1);
    i2c_write(data | ENABLE); // Enable high
    i2c_stop();
    _delay_us(1);

    i2c_start();
    i2c_write(LCD_ADDR << 1);
    i2c_write(data & ~ENABLE); // Enable low
    i2c_stop();
    _delay_us(50);
}

// Function to initialize the LCD
void lcd_init() {
    _delay_ms(50); // Wait for LCD to power up
    lcd_write_nibble(0x30, LCD_CMD); // Wake-up sequence
    _delay_ms(5);
    lcd_write_nibble(0x30, LCD_CMD);
    _delay_ms(5);
    lcd_write_nibble(0x30, LCD_CMD);
    _delay_ms(1);
    lcd_write_nibble(0x20, LCD_CMD); // Set 4-bit mode

    lcd_send(0x28, LCD_CMD); // Function set: 2-line, 5x8 font
    lcd_send(0x0C, LCD_CMD); // Display ON, Cursor OFF
    lcd_send(0x06, LCD_CMD); // Entry mode: Move right, no shift
    lcd_send(0x01, LCD_CMD); // Clear display
    _delay_ms(5);
}

// Function to print a string to LCD
void lcd_print(const char *str) {
    while (*str) {
        lcd_send(*str++, LCD_DATA);
    }
}

// Function to print a string to a specific row on the LCD
void lcd_print_row(uint8_t row, const char *str) {
    lcd_set_cursor(row, 1); // Set cursor to the start of the specified row
    lcd_print(str); // Print the string
}

// Function to turn on LCD backlight
void lcd_backlight_on() {
    i2c_start();
    i2c_write(LCD_ADDR << 1);
    i2c_write(LCD_BACKLIGHT);
    i2c_stop();
}

// Function to clear the screen
void lcd_clear() {
    lcd_send(0x01, LCD_CMD); // Clear display command
    _delay_ms(5); // Wait for the command to complete
}

// Function to set the cursor position
void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t address = (row == 0) ? col : (0x40 + col);
    lcd_send(0x80 | address, LCD_CMD);
    cursor_row = row;
    cursor_col = col;
}

// Function to show the cursor
void lcd_show_cursor() {
    lcd_send(0x0E, LCD_CMD); // Display ON, Cursor ON
}

// Function to hide the cursor
void lcd_hide_cursor() {
    lcd_send(0x0C, LCD_CMD); // Display ON, Cursor OFF
}

// Function to get the cursor row
uint8_t lcd_get_cursor_row() {
    return cursor_row;
}

// Function to get the cursor column
uint8_t lcd_get_cursor_col() {
    return cursor_col;
}