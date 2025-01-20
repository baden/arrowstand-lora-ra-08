#pragma once

#include <stdint.h>
#include <stdbool.h>

void adxl345_init(uint8_t sensitivity);
void adxl345_setSensitivity(uint8_t sensitivity);
bool adxl345_is_active();
