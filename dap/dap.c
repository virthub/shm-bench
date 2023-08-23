#include "barrier.h"
#include "dap.h"

void release_wait()
{
    FILE *fp = fopen(DAP_RELEASE, "w");
    
    fclose(fp);
    while (!access(DAP_RELEASE, F_OK))
        sleep(1);
}


void free_pages(dap_pages_t *pages)
{
    shmdt(pages->buf);
    shmctl(pages->desc, IPC_RMID, NULL);
    free(pages);
}


dap_pages_t *alloc_pages(int nr_pages)
{
    dap_pages_t *pages = (dap_pages_t *)malloc(sizeof(dap_pages_t));

    if (!pages) {
        printf("Error: failed to allocate pages");
        return NULL;
    }
    memset(pages, 0, sizeof(dap_pages_t));
    if ((pages->desc = shmget(KEY_MAIN, nr_pages * DAP_PAGE_SIZE, IPC_DIPC | IPC_CREAT)) < 0) {
        free(pages);
        return NULL;
    }
    pages->buf = shmat(pages->desc, NULL, 0);
    if (pages->buf == (char *)-1) {
        shmctl(pages->desc, IPC_RMID, NULL);
        free(pages);
        return NULL;
    }
    return pages;
}


void dap(dap_pages_t *pages, int nr_pages, int ratio)
{
    int i;
    int j;

    for (i = 0; i < DAP_ROUNDS; i++) {
        int pgno = rand() % nr_pages;
        int len = rand() % DAP_PAGE_SIZE;
        int start = rand() % DAP_PAGE_SIZE;
        char *ptr = &pages->buf[pgno * DAP_PAGE_SIZE];

        if (rand() % (ratio + 1)) { // read
            for (j = 0; j < len; j++) {
                if (ptr[start++]);
                if (start == DAP_PAGE_SIZE)
                    start = 0;
            }
        } else { // write
            for (j = 0; j < len; j++) {
                ptr[start++] = rand() & 0xff;
                if (start == DAP_PAGE_SIZE)
                    start = 0;
            }
        }
    }
}


void save(float total_time)
{
    FILE *filp;

#ifdef SHOW_RESULT
    printf("total_time=%f\n", total_time);
#endif
    filp = fopen(DAP_RESULT, "w");
    fprintf(filp, "total_time=%f", total_time);
    fclose(filp);
}


int start_test(int nr_nodes, int nr_pages, int ratio)
{
    int id;
    float total_time;
    dap_pages_t *pages;
    struct timeval time_start, time_end;

    dap_log("step 0, preparing ...\n");
    pages = alloc_pages(nr_pages);
    if (!pages) {
        printf("Error: failed to alloc pages\n");
        return -1;
    }
    id = barrier(BARRIER_START, nr_nodes);
    srand((unsigned)time(0) + (unsigned)id);
    dap_log("step 1, testing ...\n");
    gettimeofday(&time_start, NULL);
    dap(pages, nr_pages, ratio);
    dap_log("step 2, waiting ...\n");
    barrier(BARRIER_END, nr_nodes);
    gettimeofday(&time_end, NULL);
    dap_log("step 3, save result\n");
    total_time = ((time_end.tv_sec * 1000000 + time_end.tv_usec) - (time_start.tv_sec * 1000000 + time_start.tv_usec)) / 1000000.0;
    save(total_time);
    dap_log("step 4, release\n");
    release_wait();
    free_pages(pages);
    return 0;
}


void usage()
{
    printf("Usage: dap [-n nodes] [-p pages] [-r ratio]\n");
    printf("-n: the number of nodes\n");
    printf("-p: the number of pages\n");
    printf("-r: read to write ratio\n");
}


int main(int argc, char **argv)
{
    char ch;
    int nr_nodes = 0;
    int nr_pages = DAP_PAGES;
    int ratio = DAP_RW_RATIO;

    while ((ch = getopt(argc, argv, "n:p:r:h")) != -1) {
        switch(ch) {
        case 'n':
            nr_nodes = atoi(optarg);
            if (0 == nr_nodes) {
                printf("Error: invalid number of nodes\n");
                exit(-1);
            }
            break;
        case 'p':
            nr_pages = atoi(optarg);
            if (0 == nr_pages) {
                printf("Error: invalid number of pages\n");
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
            exit(1);
        }
    }
    return start_test(nr_nodes, nr_pages, ratio);
}
