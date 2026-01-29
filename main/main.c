/**
 * @file main.c
 * @author Marko Fuček
 * @brief Minimalni funkcionalni test mmWave API-ja.
 * 
 * Ovaj main služi isključivo za test i prikaz osnovnih funkcionalnosti mmWave
 * aplikacijskog API-ja i ne predstavlja punu aplikaciju.
 * 
 * @note Odkomentirati onu liniju za koji sloj se test želi pokrenuti:
 * [1] HAL test
 * [2] mmWave core test
 * [3] application test
 * 
 * @note Mogu se odkomentirati sve linije ako se žele izvršiti svi testovi.
 * 
 * @version 0.1
 * @date 2026-01-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include <string.h>
#include "tests/test_hal.h"
#include "tests/test_app.h"
#include "tests/test_mmwave_core.h"

void app_main(void)
{
    //hal_mmwave_run_test();
    //mmwave_core_run_test();
    app_mmwave_run_test();

}