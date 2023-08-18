//==============================================================================
// Includes
//==============================================================================

#include <string.h>
#include <math.h>

#include <esp_timer.h>
#include "sound.h"
#include "hdw-bzr.h"
#include "hdw-mic_emu.h"
#include "emu_main.h"

//==============================================================================
// Defines
//==============================================================================

#define SAMPLING_RATE 8000

//==============================================================================
// Structs
//==============================================================================

/**
 * @brief A buzzer track which a song is played on, either BGM or SFX
 */
typedef struct
{
    const songTrack_t* sChan; ///< The song currently being played on this track
    int32_t note_index;       ///< The note index into the song
    int64_t start_time;       ///< TODO
    bool should_loop;         ///< TODO
} bzrTrack_t;

/**
 * @brief A buzzer, currently either left or right
 */
typedef struct
{
    noteFrequency_t cFreq; ///< The current frequency of the note being played
    uint16_t vol;          ///< The current volume
    bzrTrack_t bgm;        ///< The BGM track for this buzzer
    bzrTrack_t sfx;        ///< The SFX track for this buzzer
} buzzer_t;

//==============================================================================
// Const variables
//==============================================================================

const uint16_t volLevels[] = {
    0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096,
};

//==============================================================================
// Variables
//==============================================================================

/// The sound driver
static struct SoundDriver* soundDriver = NULL;

// Output buzzers
static buzzer_t buzzers[NUM_BUZZERS] = {0};
uint16_t bgmVolume;
uint16_t sfxVolume;

//==============================================================================
// Function Prototypes
//==============================================================================

static bool buzzer_track_check_next_note(bzrTrack_t* track, int16_t buzzer, uint16_t volume, bool isActive);
void buzzer_check_next_note(void* arg);
void EmuSoundCb(struct SoundDriver* sd, short* in, short* out, int samples_R, int samples_W);

//==============================================================================
// Functions
//==============================================================================

/**
 * @brief Initialize the buzzer
 *
 * @param bzrGpioL The GPIO the left buzzer is attached to
 * @param ledcTimerL The LEDC timer used to drive the left buzzer
 * @param ledcChannelL The LEDC channel used to drive the left buzzer
 * @param bzrGpioR The GPIO the right buzzer is attached to
 * @param ledcTimerR The LEDC timer used to drive the right buzzer
 * @param ledcChannelR The LEDC channel used to drive the right buzzer
 * @param bgmVolume Starting background sound volume, 0 to 4096
 * @param sfxVolume Starting effects sound volume, 0 to 4096
 */
void initBuzzer(gpio_num_t bzrGpioL, ledc_timer_t ledcTimerL, ledc_channel_t ledcChannelL, gpio_num_t bzrGpioR,
                ledc_timer_t ledcTimerR, ledc_channel_t ledcChannelR, uint16_t _bgmVolume, uint16_t _sfxVolume)
{
    bzrStop();

    if (!soundDriver)
    {
        soundDriver = InitSound(0, EmuSoundCb, SAMPLING_RATE, 1, 2, 256, 0, 0);
    }

    memset(&buzzers, 0, sizeof(buzzers));
    bgmVolume = _bgmVolume;
    sfxVolume = _sfxVolume;

    const esp_timer_create_args_t checkNoteTimeArgs = {
        .arg                   = NULL,
        .callback              = buzzer_check_next_note,
        .dispatch_method       = ESP_TIMER_TASK,
        .name                  = "BZR",
        .skip_unhandled_events = true,
    };
    esp_timer_handle_t checkNoteTimerHandle = NULL;
    esp_timer_create(&checkNoteTimeArgs, &checkNoteTimerHandle);
    esp_timer_start_periodic(checkNoteTimerHandle, 1);
}

/**
 * @brief Deinitialize the buzzer
 */
void deinitBuzzer(void)
{
    if (soundDriver)
    {
#if defined(_WIN32)
        CloseSound(NULL);
#else
        CloseSound(soundDriver); // when calling this on Windows, it halts
#endif
        soundDriver = NULL;
    }
}

/**
 * @brief Set the buzzer's bgm volume
 *
 * @param vol The background volume, 0 to 13
 */
void bzrSetBgmVolume(uint16_t vol)
{
    bgmVolume = volLevels[vol];
}

/**
 * @brief Set the buzzer's sfx volume
 *
 * @param vol The background volume, 0 to 13
 */
void bzrSetSfxVolume(uint16_t vol)
{
    sfxVolume = volLevels[vol];
}

/**
 * @brief Start playing a background music on the buzzer. This has lower priority
 * than sound effects
 *
 * @param song The song to play as a sequence of notes
 * @param track The track to play BGM on
 */
