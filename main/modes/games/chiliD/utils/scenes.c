/**
 * @file scenes.c
 * @author Gerald McAlister (GEMISIS)
 * @brief
 * @version 1.0
 * @date 2025-1-1
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "scenes.h"

void runScene(scene_t** scene, int64_t elapsedUs) {
    if ((*scene)->next == NULL) {
        if ((*scene)->fnSceneLoop != NULL) {
            (*scene)->fnSceneLoop((*scene)->context, elapsedUs);
        }
    } else {
        scene_t* temp = (*scene)->next;
        (*scene)->next = NULL;

        if ((*scene)->fnSceneEnd != NULL) {
            (*scene)->fnSceneEnd((*scene)->context);
        }

        (*scene) = temp;

        if ((*scene)->fnSceneStart != NULL) {
            (*scene)->fnSceneStart((*scene)->context);
        }
    }
}

extern void transitionScene(scene_t* current, scene_t* next) {
    current->next = next;
}
