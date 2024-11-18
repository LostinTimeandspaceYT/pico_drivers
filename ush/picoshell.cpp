#include "picoshell.h"

// working buffers allocations (size could be customized)
#define BUF_IN_SIZE 512
#define BUF_OUT_SIZE 512
#define PATH_MAX_SIZE 256

static char ush_in_buf[BUF_IN_SIZE];
static char ush_out_buf[BUF_OUT_SIZE];

// Picoshell instance handler
struct ush_object ush;

// non-blocking read interface
static int ush_read(struct ush_object *self, char *ch) {
  // should be implemented as a FIFO
  if (stdio_get_until(ch, BUF_IN_SIZE, PICO_ERROR_TIMEOUT) > 0) {
    return 1;
  }
  return 0;
}

// non-blocking write interface
static int ush_write(struct ush_object *self, char ch) {
  // should be implemented as a FIFO
  return (printf("%c", ch) == 1);
}

// I/O interface descriptor
static const struct ush_io_interface ush_iface = {
    .read = ush_read,
    .write = ush_write,
};

// Picoshell descriptor
static const struct ush_descriptor ush_desc = {
    .io = &ush_iface,                           // I/O interface pointer
    .input_buffer = ush_in_buf,                 // working input buffer
    .input_buffer_size = sizeof(ush_in_buf),    // working input buffer size
    .output_buffer = ush_out_buf,               // working output buffer
    .output_buffer_size = sizeof(ush_out_buf),  // working output buffer size
    .path_max_length = PATH_MAX_SIZE,           // path maximum length (stack)
    .hostname = "Pico2",                        // hostname (in prompt)
};

extern void picoshell_cmds_add(void);
extern void picoshell_root_mount(void);
extern void picoshell_dev_mount(void);
extern void picoshell_bin_mount(void);

// Add directories as needed.
extern void picoshell_i2c_mount(void);

void picoshell_init(void) {
  //begin serial interface.

  ush_init(&ush, &ush_desc);

  picoshell_cmds_add();

  //NOTE: Root must be mounted first!
  picoshell_root_mount();
  picoshell_dev_mount();
  picoshell_bin_mount();
 /*picoshell_i2c_mount();*/
}

void picoshell_service() {
  ush_service(&ush);
}

