#include "vlock.h"
#include "eap.h"

int vlock_create(int key, int nr_nodes, int nr_locks)
{
    assert(vlock_size(nr_nodes) < PAGE_SIZE);
    return shmget(key, vlock_vec_size(nr_locks), IPC_DIPC | IPC_CREAT);
}


int vlock_find(int key, int nr_locks)
{
    return shmget(key, vlock_vec_size(nr_locks), IPC_DIPC);
}


void *vlock_check(int desc)
{
    return shmat(desc, NULL, 0);
}


void *vlock_init(int desc, int nr_nodes, int nr_locks)
{
    int i;
    void *vec = shmat(desc, NULL, 0);

    memset(vec, 0, vlock_vec_size(nr_locks));
    for (i = 0; i < nr_locks; i++) {
        vlock_t *lock = vlock_get(vec, i);

        lock->total = nr_nodes;
    }
    return vec;
}


void vlock_lock(int id, void *vec, int idx)
{
    int pos = id - 1;
    vlock_t *lock = vlock_get(vec, idx);
    int total = lock->total;

    assert(id > 0);
#ifdef SHOW_LOCK
    vlock_t prev;
    memset(&prev, 0, sizeof(vlock_t));
#endif
start:
    lock->B[pos] = 1;
    lock->X = id;
    if (lock->Y != 0) {
        lock->B[pos] = 0;
        while (lock->Y) {
#ifdef SHOW_LOCK
            if (memcmp(lock, &prev, sizeof(vlock_t))) {
                prev = *lock;
                show_binary("lock->Y (first time)", lock, sizeof(vlock_t));
            }
#endif
        }
        goto start;
    }
    lock->Y = id;
    if (lock->X != id) {
        int i;
        
        lock->B[pos] = 0;
        for (i = 0; i < total; i++) {
            while (lock->B[i]) {
#ifdef SHOW_LOCK
                if (memcmp(lock, &prev, sizeof(vlock_t))) {
                    char name[64] = {0};

                    prev = *lock;
                    sprintf(name, "lock->B[%d]", i);
                    show_binary(name, lock, sizeof(vlock_t));
                }
#endif
            }
        }

        if (lock->Y != id) {
            while (lock->Y) {
#ifdef SHOW_LOCK
                if (memcmp(lock, &prev, sizeof(vlock_t))) {
                    prev = *lock;
                    show_binary("lock->Y (second time)", lock, sizeof(vlock_t));
                }
#endif
            }
            goto start;
        }
    }
#ifdef SHOW_LOCK
    show_binary("lock", lock, sizeof(vlock_t));
#endif
}


void vlock_unlock(int id, void *vec, int idx)
{
    vlock_t *lock = vlock_get(vec, idx);

    assert(id > 0);
    lock->Y = 0;
    lock->B[id - 1] = 0;
#ifdef SHOW_LOCK
    show_binary("unlock", lock, sizeof(vlock_t));
#endif
}


void vlock_release(int desc, void *vec)
{
    if (vec && (vec != (vlock_t *)-1))
        shmdt(vec);
    if (desc >= 0)
        shmctl(desc, IPC_RMID, NULL);
}