void bzrPlayBgm(const song_t* song, buzzerPlayTrack_t track)
{
    int64_t startTime = esp_timer_get_time();
    if (1 == song->numTracks)
    {
        // Song is mono, play it accordingly
        switch (track)
        {
            case BZR_STEREO:
            {
                // Play the same thing on both buzzers
                buzzers[BZR_LEFT].bgm.sChan       = &song->tracks[0];
                buzzers[BZR_LEFT].bgm.note_index  = -1;
                buzzers[BZR_LEFT].bgm.start_time  = startTime;
                buzzers[BZR_LEFT].bgm.should_loop = song->shouldLoop;

                buzzers[BZR_RIGHT].bgm.sChan       = &song->tracks[0];
                buzzers[BZR_RIGHT].bgm.note_index  = -1;
                buzzers[BZR_RIGHT].bgm.start_time  = startTime;
                buzzers[BZR_RIGHT].bgm.should_loop = song->shouldLoop;
                break;
            }
            case BZR_LEFT:
            case BZR_RIGHT:
            {
                // Play the mono song on the given track
                buzzers[track].bgm.sChan       = &song->tracks[0];
                buzzers[track].bgm.note_index  = -1;
                buzzers[track].bgm.start_time  = startTime;
                buzzers[track].bgm.should_loop = song->shouldLoop;
                break;
            }
        }
    }
    else
    {
        // Play the stereo song on both buzzers
        buzzers[BZR_LEFT].bgm.sChan       = &song->tracks[0];
        buzzers[BZR_LEFT].bgm.note_index  = -1;
        buzzers[BZR_LEFT].bgm.start_time  = startTime;
        buzzers[BZR_LEFT].bgm.should_loop = song->shouldLoop;

        buzzers[BZR_RIGHT].bgm.sChan       = &song->tracks[1];
        buzzers[BZR_RIGHT].bgm.note_index  = -1;
        buzzers[BZR_RIGHT].bgm.start_time  = startTime;
        buzzers[BZR_RIGHT].bgm.should_loop = song->shouldLoop;
    }
}

/**
 * @brief Start playing a sound effect on the buzzer. This has higher priority
 * than background music
 *
 * @param song The song to play as a sequence of notes
 * @param track The track to play SFX on
 */
void bzrPlaySfx(const song_t* song, buzzerPlayTrack_t track)
{
    int64_t startTime = esp_timer_get_time();
    if (1 == song->numTracks)
    {
        // Song is mono, play it accordingly
        switch (track)
        {
            case BZR_STEREO:
            {
                // Play the same thing on both buzzers
                buzzers[BZR_LEFT].sfx.sChan       = &song->tracks[0];
                buzzers[BZR_LEFT].sfx.note_index  = -1;
                buzzers[BZR_LEFT].sfx.start_time  = startTime;
                buzzers[BZR_LEFT].sfx.should_loop = song->shouldLoop;

                buzzers[BZR_RIGHT].sfx.sChan       = &song->tracks[0];
                buzzers[BZR_RIGHT].sfx.note_index  = -1;
                buzzers[BZR_RIGHT].sfx.start_time  = startTime;
                buzzers[BZR_RIGHT].sfx.should_loop = song->shouldLoop;
                break;
            }
            case BZR_LEFT:
            case BZR_RIGHT:
            {
                // Play the mono song on the given track
                buzzers[track].sfx.sChan       = &song->tracks[0];
                buzzers[track].sfx.note_index  = -1;
                buzzers[track].sfx.start_time  = startTime;
                buzzers[track].sfx.should_loop = song->shouldLoop;
                break;
            }
        }
    }
    else
    {
        // Play the stereo song on both buzzers
        buzzers[BZR_LEFT].sfx.sChan       = &song->tracks[0];
        buzzers[BZR_LEFT].sfx.note_index  = -1;
        buzzers[BZR_LEFT].sfx.start_time  = startTime;
        buzzers[BZR_LEFT].sfx.should_loop = song->shouldLoop;

        buzzers[BZR_RIGHT].sfx.sChan       = &song->tracks[1];
        buzzers[BZR_RIGHT].sfx.note_index  = -1;
        buzzers[BZR_RIGHT].sfx.start_time  = startTime;
        buzzers[BZR_RIGHT].sfx.should_loop = song->shouldLoop;
    }
}

/**
 * @brief Stop the buzzer from playing anything
 */
void bzrStop(void)
{
    memset(buzzers, 0, sizeof(buzzers));
    bzrPlayNote(SILENCE, BZR_LEFT, 0);
    bzrPlayNote(SILENCE, BZR_RIGHT, 0);
}

/////////////////////////////

/**
 * @brief Start playing a single note on the buzzer.
 * This note will play until stopped.
 * This has IRAM_ATTR because it may be called from an interrupt
 *
 * @param freq The frequency of the note
 * @param track The track to play a note on
 * @param volume The volume, 0 to 4096
 */
void bzrPlayNote(noteFrequency_t freq, buzzerPlayTrack_t track, uint16_t volume)
{
    switch (track)
    {
        case BZR_STEREO:
        {
            bzrPlayNote(freq, BZR_LEFT, volume);
            bzrPlayNote(freq, BZR_RIGHT, volume);
            break;
        }
        case BZR_LEFT:
        case BZR_RIGHT:
        {
            buzzers[track].cFreq = freq;
            buzzers[track].vol   = volume;
            break;
        }
    }
}

