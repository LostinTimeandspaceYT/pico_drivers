#include "pico/stdlib.h"
#include "picoshell.h"
#include "ush_node.h"
#include "ush_types.h"

// toggle file execute callback
static void toggle_exec_callback(struct ush_object *self,
                                 struct ush_file_descriptor const *file, int argc,
                                 char *argv[]) {
  // simple toggle led, without any arguments validation
  gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
}

// set file execute callback
static void set_exec_callback(struct ush_object *self,
                              struct ush_file_descriptor const *file, int argc,
                              char *argv[]) {
  // arguments count validation
  if (argc != 2) {
    // return predefined error message
    ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
    return;
  }

  // arguments validation
  if (strcmp(argv[1], "1") == 0) {
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
  } else if (strcmp(argv[1], "0") == 0) {
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
  } else {
    // return predefined error message
    ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
    return;
  }
}

// bin directory handler
static struct ush_node_object bin;


// bin directory files descriptor
static const struct ush_file_descriptor bin_files[] = {
  {
    .name = "toggle",              // toggle file name
    .description = "toggle led",   // optional file description
    .help = "usage: toggle\r\n",   // optional help manual
    .exec = toggle_exec_callback,  // optional execute callback
  },
  {
    .name = "set",  // set file name
    .description = "set led",
    .help = "usage: set {0,1}\r\n",
    .exec = set_exec_callback
  }
};

extern struct ush_object ush;

void picoshell_bin_mount(void) {
  ush_node_mount(&ush, "/bin", &bin, bin_files, sizeof(bin_files) / sizeof(bin_files[0]));
}
