#include "picoshell.h"

#ifdef USH_ENABLE_NODE_DISPLAY_SSD1306

#include "ush_commands.h"
#include "ush_node.h"

#include <pico_drivers/display/ssd1306/ssd1306.hpp>


OLED &get_oled_display();
extern struct ush_object ush;

void display_print_exec(struct ush_object *self, struct ush_file_descriptor const *file, int argc,
                         char *argv[]);
void display_draw_shape_exec(struct ush_object *self, struct ush_file_descriptor const *file, int argc,
                             char *argv[]);
void display_clear_exec(struct ush_object *self, struct ush_file_descriptor const *file, int argc,
                        char *argv[]);
void display_rotate_exec(struct ush_object *self, struct ush_file_descriptor const *file, int argc,
                         char *argv[]);

static struct ush_node_object display_commands_node;
static struct ush_node_object display_dir_node;

static const struct ush_file_descriptor display_command_files[] = {
    {
        .name = "print",
        .description = "print text to OLED",
        .help = "printf <text...>\r\n",
        .exec = display_print_exec,
    },
    {
        .name = "draw_shape",
        .description = "draw a shape on the OLED",
        .help = "draw_shape <filled_rect|outline_rect|filled_circle> <x> <y> <a> [b]\r\n",
        .exec = display_draw_shape_exec,
    },
    {
        .name = "clear_display",
        .description = "clear the OLED buffer",
        .help = "clear the display\r\n",
        .exec = display_clear_exec,
    },
    {
        .name = "rotate_display",
        .description = "set OLED rotation",
        .help = "rotate the display <0|90|180|270>\r\n",
        .exec = display_rotate_exec,
    },
};

static bool consume_flag(int &argc, char *argv[], const char *flag) {
    if (argc > 1 && strcmp(argv[argc - 1], flag) == 0) {
        --argc;
        return true;
    }
    return false;
}

static bool parse_int_argument(const char *arg, int32_t min, int32_t max, int32_t &out) {
    char *end = nullptr;
    const long value = strtol(arg, &end, 10);
    if ((end == nullptr) || (end == arg) || (*end != '\0')) {
        return false;
    }
    if (value < min || value > max) {
        return false;
    }
    out = static_cast<int32_t>(value);
    return true;
}

void display_print_exec(struct ush_object *self, struct ush_file_descriptor const *file,
                         int argc, char *argv[]) {
    (void)file;

    if (argc < 2) {
        ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
        return;
    }

    static constexpr size_t TEXT_BUFFER_SIZE = 160;
    char text_buffer[TEXT_BUFFER_SIZE];
    size_t cursor = 0;

    for (int i = 1; i < argc; ++i) {
        const size_t part_len = strlen(argv[i]);
        const bool needs_space = (i + 1 < argc);
        if (cursor + part_len + (needs_space ? 1 : 0) >= TEXT_BUFFER_SIZE) {
            ush_print(self, (char *)"ERROR: text too long\r\n");
            return;
        }

        memcpy(text_buffer + cursor, argv[i], part_len);
        cursor += part_len;
        if (needs_space) {
            text_buffer[cursor++] = ' ';
        }
    }
    text_buffer[cursor] = '\0';


    OLED &display = get_oled_display();
    display.clear_buffer();
    display.set_cursor(0, 0);
    display.set_text_wrap(true);
    display.printf("%s", text_buffer);
    display.set_text_wrap(false);
    display.show();


    ush_print(self, (char *)"OK\r\n");
}

