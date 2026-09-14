#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "nav_stack.h"

extern NavStack g_nav_stack;

void app_task_init();
void app_task_start(UBaseType_t priority = 1, BaseType_t core = 1);
const NavigationState& app_get_current_nav_state();
