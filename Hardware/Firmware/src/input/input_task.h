#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "input_events.h"

extern QueueHandle_t g_input_queue;

void input_task_init();
void input_task_start(UBaseType_t priority = 2, BaseType_t core = 1);
