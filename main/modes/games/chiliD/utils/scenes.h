/**
 * @file scenes.h
 * @author Gerald McAlister (GEMISIS)
 * @brief
 * @version 1.0
 * @date 2025-1-1
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include "swadge2024.h"

typedef struct {
    scene_t* next;
    void* context;

    void (*fnSceneStart)(void* context);
    void (*fnSceneLoop)(void* context, int64_t elapsedUs);
    void (*fnSceneEnd)(void* context);
} scene_t;

extern void runScene(scene_t* scene, int64_t elapsedUs);
extern void transitionScene(scene_t* current, scene_t* next);
