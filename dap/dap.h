#ifndef _DAP_H
#define _DAP_H

#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dipc.h"

#define KEY_MAIN      8100
#define KEY_LOCK      8200
#define BARRIER_START 8300
#define BARRIER_END   8400

#define DAP_ROUNDS    10000
#define DAP_PAGES     256
#define DAP_PAGE_SIZE 4096
#define DAP_RW_RATIO  2

#define DAP_RESULT    "/tmp/dap_results"
#define DAP_RELEASE   "/tmp/.dap_finish"

// #define SHOW_LOG
#define SHOW_RESULT

#ifdef SHOW_LOG
#define dap_log printf
#else
#define dap_log(...) do {} while (0)
#endif

typedef struct {
    int desc;
    char *buf;
} dap_pages_t;

#endif
