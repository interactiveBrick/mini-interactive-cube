#ifndef _USER_COMM_H_
#define _USER_COMM_H_

void comm_init();
void comm_update();
void comm_debuginfo();

void comm_send_key(uint8_t btn, uint8_t down);
void comm_send_osc(uint8_t *bytes, uint8_t len);

#endif