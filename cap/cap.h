#ifndef _CAP_H
#define _CAP_H

#include <sys/time.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "dipc.h"
#include "barrier.h"

#define KEY_MAIN        6100
#define BARRIER_START   6200
#define BARRIER_TEST    6300
#define BARRIER_END     6400

#define GUARD_TIME      10 // msec

#define CAP_RW_RATIO    2
#define CAP_SIZE        262144
#define CAP_ROUNDS      10000

#define CAP_RESULTS     "/tmp/cap_results"
#define CAP_RELEASE     "/tmp/.cap_finish"

// #define SHOW_LOG
#define SHOW_RESULTS

#ifdef SHOW_LOG
#define cap_log printf
#else
#define cap_log(...) do {} while (0)
#endif

#endif
