//==============================================================================
// Includes
//==============================================================================
#include "esp_random.h"
#include "random_bigbug.h"

//==============================================================================
// Functions
//==============================================================================
int bb_randomInt(int lowerBound, int upperBound)
{
    return esp_random() % (upperBound - lowerBound + 1) + lowerBound;
}