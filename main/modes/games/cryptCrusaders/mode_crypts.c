/**
 * @file mode_crypts.c
 * @author Gerald McAlister (GEMISIS)
 * @brief A game where you explore dungeons and collect treasure.
 * @version 1.0
 * @date 2025-3-15
 *
 * @copyright Copyright (c) 2025
 *
 */

//==============================================================================
// Includes
//==============================================================================

#include "mode_crypts.h"
#include "esp_log.h"

//==============================================================================
// Defines
//==============================================================================

#define CG_FRAMERATE 16667
#define SECOND       1000000
#define TIMER        SECOND * 10

//==============================================================================
// Consts
//==============================================================================

static const char cryptCrusadersTitle[]      = "Crypt Crusaders"; // Game title

//==============================================================================
// Function declarations
//==============================================================================

/**
 * @brief Constructs the mode
 *
 */
static void cryptCrusadersEnterMode(void);

/**
 * @brief Deconstructs the mode
 *
 */
static void cryptCrusadersExitMode(void);

/**
 * @brief Main loop of Chowa Grove
 *
 * @param elapsedUs
 */
static void cryptCrusadersMainLoop(int64_t elapsedUs);

//==============================================================================
// Variables
//==============================================================================

swadgeMode_t cryptCrusadersMode = {
    .modeName                 = cryptCrusadersTitle,
    .wifiMode                 = ESP_NOW,
    .overrideUsb              = false,
    .usesAccelerometer        = true,
    .usesThermometer          = false,
    .overrideSelectBtn        = false,
    .fnEnterMode              = cryptCrusadersEnterMode,
    .fnExitMode               = cryptCrusadersExitMode,
    .fnMainLoop               = cryptCrusadersMainLoop,
    .fnAudioCallback          = NULL,
    .fnBackgroundDrawCallback = NULL,
    .fnEspNowRecvCb           = NULL,
    .fnEspNowSendCb           = NULL,
    .fnAdvancedUSB            = NULL,
    .fnDacCb                  = NULL,
};

static void cryptCrusadersEnterMode(void) {
}

static void cryptCrusadersExitMode(void) {
}

static void cryptCrusadersMainLoop(int64_t elapsedUs) {
    // Process button events
    buttonEvt_t evt = {0};
    while (checkButtonQueueWrapper(&evt))
    {
    }
    fillDisplayArea(0, 0, TFT_WIDTH, TFT_HEIGHT, c000);
}