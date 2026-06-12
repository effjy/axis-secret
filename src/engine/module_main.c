#include "settings.h"
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/resource.h>

/*
 * Shared secret handed to us by the calculator at spawn time. If we don't
 * receive it, we were run directly rather than triggered from the calculator,
 * so we quietly decline to start.
 */
#define CALC_ENGINE_KEY "31415926.5-a7f3c9"

#include "panel.h"
#include "helpers.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    /* Refuse to run unless launched by the calculator with the shared secret. */
    const char *key = getenv("CALC_ENGINE_KEY");
    if (key == NULL || strcmp(key, CALC_ENGINE_KEY) != 0) {
        fprintf(stderr, "precision-engine: no compute context available\n");
        return 1;
    }
    /* Scrub the token so it can't leak to any child processes we spawn. */
    unsetenv("CALC_ENGINE_KEY");

    /* Disable core dumps for security */
    prctl(PR_SET_DUMPABLE, 0);
    struct rlimit limit = { .rlim_cur = 0, .rlim_max = 0 };
    setrlimit(RLIMIT_CORE, &limit);
    
    /* Initialize libsodium */
    if (sodium_init() < 0) {
        fprintf(stderr, "Error: Failed to initialize libsodium\n");
        return 1;
    }

    printf("%s v%s - %s\n", APP_NAME, APP_VERSION, APP_TITLE);
    
    /* Check for unencrypted swap and warn user */
    if (check_swap_security()) {
        printf("WARNING: Unencrypted swap detected! This may compromise security.\n");
        printf("Consider encrypting swap with LUKS or disabling swap entirely.\n");
        printf("Continuing anyway...\n");
    }
    
    printf("Starting GUI application...\n");
    /* Create and run GUI */
    int status = create_gui(argc, argv);
    
    return status;
}
