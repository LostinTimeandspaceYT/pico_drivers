#include "pico/stdlib.h"
#include "picoshell.h"
#include "ush_node.h"

// info file get data callback
size_t info_get_data_callback(struct ush_object *self,
                              struct ush_file_descriptor const *file, uint8_t **data) {
  static const char *info = "Use Picoshell and make fun!\r\n";

  // return pointer to data
  *data = (uint8_t *)info;
  // return data size
  return strlen(info);
}

// root directory handler
static struct ush_node_object root;

extern struct ush_object ush;

// root directory files descriptor
static const struct ush_file_descriptor root_files[] = {
  {
    .name = "info.txt",  // info.txt file name
    .description = NULL,
    .help = NULL,
    .exec = NULL,
    .get_data = info_get_data_callback,
  }
};

void picoshell_root_mount(void) {
  ush_node_mount(&ush, "/", &root, root_files, sizeof(root_files) / sizeof(root_files[0]));
}
