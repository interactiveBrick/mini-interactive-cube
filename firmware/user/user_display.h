#ifndef _USER_DISPLAY_H_
#define _USER_DISPLAY_H_

void display_init();

void display_update();

void display_set_pixel(uint8_t pixel, uint8_t value);
void display_set_side(uint8_t side, uint32_t value);
void display_set_all(uint8_t *values);

#endif