void display_draw_shape_exec(struct ush_object *self, struct ush_file_descriptor const *file,
                             int argc, char *argv[]) {
    (void)file;

    if (argc < 2) {
        ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
        return;
    }

    const char *type = argv[1];
    int needed_args = 0;
    enum class ShapeKind : uint8_t { FilledRect, OutlineRect, FilledCircle };
    ShapeKind kind;

    if ((strcmp(type, "filled_rect") == 0) || (strcmp(type, "rect") == 0)) {
        kind = ShapeKind::FilledRect;
        needed_args = 6;
    } else if ((strcmp(type, "outline_rect") == 0) || (strcmp(type, "rect_outline") == 0)) {
        kind = ShapeKind::OutlineRect;
        needed_args = 6;
    } else if ((strcmp(type, "filled_circle") == 0) || (strcmp(type, "circle") == 0)) {
        kind = ShapeKind::FilledCircle;
        needed_args = 5;
    } else {
        ush_print(self, (char *)"ERROR: unknown shape type\r\n");
        return;
    }

    if (argc != needed_args) {
        ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
        return;
    }

    OLED &display = get_oled_display();
    const int32_t display_width = display.width_px();
    const int32_t display_height = display.height_px();

    int32_t x = 0;
    int32_t y = 0;
    int32_t a = 0;
    int32_t b = 0;

    if (!parse_int_argument(argv[2], 0, display_width - 1, x) ||
        !parse_int_argument(argv[3], 0, display_height - 1, y)) {
        ush_print(self, (char *)"ERROR: invalid coordinates\r\n");
        return;
    }

    switch (kind) {
    case ShapeKind::FilledRect:
    case ShapeKind::OutlineRect:
        if (!parse_int_argument(argv[4], 1, display_width, a) ||
            !parse_int_argument(argv[5], 1, display_height, b)) {
            ush_print(self, (char *)"ERROR: invalid dimensions\r\n");
            return;
        }
        if ((x + a) > display_width || (y + b) > display_height) {
            ush_print(self, (char *)"ERROR: shape exceeds display bounds\r\n");
            return;
        }
        break;
    case ShapeKind::FilledCircle:
        if (!parse_int_argument(argv[4], 1, display_width, a)) {
            ush_print(self, (char *)"ERROR: invalid radius\r\n");
            return;
        }
        b = a;
        const int32_t diameter = a * 2;
        if ((x + diameter) > display_width || (y + diameter) > display_height) {
            ush_print(self, (char *)"ERROR: shape exceeds display bounds\r\n");
            return;
        }
        break;
    }


    switch (kind) {
    case ShapeKind::FilledRect:
        display.draw_filled_rectangle(static_cast<uint8_t>(x), static_cast<uint8_t>(y),
                                      static_cast<uint8_t>(a), static_cast<uint8_t>(b));
        break;
    case ShapeKind::OutlineRect:
        display.draw_rectangle(static_cast<uint8_t>(x), static_cast<uint8_t>(y),
                               static_cast<uint8_t>(a), static_cast<uint8_t>(b));
        break;
    case ShapeKind::FilledCircle: {
        const uint8_t radius = static_cast<uint8_t>(a);
        const uint8_t center_x = static_cast<uint8_t>(x + radius);
        const uint8_t center_y = static_cast<uint8_t>(y + radius);
        display.draw_filled_circle(center_x, center_y, radius);
        break;
    }
    }

    display.show();


    ush_print(self, (char *)"OK\r\n");
}

void display_clear_exec(struct ush_object *self, struct ush_file_descriptor const *file, int argc,
                        char *argv[]) {
    (void)file;

    if (argc != 1) {
        ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
        return;
    }

    OLED &display = get_oled_display();
    display.clear_buffer();
    display.show();


    ush_print(self, (char *)"OK\r\n");
}

void display_rotate_exec(struct ush_object *self, struct ush_file_descriptor const *file, int argc,
                         char *argv[]) {
    (void)file;

    if (argc != 2) {
        ush_print_status(self, USH_STATUS_ERROR_COMMAND_WRONG_ARGUMENTS);
        return;
    }

    int32_t angle = 0;
    if (!parse_int_argument(argv[1], 0, 270, angle)) {
        ush_print(self, (char *)"ERROR: invalid angle\r\n");
        return;
    }

    OLED::Rotation rotation = OLED::Rotation::Deg0;
    switch (angle) {
    case 0:
        rotation = OLED::Rotation::Deg0;
        break;
    case 90:
        rotation = OLED::Rotation::Deg90;
        break;
    case 180:
        rotation = OLED::Rotation::Deg180;
        break;
    case 270:
        rotation = OLED::Rotation::Deg270;
        break;
    default:
        ush_print(self, (char *)"ERROR: angle must be 0, 90, 180, or 270\r\n");
        return;
    }

    OLED &display = get_oled_display();
    display.set_rotation(rotation);
    display.clear_buffer();
    display.show();


    ush_print(self, (char *)"OK\r\n");
}


void picoshell_display_ssd1306_mount(void) {
    const size_t file_count =
        sizeof(display_command_files) / sizeof(display_command_files[0]);

    ush_commands_add(&ush, &display_commands_node, display_command_files, file_count);
    ush_node_mount(&ush, "/display", &display_dir_node, display_command_files, file_count);
}

#else

void picoshell_display_ssd1306_mount(void) {}

#endif
