#include "barrier.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dipc.h"

union semun {
    int val;                /* value for SETVAL */
    struct semid_ds *buf;   /* buffer for IPC_STAT & IPC_SET */
    unsigned short *array;  /* array for GETALL & SETALL */
    struct seminfo *__buf;  /* buffer for IPC_INFO */
};

int barrier(int key, int total)
{
    int i;
    int desc;
    int ret = -1;
    union semun sem;
    int key_start = key + 1;

    desc = semget(key, 1, IPC_DIPC | IPC_CREAT | IPC_EXCL);
    if (desc < 0) {
        desc = semget(key, 1, IPC_DIPC | IPC_CREAT);
        if (desc < 0) {
            printf("barrier: failed\n");
            exit(-1);
        }
    } else {
        sem.val = 1;
        if (semctl(desc, 0, SETVAL, sem) < 0) {
            printf("barrier: failed\n");
            exit(-1);
        }
    }
    for (i = 0; i < total; i++) {
        if (semget(key_start, 1, IPC_CREAT | IPC_DIPC | IPC_EXCL) >= 0) {
            if (i == total - 1) {
                struct sembuf down;

                memset(&down, 0, sizeof(struct sembuf));
                down.sem_op = -1;
                if (semop(desc, &down, 1) < 0) {
                    printf("barrier: failed\n");
                    exit(-1);
                }
            } else {
                struct sembuf wait;

                memset(&wait, 0, sizeof(struct sembuf));
                if (semop(desc, &wait, 1) < 0) {
                    printf("barrier: failed\n");
                    exit(-1);
                }
            }
            ret = i + 1;
            break;
        }
        key_start++;
    }
    return ret;
}
