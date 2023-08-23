#include "eap.h"
#include "vlock.h"
#include "barrier.h"

void msleep(int msec)
{
    usleep(msec * 1000);
}


void release_wait()
{
    FILE *fp = fopen(EAP_RELEASE, "w");
    
    fclose(fp);
    while (!access(EAP_RELEASE, F_OK))
        sleep(1);
}


void eap(int id, char *shmbuf, char *lock_vec, int nr_blocks, size_t block_size, float p, int ratio)
{
    int i;
    int j;
    int pos = rand() % nr_blocks;
    
    for (i = 0; i < EAP_ROUNDS; i++) {
        int len = rand() % block_size;
        int start = rand() % block_size;
        char *ptr = &shmbuf[pos * block_size];

        eap_log("round=%d (blk=%d/%d)\n", i, pos, nr_blocks);
        vlock_lock(id, lock_vec, pos);
        if (rand() % (ratio + 1)) { // read
            for (j = 0; j < len; j++) {
                if (ptr[start++]);
                if (start == block_size)
                    start = 0;
            }
        } else { // write
            for (j = 0; j < len; j++) {
                ptr[start++] = rand() & 0xff;
                if (start == block_size)
                    start = 0;
            }
        }
        vlock_unlock(id, lock_vec, pos);
        if (((double)rand() / RAND_MAX) <= p)
            pos = (pos + 1) % nr_blocks;
        else
            pos = rand() % nr_blocks;
    }
}


void save(float total_time)
{
    FILE *filp;

#ifdef SHOW_RESULTS
    printf("total_time=%f\n", total_time);
#endif
    filp = fopen(EAP_RESULTS, "w");
    fprintf(filp, "total_time=%f", total_time);
    fclose(filp);
}


int start_leader(int id, int nr_nodes, int nr_blocks, size_t block_size, float p, int ratio)
{
    int desc;
    int ret = -1;
    char *shmbuf;
    int desc_lock;
    void *lock_vec;
    float total_time;
    struct timeval time_start, time_end;
    size_t shmsz = nr_blocks * block_size;

    eap_log("leader: step 0, preparing ...\n");
    desc = shmget(KEY_MAIN, shmsz, IPC_DIPC | IPC_CREAT);
    if (desc < 0)
        goto out;
    shmbuf = shmat(desc, NULL, 0);
    if (shmbuf == (char *)-1)
        goto release;
    memset(shmbuf, 0, shmsz);
    msleep(GUARD_TIME);
    desc_lock = vlock_create(KEY_LOCK, nr_nodes, nr_blocks);
    if (desc_lock < 0)
        goto release_lock;
    lock_vec = vlock_init(desc_lock, nr_nodes, nr_blocks);
    if (lock_vec == (void *)-1)
        goto release_lock;
    msleep(GUARD_TIME);
    barrier(BARRIER_TEST, nr_nodes);
    eap_log("leader: step 1, testing ...\n");
    gettimeofday(&time_start, NULL);
    eap(id, shmbuf, lock_vec, nr_blocks, block_size, p, ratio);
    eap_log("leader: step 2, waiting ...\n");
    barrier(BARRIER_END, nr_nodes);
    gettimeofday(&time_end, NULL);
    eap_log("leader: step 3, save result\n");
    total_time = time_end.tv_sec - time_start.tv_sec + (time_end.tv_usec - time_start.tv_usec) / 1000000.0;
    save(total_time);
    eap_log("leader: step 4, release\n");
    release_wait();
    shmdt(shmbuf);
    ret = 0;
release_lock:
    vlock_release(desc_lock, lock_vec);
release:
    shmctl(desc, IPC_RMID, NULL);
out:
    if (ret < 0)
        printf("leader: failed to start\n");
    return ret;
}