/**
 * @brief Stop playing a single note on the buzzer
 * This has IRAM_ATTR because it may be called from an interrupt
 *
 * @param track The track to stop
 */
void bzrStopNote(buzzerPlayTrack_t track)
{
    bzrPlayNote(SILENCE, track, 0);
}

////////////////////////////////

/**
 * @brief Call this periodically to check if the next note in the song should be played
 *
 * @param arg unused
 */
void buzzer_check_next_note(void* arg)
{
    for (int16_t cIdx = 0; cIdx < NUM_BUZZERS; cIdx++)
    {
        buzzer_t* buzzer = &buzzers[cIdx];

        bool sfxIsActive = buzzer_track_check_next_note(&buzzer->sfx, cIdx, sfxVolume, true);
        bool bgmIsActive = buzzer_track_check_next_note(&buzzer->bgm, cIdx, bgmVolume, !sfxIsActive);

        // If nothing is playing, but there is BGM (i.e. SFX finished)
        if ((false == sfxIsActive) && (false == bgmIsActive) && (NULL != buzzer->bgm.sChan))
        {
            // Immediately start playing BGM to get back on track faster
            bzrPlayNote(buzzer->bgm.sChan->notes[buzzer->bgm.note_index].note, cIdx, bgmVolume);
        }
    }
}

/**
 * @brief Advance the notes in a specific track and play them if the track is active
 *
 * @param track The track to check notes in
 * @param bIdx The index of the buzzer to play on
 * @param volume The volume to play at
 * @param isActive true if this is active and should set a note to be played
 *                 false to just advance notes without playing
 * @return true  if this track is playing a note
 *         false if it is not
 */
static bool buzzer_track_check_next_note(bzrTrack_t* track, int16_t bIdx, uint16_t volume, bool isActive)
{
    // Check if there is a song and there are still notes
    if ((NULL != track->sChan) && (track->note_index < track->sChan->numNotes))
    {
        // Get the current time
        int64_t cTime = esp_timer_get_time();

        // Check if it's time to play the next note
        if ((-1 == track->note_index)
            || (cTime - track->start_time >= (1000 * track->sChan->notes[track->note_index].timeMs)))
        {
            // Move to the next note
            track->note_index++;
            track->start_time = cTime;

            // Loop if requested
            if (track->should_loop && (track->note_index == track->sChan->numNotes))
            {
                track->note_index = 0;
            }

            // If there is a note
            if (track->note_index < track->sChan->numNotes)
            {
                if (isActive)
                {
                    // Play the note
                    bzrPlayNote(track->sChan->notes[track->note_index].note, bIdx, volume);
                }
            }
            else
            {
                if (isActive)
                {
                    // Song is over
                    bzrStopNote(bIdx);
                }

                track->start_time = 0;
                track->note_index = 0;
                track->sChan      = NULL;
                // Track is inactive
                return false;
            }
        }
        // Track is active
        return true;
    }
    // Track is inactive
    return false;
}

/**
 * @brief Callback for sound events, both input and output
 * Handle output here, pass input to handleSoundInput()
 *
 * @param sd The sound driver
 * @param in A pointer to read samples from. May be NULL
 * @param out A pointer to write samples to. May be NULL
 * @param samples_R The number of samples to read
 * @param samples_W The number of samples to write
 */
void EmuSoundCb(struct SoundDriver* sd, short* in, short* out, int samples_R, int samples_W)
{
    // Pass to microphone
    handleSoundInput(sd, in, out, samples_R, samples_W);

    // If this is an output callback, and there are samples to write
    if (samples_W && out)
    {
        // Keep track of our place in the wave
        static float placeInWave[NUM_BUZZERS] = {0, 0};

        for (int cIdx = 0; cIdx < NUM_BUZZERS; cIdx++)
        {
            // If there is a note to play
            if (buzzers[cIdx].cFreq)
            {
                float transitionPoint = (2 * M_PI * buzzers[cIdx].vol) / 8192;
                // For each sample
                for (int i = 0; i < samples_W; i += 2)
                {
                    // Write the sample, interleaved
                    out[i + cIdx] = 1024 * ((placeInWave[cIdx] < transitionPoint) ? 1 : -1);
                    // Advance the place in the wave
                    placeInWave[cIdx] += ((2 * M_PI * buzzers[cIdx].cFreq) / ((float)SAMPLING_RATE));
                    // Keep it bound between 0 and 2*PI
                    if (placeInWave[cIdx] >= (2 * M_PI))
                    {
                        placeInWave[cIdx] -= (2 * M_PI);
                    }
                }
            }
            else
            {
                // No note to play
                for (int i = 0; i < samples_W; i += 2)
                {
                    // Write the sample, interleaved
                    out[i + cIdx] = 0;
                }
                placeInWave[cIdx] = 0;
            }
        }
    }
}
