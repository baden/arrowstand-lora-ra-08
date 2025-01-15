#include <stdbool.h>

void board_led_init(void);
void board_led_rgb(bool r, bool g, bool b);
void board_led_cold(bool on);
void board_led_warm(bool on);
