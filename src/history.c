#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>

static HistoryEntry history_list[MAX_HISTORY];
static int history_count = 0;
static char *history_file_path = NULL;

static void ensure_history_dir(void) {
    const char *config_dir = g_get_user_config_dir();
    char *dir = g_build_filename(config_dir, "calculator", NULL);
    g_mkdir_with_parents(dir, 0755);
    history_file_path = g_build_filename(dir, "history.txt", NULL);
    g_free(dir);
}

void history_init(void) {
    ensure_history_dir();
    
    for (int i = 0; i < MAX_HISTORY; i++) {
        history_list[i].expression = NULL;
        history_list[i].result = NULL;
    }
    history_count = 0;
    
    FILE *f = g_fopen(history_file_path, "r");
    if (!f) return;
    
    char line_expr[1024];
    char line_res[1024];
    while (fgets(line_expr, sizeof(line_expr), f) && fgets(line_res, sizeof(line_res), f)) {
        line_expr[strcspn(line_expr, "\n")] = '\0';
        line_res[strcspn(line_res, "\n")] = '\0';
        
        if (history_count < MAX_HISTORY) {
            history_list[history_count].expression = g_strdup(line_expr);
            history_list[history_count].result = g_strdup(line_res);
            history_count++;
        } else {
            g_free(history_list[0].expression);
            g_free(history_list[0].result);
            for (int i = 1; i < MAX_HISTORY; i++) {
                history_list[i-1] = history_list[i];
            }
            history_list[MAX_HISTORY-1].expression = g_strdup(line_expr);
            history_list[MAX_HISTORY-1].result = g_strdup(line_res);
        }
    }
    fclose(f);
}

void history_free(void) {
    for (int i = 0; i < history_count; i++) {
        g_free(history_list[i].expression);
        g_free(history_list[i].result);
        history_list[i].expression = NULL;
        history_list[i].result = NULL;
    }
    history_count = 0;
    if (history_file_path) {
        g_free(history_file_path);
        history_file_path = NULL;
    }
}

int history_get_count(void) {
    return history_count;
}

const HistoryEntry* history_get_entry(int index) {
    if (index < 0 || index >= history_count) return NULL;
    return &history_list[index];
}

static void save_history_to_file(void) {
    if (!history_file_path) return;
    FILE *f = g_fopen(history_file_path, "w");
    if (!f) return;
    
    for (int i = 0; i < history_count; i++) {
        fprintf(f, "%s\n%s\n", history_list[i].expression, history_list[i].result);
    }
    fclose(f);
}

void history_add(const char *expression, const char *result) {
    if (history_count < MAX_HISTORY) {
        history_list[history_count].expression = g_strdup(expression);
        history_list[history_count].result = g_strdup(result);
        history_count++;
    } else {
        g_free(history_list[0].expression);
        g_free(history_list[0].result);
        for (int i = 1; i < MAX_HISTORY; i++) {
            history_list[i-1] = history_list[i];
        }
        history_list[MAX_HISTORY-1].expression = g_strdup(expression);
        history_list[MAX_HISTORY-1].result = g_strdup(result);
    }
    save_history_to_file();
}

void history_clear(void) {
    for (int i = 0; i < history_count; i++) {
        g_free(history_list[i].expression);
        g_free(history_list[i].result);
        history_list[i].expression = NULL;
        history_list[i].result = NULL;
    }
    history_count = 0;
    
    if (history_file_path) {
        FILE *f = g_fopen(history_file_path, "w");
        if (f) fclose(f);
    }
}
