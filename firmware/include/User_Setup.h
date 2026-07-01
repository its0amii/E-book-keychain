// include/User_Setup.h — TFT_eSPI configuration
// ============================================================
// Drop-in replacement for the library's default User_Setup.h.
// Targets the Waveshare ESP32-C6-LCD-1.47 (172x320 ST7789).
// Change the height/width to 240x280 for the 1.69" version.
// ============================================================

#define USER_SETUP_INFO  "E-Book Keychain - ESP32-C6 LCD"
#define ST7789_DRIVER
#define TFT_WIDTH   172
#define TFT_HEIGHT  320
#define TFT_ROTATION 0
#define TFT_INVERSION_ON
#define TFT_RGB_ORDER TFT_RGB
#define TFT_MISO -1
#define TFT_MOSI  6
#define TFT_SCLK  7
#define TFT_CS   14
#define TFT_DC   15
#define TFT_RST  21
#define TFT_BL   22

#define SPI_FREQUENCY        40000000
#define SPI_READ_FREQUENCY   20000000
#define SPI_TOUCH_FREQUENCY   2500000

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

#define ESP32
