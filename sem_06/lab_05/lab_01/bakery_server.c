/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

 #define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

#include "bakery.h"

u_int64_t starts[32] = {0};
int choosing[32] = {0};
int numbers[32] = {0};
int cur_id = 0;
char cur_letter = 'a';

static uint64_t now_nano()
{
    struct timespec t;
    if (clock_gettime(CLOCK_REALTIME, &t))
        return -1;
    return t.tv_sec * 1000000000LL + t.tv_nsec;
}

struct BAKERY *
get_number_1_svc(struct BAKERY *argp, struct svc_req *rqstp) {
    static struct BAKERY result;
    int max = 0, i = cur_id;

    cur_id++;
    choosing[i] = true;
    for (int j = 0; j < 32; j++)
        if (numbers[j] > max)
            max = numbers[j];
    numbers[i] = max + 1;
    
    result.id = i;
    result.number = numbers[i];
    
    choosing[i] = false;
    return &result;
}

struct BAKERY *
wait_service_1_svc(struct BAKERY *argp, struct svc_req *rqstp) {
    static struct BAKERY result;
    starts[argp->id] = now_nano();
    return &result;
}

struct BAKERY *
get_service_1_svc(struct BAKERY *argp, struct svc_req *rqstp) {
    static struct BAKERY result;
    uint64_t start, end;
    int i = argp->id;

    result.id = i;
    result.number = argp->number;
    uint64_t timeout = 0;

    uint64_t start_time = now_nano();
    for (int j = 0; j < 32; j++) 
    {
        while (choosing[j]);
        while ((numbers[j] > 0) && (numbers[j] < numbers[i] || (numbers[j] == numbers[i] && j < i)))
        {
        	end = now_nano();
        	if (end - start > 1e8)
            {
                j++;
                start = now_nano();
			}
        }
    }
    printf("Client %d: %llu ns\n", argp->id, (now_nano() - starts[argp->id]));
    printf("Client %d: %llu ns\n", argp->id, (now_nano() - start_time));

    result.letter = cur_letter;
    cur_letter++;

    numbers[i] = 0;


    return &result;
}
