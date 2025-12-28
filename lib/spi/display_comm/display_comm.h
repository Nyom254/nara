#ifndef DISPLAY_COMM_H
#define DISPLAY_COMM_H

void ili9488_init(void);
void spi_dma_init(void);
void lv_port_disp_init(void);
bool lvgl_tick_cb(struct repeating_timer *t);

#endif
