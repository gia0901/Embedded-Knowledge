/* drv_panel_chipA — driver that cua mot chip: tu dang ky ham vao bang luc "insmod".
 * O day "insmod" = constructor chay khi libdisplay.so duoc nap. */
#include <stdio.h>
#include "panel_ops.h"

static int a_init(void) {
    return 0;
}

static int a_set_freq(int hz) {
    printf("  [chipA] set_freq(%d)\n", hz);
    return 0;
}

static int a_set_brightness(int l) {
    printf("  [chipA] set_brightness(%d)\n", l);
    return 0;
}

// ghép bảng
static const struct panel_ops chipA_ops = {
    .size = sizeof(struct panel_ops),
    .init = a_init,
    .set_freq = a_set_freq,
    .set_brightness = a_set_brightness,
};

__attribute__((constructor(101))) /* 101: chay TRUOC lib_init (mac dinh 65535) */
static void drv_panel_chipA_insmod(void) {
    panel_register_ops(&chipA_ops); // truyền bảng từ ChipX về driver nền
}
