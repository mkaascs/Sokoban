#include "profile.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static ProfileEntry profile_entries[MAX_PROFILE_ENTRIES];
static int profile_count = 0;

static clock_t prof_start_time;

static int profile_find(const char* name) {
    for (int index = 0; index < profile_count; index++)
        if (strcmp(profile_entries[index].name, name) == 0)
            return index;

    return -1;
}

void profile_start() {
    prof_start_time = clock();
}

void profile_end(const char* name) {
    const double elapsed = (double)(clock() - prof_start_time) / CLOCKS_PER_SEC;

    int index = profile_find(name);
    if (index == -1) {
        index = profile_count++;
        profile_entries[index].name = name;
        profile_entries[index].total_time = 0;
        profile_entries[index].calls = 0;
    }

    profile_entries[index].total_time += elapsed;
    profile_entries[index].calls += 1;
}

void profile_print() {
    printf("\n=== FRAME PROFILING ===\n");
    for (int i = 0; i < profile_count; i++) {
        printf("%-25s | total: %8.6f s | calls: %4d | avg: %8.6f ms\n",
            profile_entries[i].name,
            profile_entries[i].total_time,
            profile_entries[i].calls,
            (profile_entries[i].total_time / profile_entries[i].calls) * 1000.0
        );

        profile_entries[i].total_time = 0;
        profile_entries[i].calls = 0;
    }
}