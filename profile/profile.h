#ifndef PROFILE_H
#define PROFILE_H

#define MAX_PROFILE_ENTRIES 64

typedef struct ProfileEntry {
    const char* name;
    double total_time;
    int calls;
} ProfileEntry;

void profile_start();
void profile_end(const char* name);
void profile_print();

#endif //PROFILE_H
