/**
 * @file mode_chilliD.h
 * @author Gerald McAlister (GEMISIS)
 * @brief A game where you try to eat as many chilidogs as possible in a time limit.
 * @version 1.0
 * @date 2024-12-31
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

//==============================================================================
// Includes
//==============================================================================

#include "swadge2024.h"

// Make Swadgemode available
extern swadgeMode_t chiliDMode;

typedef enum {
    CHILID_MENU,
    CHILID_GAME
} chiliD_modes;

typedef struct {
    font_t menuFont;
    wsg_t titleScreen;
    wsg_t pressStart;
    chiliD_modes mode;
    bool flashState;
    int64_t previouslyElapsedUs;
    float prevAccelQuat[4];
    int score;
} chiliD_t;