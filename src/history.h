#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>

typedef struct {
    char *expression;
    char *result;
} HistoryEntry;

#define MAX_HISTORY 50

void history_init(void);
void history_free(void);
int history_get_count(void);
const HistoryEntry* history_get_entry(int index);
void history_add(const char *expression, const char *result);
void history_clear(void);

#endif /* HISTORY_H */
