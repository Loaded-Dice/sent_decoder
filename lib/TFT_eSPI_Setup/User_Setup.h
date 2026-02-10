// Setup_SentDecoder_ST7735.h - Original Konfiguration aus Arduino IDE

#define USER_SETUP_ID 0xFFFFFFFF
#define DISABLE_ALL_LIBRARY_WARNINGS

#define ST7735_DRIVER
#define ST7735_GREENTAB2 // ST7735_GREENTAB2 ST7735_REDTAB

#define TFT_WIDTH  128
#define TFT_HEIGHT 160

// ESP32 Pin-Konfiguration für SENT Decoder
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   18
#define TFT_DC   17
#define TFT_RST  16
#define TFT_MISO 12
#define TFT_BL   19
#define TFT_BACKLIGHT_ON HIGH

// Fonts
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// SPI Frequenz
#define SPI_FREQUENCY  40000000
