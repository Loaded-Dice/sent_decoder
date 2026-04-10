#include "hardware_setup.h"
#include "signal_processing.h"
#include "inputs.h"
#include "display.h"
#include "serial_comm.h"
#include "periodic_tasks.h"
#include "misc_functions.h"
#include "led_status.h"

void IRAM_ATTR sent_isr_handler(void* arg) {
  static uint32_t lastTicks = 0;
  TIMERG0.hw_timer[0].update.val = 1;
  uint32_t currTicks = TIMERG0.hw_timer[0].lo.val;
  uint32_t delta = currTicks - lastTicks;  // Simplified: overflow handling automatic with uint32_t
  
  // Filter out spurious interrupts (< 2µs = 80 ticks @ 40MHz)
  if (delta < 80) {
    REG_WRITE(GPIO_STATUS_W1TC_REG, (1 << SENT_PIN));
    return;
  }
  
  uint32_t nextHead = (ringBuffer.head + 1) & (RING_BUFFER_SIZE - 1);
  if (nextHead != ringBuffer.tail) {
    ringBuffer.buffer[ringBuffer.head] = delta;
    ringBuffer.head = nextHead;
  } 
  else { 
    ringBuffer.overflow = true; 
  }
  
  lastTicks = currTicks;
  REG_WRITE(GPIO_STATUS_W1TC_REG, (1 << SENT_PIN));
}

void core1_loop(void* pvParameters) {
  Serial.begin(UART_BAUD);  // Increased for higher data throughput (was 115200)
  pinMode(FAULT_PIN, INPUT_PULLUP);
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, AUTO_ENABLE_SENSOR);
  setupTft();
  init_pcnt();
  initStatusLed();
  
  while (true) {
    processSignal();
    shortGuard();
    calcStats();
    chkStateUpdates();
    tftMain();
    serialRead();
    periodicUART();
    handleInputs();
    updateStatusLed();

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void register_isr_on_core0(void* arg) {
    esp_task_wdt_deinit();
    setupGptimer0();
    
    if (sent_isr_handle) esp_intr_free(sent_isr_handle);
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SENT_PIN) | (1ULL << ENCA_PIN) | (1ULL << ENCBTN_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    // Configure pins for Arduino digitalRead compatibility
    pinMode(ENCBTN_PIN, INPUT);
    
    ESP_ERROR_CHECK(esp_intr_alloc(ETS_GPIO_INTR_SOURCE, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3, sent_isr_handler, NULL, &sent_isr_handle));
    gpio_set_intr_type(SENT_PIN, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(SENT_PIN);
    vTaskDelete(NULL);
}

void setupGptimer0() {
  TIMERG0.hw_timer[0].config.tx_en = 0;
  TIMERG0.hw_timer[0].config.tx_divider = TIMER_CLK_DIV;
  TIMERG0.hw_timer[0].config.tx_increase = 1;
  TIMERG0.hw_timer[0].config.tx_autoreload = 0;
  TIMERG0.hw_timer[0].loadlo.val = 0;
  TIMERG0.hw_timer[0].loadhi.val = 0;
  TIMERG0.hw_timer[0].load.val = 1;
  TIMERG0.hw_timer[0].config.tx_en = 1;
}

void init_pcnt(void) {
    pcnt_unit_config_t unit_config = {
        .low_limit = -32767,
        .high_limit = 32767,
        .flags = {
            .accum_count = 1,
        },
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    pcnt_chan_config_t chan_config = {
        .edge_gpio_num = ENCA_PIN,
        .level_gpio_num = ENCB_PIN,
    };
    pcnt_channel_handle_t pcnt_chan = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_config, &pcnt_chan));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan, PCNT_CHANNEL_EDGE_ACTION_INCREASE,  PCNT_CHANNEL_EDGE_ACTION_DECREASE ));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE ));

    pcnt_glitch_filter_config_t filter_config = {.max_glitch_ns = 1000, };
   
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));
    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
    ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_NUM_39, GPIO_INTR_DISABLE));
    ESP_ERROR_CHECK(gpio_intr_disable(GPIO_NUM_39));
}

void setupTft(){
    #ifdef DISPLAY_ACTIVE    
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    spr_valch1.setColorDepth(1);
    spr_valch2.setColorDepth(1);
    spr_valsupp.setColorDepth(1);
    spr_valch1.createSprite(60, 20);
    spr_valch2.createSprite(60, 20);
    spr_valsupp.createSprite(60, 20);

    spr_valch1.fillSprite(TFT_TRANSPARENT);
    spr_valch2.fillSprite(TFT_TRANSPARENT);
    spr_valsupp.fillSprite(TFT_TRANSPARENT);

    // Display identifier for 3 seconds
    displayIdentifier();
    #endif
    
    // Send identifier via UART
    //infoMsgJson(IDENTIFY);
}
