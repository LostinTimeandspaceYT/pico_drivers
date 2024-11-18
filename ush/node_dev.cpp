#include "picoshell.h"
#include "../utils/common.hpp"

// led file get data callback
size_t led_get_data_callback(struct ush_object *self,
                             struct ush_file_descriptor const *file, uint8_t **data) {
  bool state = gpio_get(PICO_DEFAULT_LED_PIN);
  // return pointer to data
  *data = (uint8_t *)((state) ? "1\r\n" : "0\r\n");
  // return data size
  return strlen((char *)(*data));
}

// led file set data callback
void led_set_data_callback(struct ush_object *self,
                           struct ush_file_descriptor const *file, uint8_t *data,
                           size_t size) {
  // data size validation
  if (size < 1) return;

  // arguments validation
  if (data[0] == '1') {
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
  } else if (data[0] == '0') {
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
  }
}

// time file get data callback
size_t uptime_get_data_callback(struct ush_object *self,
                              struct ush_file_descriptor const *file, uint8_t **data) {
  static char time_buf[16];
  // read current time
  long current_time = millis();
  // convert
  snprintf(time_buf, sizeof(time_buf), "%ld\r\n", current_time);
  time_buf[sizeof(time_buf) - 1] = 0;
  // return pointer to data
  *data = (uint8_t *)time_buf;
  // return data size
  return strlen((char *)(*data));
}


// dev directory handler
static struct ush_node_object dev;

// dev directory files descriptor
static const struct ush_file_descriptor dev_files[] = {
  {
    .name = "led",
    .description = NULL,
    .help = NULL,
    .exec = NULL,
    .get_data = led_get_data_callback,  // optional data getter callback
    .set_data = led_set_data_callback,  // optional data setter callback
  },
  {
    .name = "uptime",
    .description = NULL,
    .help = NULL,
    .exec = NULL,
    .get_data = uptime_get_data_callback,
  },
};

extern struct ush_object ush;

void picoshell_dev_mount(void) {
  ush_node_mount(&ush, "/dev", &dev, dev_files, sizeof(dev_files) / sizeof(dev_files[0]));
}
