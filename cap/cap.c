#include "cap.h"

void msleep(int msec)
{
    usleep(msec * 1000);
}


void release_wait()
{
    FILE *fp = fopen(CAP_RELEASE, "w");
    
    fclose(fp);
    while (!access(CAP_RELEASE, F_OK))
        sleep(1);
}


void cap(char *shmbuf, int shmsz, int ratio)
{
    int i;
    int j;
    
    for (i = 0; i < CAP_ROUNDS; i++) {
        int start = 0;
        int size = shmsz;
        
        cap_log("round=%d\n", i);
        while (size > PAGE_SIZE) {
            int pos;
            int len;

            size /= 2;
            if (rand() % 2)
                start += size;
            pos = rand() % size;
            len = rand() % size;
            if (rand() % (ratio + 1)) { // read
                for (j = 0; j < len; j++) {
                    if (shmbuf[start + pos]);
                    pos++;
                    if (pos == size)
                        pos = 0;
                }
            } else { // write
                for (j = 0; j < len; j++) {
                    shmbuf[start + pos] = rand() & 0xff;
                    pos++;
                    if (pos == size)
                        pos = 0;
                }
            }
        }
    }
}


void save(float total_time)
{
    FILE *filp;

#ifdef SHOW_RESULTS
    printf("total_time=%f\n", total_time);
#endif
    filp = fopen(CAP_RESULTS, "w");
    fprintf(filp, "total_time=%f", total_time);
    fclose(filp);
}


int start_leader(int nr_nodes, int shm_size, int ratio)
{
    int desc;
    int ret = -1;
    char *shmbuf;
    float total_time;
    struct timeval time_start, time_end;
    
    cap_log("leader: step 0, preparing ...\n");
    desc = shmget(KEY_MAIN, shm_size, IPC_DIPC | IPC_CREAT);
    if (desc < 0)
        goto out;
    shmbuf = shmat(desc, NULL, 0);
    if (shmbuf == (char *)-1)
        goto release;
    memset(shmbuf, 0, shm_size);
    msleep(GUARD_TIME);
    barrier(BARRIER_TEST, nr_nodes);
    cap_log("leader: step 1, testing ...\n");
    gettimeofday(&time_start, NULL);
    cap(shmbuf, shm_size, ratio);
    cap_log("leader: step 2, waiting ...\n");
    barrier(BARRIER_END, nr_nodes);
    gettimeofday(&time_end, NULL);
    cap_log("leader: step 3, save result\n");
    total_time = time_end.tv_sec - time_start.tv_sec + (time_end.tv_usec - time_start.tv_usec) / 1000000.0;
    save(total_time);
    cap_log("leader: step 4, release\n");
    release_wait();
    shmdt(shmbuf);
    ret = 0;
release:
    shmctl(desc, IPC_RMID, NULL);
out:
    if (ret < 0)
        printf("leader: failed to start\n");
    return ret;
}


int start_member(int nr_nodes, int shm_size, int ratio)
{
    int desc;
    char *shmbuf;
    float total_time;
    struct timeval time_start, time_end;

    cap_log("member: step 0, preparing ...\n");
    while ((desc = shmget(KEY_MAIN, shm_size, IPC_DIPC)) < 0)
        sleep(1);
    shmbuf = shmat(desc, NULL, 0);
    if (shmbuf == (char *)-1)
        goto err;
    msleep(GUARD_TIME);
    barrier(BARRIER_TEST, nr_nodes);
    cap_log("member: step 1, testing ...\n");
    gettimeofday(&time_start, NULL);
    cap(shmbuf, shm_size, ratio);
    cap_log("member: step 2, waiting ...\n");
    barrier(BARRIER_END, nr_nodes);
    gettimeofday(&time_end, NULL);
    total_time = ((time_end.tv_sec * 1000000 + time_end.tv_usec) - (time_start.tv_sec * 1000000 + time_start.tv_usec)) / 1000000.0;
    cap_log("member: step 3, save result\n");
    save(total_time);
    cap_log("member: step 4, release\n");
    release_wait();
    return 0;
err:
    printf("member: failed to start\n");
    return -1;
}


int start(int nr_nodes, int shm_size, int ratio)
{
    int id = barrier(BARRIER_START, nr_nodes);
    
    if ((id <= 0) || (id > nr_nodes)) {
        printf("Error: invalid id\n");
        return -1;
    }
    srand((unsigned)time(0) + (unsigned)id);
    if (1 == id) {
        if (start_leader(nr_nodes, shm_size, ratio))
            return -1;
    } else {
        if (start_member(nr_nodes, shm_size, ratio))
            return -1;
    }
    return 0;
}


void usage()
{
    printf("Usage: cap [-n nodes] [-s size] [-r ratio]\n");
    printf("-n: the number of nodes\n");
    printf("-s: shared memory size\n");
    printf("-r: read to write ratio\n");
}


int main(int argc, char **argv)
{
    char ch;
    int nr_nodes = 0;
    int shm_size = CAP_SIZE;
    int ratio = CAP_RW_RATIO;
    
    while ((ch = getopt(argc, argv, "n:s:r:h")) != -1) {
        switch(ch) {
        case 'n':
            nr_nodes = atoi(optarg);
            if (0 == nr_nodes) {
                printf("Error: invalid number of nodes\n");
                exit(-1);
            }
            break;
        case 's':
            shm_size = atoi(optarg);
            if (0 == shm_size) {
                printf("Error: invalid size\n");
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
    return start(nr_nodes, shm_size, ratio);
}
