/**
 * @file chiliD_start.c
 * @author Gerald McAlister (GEMISIS)
 * @brief
 * @version 1.0
 * @date 2025-1-1
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "chiliD_start.h"

void chiliD_startMenu(void* context) {
    // TODO
}

void chiliD_menuLoop(void* context, int64_t elapsedUs) {
    // TODO
}

void chiliD_endMenu(void* context) {
    // TODO
}


scene_t start_scene = {
    next            =   NULL,
    context         =   NULL,
    fnSceneStart    =   chiliD_startMenu,
    fnSceneLoop     =   chiliD_menuLoop,
    fnSceneEnd      =   chiliD_endMenu,
}