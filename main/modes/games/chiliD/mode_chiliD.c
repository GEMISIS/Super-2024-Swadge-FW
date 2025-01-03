/**
 * @file mode_chilliD.h
 * @author Gerald McAlister (GEMISIS)
 * @brief A game where you try to eat as many chillidogs as possible in a time limit.
 * @version 1.0
 * @date 2024-12-31
 *
 * @copyright Copyright (c) 2024
 *
 */

//==============================================================================
// Includes
//==============================================================================

#include "mode_chiliD.h"
#include "esp_log.h"
#include "chiliD_start.h"

//==============================================================================
// Defines
//==============================================================================

#define CG_FRAMERATE 16667
#define SECOND       1000000
#define TIMER        SECOND * 10

//==============================================================================
// Consts
//==============================================================================

static const char chilliDTitle[]      = "Chili Dog Rush"; // Game title

//==============================================================================
// Function declarations
//==============================================================================

/**
 * @brief Constructs the mode
 *
 */
static void chiliDEnterMode(void);

/**
 * @brief Deconstructs the mode
 *
 */
static void chiliDExitMode(void);

/**
 * @brief Main loop of Chowa Grove
 *
 * @param elapsedUs
 */
static void chiliDMainLoop(int64_t elapsedUs);

//==============================================================================
// Variables
//==============================================================================

swadgeMode_t chiliDMode = {
    .modeName                 = chilliDTitle,
    .wifiMode                 = ESP_NOW,
    .overrideUsb              = false,
    .usesAccelerometer        = true,
    .usesThermometer          = false,
    .overrideSelectBtn        = false,
    .fnEnterMode              = chiliDEnterMode,
    .fnExitMode               = chiliDExitMode,
    .fnMainLoop               = chiliDMainLoop,
    .fnAudioCallback          = NULL,
    .fnBackgroundDrawCallback = NULL,
    .fnEspNowRecvCb           = NULL,
    .fnEspNowSendCb           = NULL,
    .fnAdvancedUSB            = NULL,
    .fnDacCb                  = NULL,
};

static chiliD_t* chiliD = NULL;
static scene_t* currentScene = NULL;

static void chiliDEnterMode(void) {
    // Mode memory allocation
    chiliD = heap_caps_calloc(1, sizeof(cGrove_t), MALLOC_CAP_SPIRAM);
    setFrameRateUs(CG_FRAMERATE);
    accelSetRegistersAndReset();

    loadWsg("chiliD_main_menu.wsg", &chiliD->titleScreen, true);
    loadWsg("chiliD_press_start.wsg", &chiliD->pressStart, true);
    loadFont("cg_font_body.font", &chiliD->menuFont, true);

    chiliD->mode = CHILID_MENU;
    chiliD->flashState = false;
    chiliD->previouslyElapsedUs = 0;
    chiliD->score = 0;

    start_scen
}

static void chiliDExitMode(void) {
    freeWsg(&chiliD->titleScreen);
    freeWsg(&chiliD->pressStart);
    freeFont(&chiliD->menuFont);
    heap_caps_free(chiliD);
}

static void chiliDMainLoop(int64_t elapsedUs) {
    if (chiliD->mode == CHILID_MENU) {
        // Process button events
        buttonEvt_t evt = {0};
        while (checkButtonQueueWrapper(&evt))
        {
            if (evt.down && evt.button == PB_A) {
                chiliD->mode = CHILID_GAME;
                chiliD->previouslyElapsedUs = 0;
                fillDisplayArea(0, 0, TFT_WIDTH, TFT_HEIGHT, c000);
                return;
            }
        }
        drawWsgSimple(&chiliD->titleScreen, 0, 0);
        if (chiliD->flashState) {
            drawWsgSimple(&chiliD->pressStart, 6, 160);
        }

        if (chiliD->previouslyElapsedUs >= SECOND) {
            chiliD->flashState = !chiliD->flashState;
            chiliD->previouslyElapsedUs = 0;
        }
        chiliD->previouslyElapsedUs += elapsedUs;
    } else if (chiliD->mode == CHILID_GAME) {
        // Process button events
        buttonEvt_t evt = {0};
        while (checkButtonQueueWrapper(&evt))
        {
            if (evt.down && evt.button == PB_B) {
                chiliD->mode = CHILID_MENU;
                chiliD->previouslyElapsedUs = 0;
                fillDisplayArea(0, 0, TFT_WIDTH, TFT_HEIGHT, c000);
                return;
            }
        }


        fillDisplayArea(0, 0, TFT_WIDTH, TFT_HEIGHT, c000);

        char formatStr[32];

        accelIntegrate();
        if (ESP_OK == accelGetQuaternion(chiliD->prevAccelQuat)) {
            snprintf(formatStr, sizeof(formatStr) - 1, "Accel Quat Vals:");
            drawText(&chiliD->menuFont, c500, formatStr, 0, 240 / 2 - 5 * 1 - 15 * 5);
            snprintf(formatStr, sizeof(formatStr) - 1, "%f", chiliD->prevAccelQuat[0]);
            drawText(&chiliD->menuFont, c500, formatStr, 0, 240 / 2 - 5 * 1 - 15 * 4);
            snprintf(formatStr, sizeof(formatStr) - 1, "%f", chiliD->prevAccelQuat[1]);
            drawText(&chiliD->menuFont, c500, formatStr, 0, 240 / 2 - 5 * 1 - 15 * 3);
            snprintf(formatStr, sizeof(formatStr) - 1, "%f", chiliD->prevAccelQuat[2]);
            drawText(&chiliD->menuFont, c500, formatStr, 0, 240 / 2 - 5 * 1 - 15 * 2);
            snprintf(formatStr, sizeof(formatStr) - 1, "%f", chiliD->prevAccelQuat[3]);
            drawText(&chiliD->menuFont, c500, formatStr, 0, 240 / 2 - 5 * 1 - 15 * 1);
        }

        snprintf(formatStr, sizeof(formatStr) - 1, "Score: %d", chiliD->score);
        drawText(&chiliD->menuFont, c500, formatStr, 280 / 2 - 8 * 5, 240 / 2 - 5 * 1);

        snprintf(formatStr, sizeof(formatStr) - 1, "Time Left: %lld", (TIMER - chiliD->previouslyElapsedUs) / SECOND + 1);
        drawText(&chiliD->menuFont, c500, formatStr, 280 / 2 - 8 * 10, 240 / 2 - 5 * 1 + 15);

        if (chiliD->previouslyElapsedUs >= TIMER) {
            ESP_LOGI("ChiliD", "Game over");
            chiliD->mode = CHILID_MENU;
            chiliD->previouslyElapsedUs = 0;
        }
        chiliD->previouslyElapsedUs += elapsedUs;
    }
}