int start_member(int id, int nr_nodes, int nr_blocks, size_t block_size, float p, int ratio)
{
    int desc;
    char *shmbuf;
    void *lock_vec;
    float total_time;
    struct timeval time_start, time_end;
    size_t shmsz = nr_blocks * block_size;

    eap_log("member: step 0, preparing ...\n");
    while ((desc = shmget(KEY_MAIN, shmsz, IPC_DIPC)) < 0)
        sleep(1);
    shmbuf = shmat(desc, NULL, 0);
    if (shmbuf == (char *)-1)
        goto err;
    msleep(GUARD_TIME);
    while ((desc = vlock_find(KEY_LOCK, nr_blocks)) < 0)
        sleep(1);
    lock_vec = vlock_check(desc);
    if (lock_vec == (void *)-1)
        goto err;
    msleep(GUARD_TIME);
    barrier(BARRIER_TEST, nr_nodes);
    eap_log("member: step 1, testing ...\n");
    gettimeofday(&time_start, NULL);
    eap(id, shmbuf, lock_vec, nr_blocks, block_size, p, ratio);
    eap_log("member: step 2, waiting ...\n");
    barrier(BARRIER_END, nr_nodes);
    gettimeofday(&time_end, NULL);
    eap_log("member: step 3, save result\n");
    total_time = ((time_end.tv_sec * 1000000 + time_end.tv_usec) - (time_start.tv_sec * 1000000 + time_start.tv_usec)) / 1000000.0;
    save(total_time);
    eap_log("member: step 4, release\n");
    release_wait();
    return 0;
err:
    printf("member: failed to start\n");
    return -1;
}


int start(int nr_nodes, int nr_blocks, size_t block_size, float p, int ratio)
{
    int id = barrier(BARRIER_START, nr_nodes);
    
    if ((id <= 0) || (id > nr_nodes)) {
        printf("Error: invalid id\n");
        return -1;
    }
    srand((unsigned)time(0) + (unsigned)id);
    if (1 == id) {
        if (start_leader(id, nr_nodes, nr_blocks, block_size, p, ratio))
            return -1;
    } else {
        if (start_member(id, nr_nodes, nr_blocks, block_size, p, ratio))
            return -1;
    }
    return 0;
}


void usage()
{
    printf("Usage: eap [-n nodes] [-b blocks] [-s size] [-p possibility] [-r ratio]\n");
    printf("-n: the number of nodes\n");
    printf("-b: the number of blocks\n");
    printf("-s: block size\n");
    printf("-p: the possibility of accessing a neighboring block\n");
    printf("-r: read to write ratio\n");
}


int main(int argc, char **argv)
{
    char ch;
    int nr_nodes = 0;
    float p = EAP_EP;
    int ratio = EAP_RW_RATIO;
    int nr_blocks = EAP_BLOCKS;
    size_t block_size = EAP_BLOCK_SIZE;
    
    while ((ch = getopt(argc, argv, "n:b:s:p:r:h")) != -1) {
        switch(ch) {
        case 'n':
            nr_nodes = atoi(optarg);
            if (0 == nr_nodes) {
                printf("Error: invalid number of nodes\n");
                exit(-1);
            }
            break;
        case 'b':
            nr_blocks = atoi(optarg);
            if (0 == nr_nodes || nr_blocks > BLOCK_MAX) {
                printf("Error: invalid number of blocks\n");
                exit(-1);
            }
            break;
        case 's':
            block_size = atoi(optarg);
            if (block_size == 0 || block_size > BLOCK_SIZE_MAX) {
                printf("Error: invalid block size\n");
                exit(-1);
            }
            break;
        case 'p':
            p = atof(optarg);
            if (p >= 1.0 || p < 0) {
                printf("Error: invalid possibility\n");
                exit(-1);
            }
            break;
        case 'r':
            ratio = atoi(optarg);
            if (0 == ratio) {
                printf("Error: invalid ratio\n");
                exit(-1);
            }
            break;
        default:
            usage();
            exit(-1);
        }
    }
    return start(nr_nodes, nr_blocks, block_size, p, ratio);
}
