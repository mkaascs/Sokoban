#include "profile.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define PROFILE_STACK_MAX 64

typedef struct {
    const char* name;
    clock_t start_time;
} ProfileFrame;

static ProfileFrame profile_stack[PROFILE_STACK_MAX];
static int stack_count = 0;

static ProfileEntry profile_entries[MAX_PROFILE_ENTRIES];
static int profile_count = 0;

static int profile_find(const char* name) {
    for (int index = 0; index < profile_count; index++)
        if (strcmp(profile_entries[index].name, name) == 0)
            return index;

    return -1;
}

void profile_start(const char* name) {
    if (stack_count > PROFILE_STACK_MAX) {
        printf("stack is full\n");
        return;
    }

    ProfileFrame* frame = &profile_stack[stack_count++];
    frame->name = name;
    frame->start_time = clock();
}

void profile_end() {
    if (stack_count <= 0)
        return;

    const ProfileFrame frame = profile_stack[--stack_count];
    const double elapsed = (double)(clock() - frame.start_time) / CLOCKS_PER_SEC;

    // Ищем/добавляем запись
    int idx = profile_find(frame.name);
    if (idx == -1) {
        idx = profile_count++;
        profile_entries[idx].name = frame.name;
        profile_entries[idx].total_time = 0;
        profile_entries[idx].calls = 0;
    }

    profile_entries[idx].total_time += elapsed;
    profile_entries[idx].calls++;
}

void profile_print() {
    while (stack_count > 0)
        profile_end();

    printf("\n=== FRAME PROFILING ===\n");
    for (int i = 0; i < profile_count; i++) {
        printf("%-40s | total: %12.6f s | calls: %30d | avg: %12.6f ms\n",
            profile_entries[i].name,
            profile_entries[i].total_time,
            profile_entries[i].calls,
            (profile_entries[i].total_time / profile_entries[i].calls) * 1000.0
        );

        profile_entries[i].total_time = 0;
        profile_entries[i].calls = 0;
    }
}