#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "lib/lvgl/lvgl.h"


// ================= DMA CONFIG =================
static int spi_dma_chan;
static dma_channel_config spi_dma_cfg;


// ================= FLUSH HANDLER =================
static volatile lv_display_t * flushing_disp = NULL;

// ================= LVGL BUFFER =================
#define TFT_HOR_RES 480
#define TFT_VER_RES 320
#define LV_LINE_COUNT 40   // 20–60 is ideal

static lv_color_t lv_buf1[TFT_HOR_RES * LV_LINE_COUNT];
static lv_color_t lv_buf2[TFT_HOR_RES * LV_LINE_COUNT];


// ================= PIN CONFIG =================
#define TFT_SPI       spi0
#define TFT_SCK       18
#define TFT_MOSI      19
#define TFT_CS        20
#define TFT_DC        21
#define TFT_RST       26

// ================= DISPLAY CONFIG =================
#define TFT_WIDTH   320
#define TFT_HEIGHT  480
#define SPI_BAUDRATE 40000000  // 40 MHz (ILI9488 SPI max ~50MHz)

// ================= LOW LEVEL =================
static inline void tft_select()   { gpio_put(TFT_CS, 0); }
static inline void tft_deselect() { gpio_put(TFT_CS, 1); }
static inline void tft_cmd_mode() { gpio_put(TFT_DC, 0); }
static inline void tft_data_mode(){ gpio_put(TFT_DC, 1); }

static void tft_write_cmd(uint8_t cmd) {
    tft_cmd_mode();
    tft_select();
    spi_write_blocking(TFT_SPI, &cmd, 1);
    tft_deselect();
}

static void tft_write_data(const uint8_t *data, size_t len) {
    tft_data_mode();
    tft_select();
    spi_write_blocking(TFT_SPI, data, len);
    tft_deselect();
}

static void tft_write_u8(uint8_t data) {
    tft_write_data(&data, 1);
}

static void tft_write_u16(uint16_t data) {
    uint8_t buf[2] = { data >> 8, data & 0xFF };
    tft_write_data(buf, 2);
}


void __isr dma_handler(void) {
    dma_hw->ints0 = 1u << spi_dma_chan;   // clear IRQ

    tft_deselect();

    if(flushing_disp) {
        lv_display_flush_ready((lv_display_t *)flushing_disp);
        flushing_disp = NULL;
    }
}

// ================= INIT =================
void ili9488_reset(void) {
    gpio_put(TFT_RST, 0);
    sleep_ms(20);
    gpio_put(TFT_RST, 1);
    sleep_ms(150);
}


void ili9488_init(void) {
    // GPIO init
    spi_init(TFT_SPI, SPI_BAUDRATE);
    spi_set_format(TFT_SPI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(TFT_SCK, GPIO_FUNC_SPI);
    gpio_set_function(TFT_MOSI, GPIO_FUNC_SPI);

    gpio_init(TFT_CS);
    gpio_init(TFT_DC);
    gpio_init(TFT_RST);

    gpio_set_dir(TFT_CS, GPIO_OUT);
    gpio_set_dir(TFT_DC, GPIO_OUT);
    gpio_set_dir(TFT_RST, GPIO_OUT);

    gpio_put(TFT_CS, 1);
    gpio_put(TFT_DC, 1);
    gpio_put(TFT_RST, 1);

    ili9488_reset();

    // ---- ILI9488 init sequence ----
    tft_write_cmd(0x01); // Software reset
    sleep_ms(150);

    tft_write_cmd(0xE0); // PGAMCTRL
    uint8_t pgamma[] = {0x00,0x03,0x09,0x08,0x16,0x0A,0x3F,0x78,0x4C,0x09,0x0A,0x08,0x16,0x1A,0x0F};
    tft_write_data(pgamma, sizeof(pgamma));

    tft_write_cmd(0xE1); // NGAMCTRL
    uint8_t ngamma[] = {0x00,0x16,0x19,0x03,0x0F,0x05,0x32,0x45,0x46,0x04,0x0E,0x0D,0x35,0x37,0x0F};
    tft_write_data(ngamma, sizeof(ngamma));

    tft_write_cmd(0xC0); // Power Control 1
    tft_write_u8(0x17);
    tft_write_u8(0x15);

    tft_write_cmd(0xC1); // Power Control 2
    tft_write_u8(0x41);

    tft_write_cmd(0xC5); // VCOM
    tft_write_u8(0x00);
    tft_write_u8(0x12);
    tft_write_u8(0x80);

    tft_write_cmd(0x36); // Memory Access Control
    tft_write_u8(0x28); // MX, RGB

    tft_write_cmd(0x3A); // Pixel format
    tft_write_u8(0x66); // RGB666

    tft_write_cmd(0xB0); // Interface Mode
    tft_write_u8(0x00);

    tft_write_cmd(0xB1); // Frame Rate
    tft_write_u8(0xA0);

    tft_write_cmd(0xB4); // Display inversion
    tft_write_u8(0x02);

    tft_write_cmd(0xB6); // Display function
    uint8_t df[] = {0x02,0x02};
    tft_write_data(df, 2);

    tft_write_cmd(0xE9);
    tft_write_u8(0x00);

    tft_write_cmd(0xF7);
    tft_write_u8(0xA9);
    tft_write_u8(0x51);
    tft_write_u8(0x2C);
    tft_write_u8(0x82);

    tft_write_cmd(0x11); // Sleep out
    sleep_ms(120);

    tft_write_cmd(0x29); // Display ON
}

// ================= DRAWING =================
void ili9488_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    tft_write_cmd(0x2A); // CASET
    tft_write_u16(x0);
    tft_write_u16(x1);

    tft_write_cmd(0x2B); // PASET
    tft_write_u16(y0);
    tft_write_u16(y1);

    tft_write_cmd(0x2C); // RAMWR
}


// ======= DMA with irq accelerated fill screen ======
void spi_dma_init(void) {
    spi_dma_chan = dma_claim_unused_channel(true);
    spi_dma_cfg = dma_channel_get_default_config(spi_dma_chan);

    channel_config_set_transfer_data_size(&spi_dma_cfg, DMA_SIZE_8);
    channel_config_set_dreq(&spi_dma_cfg, spi_get_dreq(TFT_SPI, true));
    channel_config_set_read_increment(&spi_dma_cfg, true);
    channel_config_set_write_increment(&spi_dma_cfg, false);

    dma_channel_set_irq0_enabled(spi_dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

static void spi_dma_write_async(const uint8_t *data, size_t len) {
    dma_channel_configure(
        spi_dma_chan,
        &spi_dma_cfg,
        &spi_get_hw(TFT_SPI)->dr,
        data,
        len,
        true
    );
}




// ================= LVGL INTERFACE =================

bool lvgl_tick_cb(struct repeating_timer *t) {
    lv_tick_inc(1);
    return true;
}


static void lvgl_flush_cb(
    lv_display_t * disp,
    const lv_area_t * area,
    uint8_t * px_map
) {
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    flushing_disp = disp;

    ili9488_set_window(area->x1, area->y1, area->x2, area->y2);

    tft_data_mode();
    tft_select();

    spi_dma_write_async(px_map, w * h * 3);
}




void lv_port_disp_init(void) {
    lv_display_t * disp;

    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);

    lv_display_set_flush_cb(disp, lvgl_flush_cb);

    lv_display_set_buffers(
        disp,
        lv_buf1,
        lv_buf2,
        sizeof(lv_buf1),
        LV_DISPLAY_RENDER_MODE_PARTIAL
    );
}
