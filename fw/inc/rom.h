#ifndef HAIERCTRL_ROM_H
#define HAIERCTRL_ROM_H

#include <zephyr/drivers/w1.h>

void read_id(struct w1_rom *rom);

#endif // !HAIERCTRL_ROM_H
