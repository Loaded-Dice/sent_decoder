#include <Arduino.h>
#include "config.h"
#include "hardware_setup.h"
#include "signal_processing.h"
#include "inputs.h"
#include "display.h"
#include "serial_comm.h"
#include "periodic_tasks.h"

void setup() {
  xTaskCreatePinnedToCore(register_isr_on_core0, "isr_reg", 10000, NULL, (configMAX_PRIORITIES - 1), NULL, 0);
  xTaskCreatePinnedToCore(core1_loop, "core1_loop", 4096, NULL, 1, NULL, 1);
  vTaskDelete(NULL);
}

void loop() {}
