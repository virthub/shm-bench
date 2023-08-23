#ifndef _VLOCK_H
#define _VLOCK_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dipc.h"

typedef struct shmlock {
    int total;
    int X;
    int Y;
    int B[0];
} vlock_t;

// Suppose that each lock is stored in a seperated page
#define vlock_vec_size(locks) (locks * PAGE_SIZE)
#define vlock_get(vec, idx) ((vlock_t *)((vec) + (idx) * PAGE_SIZE))
#define vlock_size(nr_nodes) (nr_nodes * sizeof(int) + sizeof(vlock_t))

void *vlock_check(int desc);
int vlock_find(int key, int locks);
void vlock_release(int desc, void *vec);
void vlock_lock(int id, void *vec, int idx);
void vlock_unlock(int id, void *vec, int idx);
int vlock_create(int key, int nr_nodes, int nr_locks);
void *vlock_init(int desc, int nr_nodes, int nr_locks);

#endif
