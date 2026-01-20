#pragma once
#include "HAL/hal_mmwave.h"
#include "mmwave_interface/mmwave_core_interface.h"

const hal_mmwave_config* app_mmwave_get_hal_config(void);
const mmWave_core_interface* app_mmwave_get_core_interface(void);