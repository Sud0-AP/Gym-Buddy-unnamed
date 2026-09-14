#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "display_events.h"

extern QueueHandle_t g_display_queue;

void display_task_init();
void display_task_start(UBaseType_t priority = 1, BaseType_t core = 1);
void display_task_render_boot_screen();
