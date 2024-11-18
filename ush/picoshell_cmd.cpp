#include "picoshell.h"

#define AIRCR_Register (*((volatile uint32_t*)(PPB_BASE + 0x0ED0C))) //for reboot

// reboot cmd file execute callback
static void reboot_exec_callback(struct ush_object *self,
                                 struct ush_file_descriptor const *file, int argc,
                                 char *argv[]) {
  // src: https://forums.raspberrypi.com/viewtopic.php?t=318747
  AIRCR_Register = 0x5FA0004;
}

// cmd commands handler
static struct ush_node_object cmd;

// cmd files descriptor
static const struct ush_file_descriptor cmd_files[] = {
  {
    .name = "reboot",
    .description = "reboot device",
    .help = NULL,
    .exec = reboot_exec_callback,
  },
};

extern struct ush_object ush;

void picoshell_cmds_add(void) {
    ush_commands_add(&ush, &cmd, cmd_files, sizeof(cmd_files) / sizeof(cmd_files[0]));
}
