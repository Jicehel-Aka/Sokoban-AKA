/*
 * platform.cpp - see platform.h.
 * SPDX-License-Identifier: MIT
 */
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef AKA_PC
// ---------------------------------------------------------------------------- PC / SDL2
// Paths longer than the buffers are simply cut (the open then fails cleanly).
#pragma GCC diagnostic ignored "-Wformat-truncation"
#include "pc_backend.h"
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <unistd.h>
#define MKDIR(p) mkdir((p), 0777)
#endif

namespace plat {

static char s_data[640];
static char s_save[640];

static bool is_dir(const char* p) { struct stat st; return stat(p, &st) == 0 && (st.st_mode & S_IFDIR); }

static bool writable(const char* dir)
{
    char t[600];
    snprintf(t, sizeof t, "%s/.write_test", dir);
    FILE* f = fopen(t, "wb");
    if (!f) return false;
    fclose(f);
    remove(t);
    return true;
}

void init()
{
    // Data folder: --data DIR, else <exe>/SOKOBAN, else <exe>/../SOKOBAN, else the source tree layout,
    // else <exe>/../share/sokoban/SOKOBAN (Linux package).
    const char* exe = aka_pc_exe_dir();
    const char* ov = aka_pc_data_dir_override();
    char cand[4][600];
    int n = 0;
    if (ov && *ov) snprintf(cand[n++], 600, "%s", ov);
    snprintf(cand[n++], 600, "%s/SOKOBAN", exe);
    snprintf(cand[n++], 600, "%s/../SD_files/SOKOBAN", exe);
    snprintf(cand[n++], 600, "%s/../share/sokoban/SOKOBAN", exe);
    snprintf(s_data, sizeof s_data, "%s", cand[0]);
    for (int i = 0; i < n; ++i)
        if (is_dir(cand[i])) { snprintf(s_data, sizeof s_data, "%s", cand[i]); break; }

    // Save folder: --save DIR, else the data folder when writable, else the user's home.
    const char* sv = aka_pc_save_dir_override();
    if (sv && *sv) {
        snprintf(s_save, sizeof s_save, "%s", sv);
        make_dir(s_save);
    } else if (writable(s_data)) {
        snprintf(s_save, sizeof s_save, "%s", s_data);
    } else {
        const char* home = getenv("HOME");
        if (!home) home = getenv("USERPROFILE");
        if (!home) home = ".";
        snprintf(s_save, sizeof s_save, "%s/.sokoban-aka", home);
        make_dir(s_save);
    }
}

const char* data_dir() { return s_data; }
const char* save_dir() { return s_save; }
void make_dir(const char* path) { if (!is_dir(path)) MKDIR(path); }
void return_to_loader() { exit(0); }
void* mutex_create() { return aka_pc_mutex_create(); }
void mutex_lock(void* m) { aka_pc_mutex_lock(m); }
void mutex_unlock(void* m) { aka_pc_mutex_unlock(m); }
void start_audio_pump(void (*pump)(void)) { aka_pc_set_audio_pump(pump); }
void run_game(void (*fn)(void)) { fn(); }

}  // namespace plat

#else
// ---------------------------------------------------------------------------- Gamebuino AKA (ESP-IDF)
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>

namespace plat {

static const char* kDir = "/sdcard/SOKOBAN";

void init() {}
const char* data_dir() { return kDir; }
const char* save_dir() { return kDir; }
void make_dir(const char* path) { struct stat st; if (stat(path, &st) != 0) mkdir(path, 0777); }

void return_to_loader()
{
    // The launcher lives in the OTA_1 partition (same convention as the other AKA games).
    const esp_partition_t* loader = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, nullptr);
    if (loader) {
        esp_ota_set_boot_partition(loader);
        esp_restart();
    }
    for (;;) vTaskDelay(pdMS_TO_TICKS(1000));      // no launcher installed: stay here
}

void* mutex_create() { return xSemaphoreCreateMutex(); }
void mutex_lock(void* m) { xSemaphoreTake((SemaphoreHandle_t)m, portMAX_DELAY); }
void mutex_unlock(void* m) { xSemaphoreGive((SemaphoreHandle_t)m); }

static void (*s_pump)(void) = nullptr;
static void audio_task(void*)
{
    for (;;) {
        s_pump();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
void start_audio_pump(void (*pump)(void))
{
    s_pump = pump;
    xTaskCreatePinnedToCore(audio_task, "AudioTask", 4096, nullptr, 5, nullptr, 0);
}

static void (*s_game)(void) = nullptr;
static void game_task(void*) { s_game(); vTaskDelete(nullptr); }
void run_game(void (*fn)(void))
{
    s_game = fn;
    xTaskCreatePinnedToCore(game_task, "GameTask", 16384, nullptr, 5, nullptr, 1);
}

}  // namespace plat
#endif

namespace plat {

void data_path(char* out, size_t n, const char* sub) { snprintf(out, n, "%s/%s", data_dir(), sub); }
void save_path(char* out, size_t n, const char* name) { snprintf(out, n, "%s/%s", save_dir(), name); }
bool file_exists(const char* path) { struct stat st; return stat(path, &st) == 0; }

}  // namespace plat
