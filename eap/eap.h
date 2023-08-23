#ifndef _EAP_H
#define _EAP_H

#include <sys/time.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "dipc.h"

#define PAGE_SIZE        4096
#define BLOCK_MAX        1024
#define BLOCK_SIZE_MAX   (1 << 20)

#define KEY_MAIN         7100
#define KEY_LOCK         7200
#define BARRIER_START    7300
#define BARRIER_TEST     7400
#define BARRIER_END      7500

#define GUARD_TIME       10   // msec

#define EAP_BLOCKS       256
#define EAP_BLOCK_SIZE   4096
#define EAP_RW_RATIO     2
#define EAP_EP           0.5 // the possibility of accessing a neighboring block
#define EAP_ROUNDS       10000

#define EAP_RESULTS      "/tmp/eap_results"
#define EAP_RELEASE      "/tmp/.eap_finish"

// #define SHOW_LOG
// #define SHOW_LOCK
// #define SHOW_BINARY
#define SHOW_RESULTS

#ifdef SHOW_LOG
#define eap_log printf
#else
#define eap_log(...) do {} while (0)
#endif

#ifdef SHOW_BINARY
static inline void show_binary(const char *name, void *ptr, size_t size)
{
    int i;
    int j;
    int cnt = 0;
    char *p = ptr;
    const int width = 32;
    char str[width + 1];

    printf("%s:\n", name);
    for (i = 0; i < size; i++) {
        char ch = p[i];

        for (j = 0; j < 8; j++) {
            if (ch & 1)
                str[cnt++] = '1';
            else
                str[cnt++] = '0';
            ch >>= 1;
        }
        if (cnt == width) {
            str[cnt] = '\0';
            printf("%s\n", str);
            cnt = 0;
        }
    }
    if (cnt) {
        str[cnt] = '\0';
        printf("%s\n", str);
    }
}
#else
#define show_binary(...) do {} while (0)
#endif

#endif
