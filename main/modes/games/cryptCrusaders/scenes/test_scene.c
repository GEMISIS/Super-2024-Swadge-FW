/**
 * @file test_scene.h
 * @author Gerald McAlister (GEMISIS)
 * @brief A quick test scene, just meant to confirm the API works.
 * @version 1.0
 * @date 2025-3-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "esp_log.h"

#include "test_scene.h"

static int totalTimeElapsedUs = 0;

static void start_test_scene(void* context) {
    ESP_LOGI("Crypt", "Test scene starting");
}

static void loop_test_scene(void* context, int64_t elapsedUs) {
    // Process button events
    buttonEvt_t evt = {0};
    while (checkButtonQueueWrapper(&evt))
    {
    }
    fillDisplayArea(0, 0, TFT_WIDTH, TFT_HEIGHT, c500);

    totalTimeElapsedUs += elapsedUs;

    if (totalTimeElapsedUs > 5 * 1000000) {
        totalTimeElapsedUs = 0;
        ESP_LOGI("Crypt", "Update run for 5 seconds");
    }
}

static void end_test_scene(void* context) {
    ESP_LOGI("Crypt", "Test scene ending");
}

scene_t test_scene = {
    .next            =   NULL,
    .context         =   NULL,
    .fnSceneStart    =   start_test_scene,
    .fnSceneLoop     =   loop_test_scene,
    .fnSceneEnd      =   end_test_scene,
};


