/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <memory.h> /* for memset */
#include "bakery.h"

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

struct BAKERY *
get_number_1(struct BAKERY *argp, CLIENT *clnt)
{
	static struct BAKERY clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, GET_NUMBER,
		(xdrproc_t) xdr_BAKERY, (caddr_t) argp,
		(xdrproc_t) xdr_BAKERY, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

struct BAKERY *
wait_service_1(struct BAKERY *argp, CLIENT *clnt)
{
	static struct BAKERY clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, WAIT_SERVICE,
		(xdrproc_t) xdr_BAKERY, (caddr_t) argp,
		(xdrproc_t) xdr_BAKERY, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

struct BAKERY *
get_service_1(struct BAKERY *argp, CLIENT *clnt)
{
	static struct BAKERY clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, GET_SERVICE,
		(xdrproc_t) xdr_BAKERY, (caddr_t) argp,
		(xdrproc_t) xdr_BAKERY, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}





/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "bakery.h"

void
bakery_program_1(char *host)
{
	CLIENT *clnt;
	struct BAKERY  *result_1;
	struct BAKERY  get_number_1_arg;
	struct BAKERY  *result_2;
	struct BAKERY  wait_service_1_arg;
	struct BAKERY  *result_3;
	struct BAKERY  get_service_1_arg;

#ifndef	DEBUG
	clnt = clnt_create (host, BAKERY_PROG, BAKERY_VER, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif	/* DEBUG */

    time_t start_numb, end_numb, start_wait, end_wait, start_serv, end_serv;
	pid_t pid;

	pid = getpid();
	printf("Cient %d started\n", pid);

	srand(time(NULL) ^ pid);
	sleep(rand() % 3 + 2);

	start_numb = clock();
	get_number_1_arg.letter = (int)pid;

	result_1 = get_number_1(&get_number_1_arg, clnt);
	if (result_1 == (struct BAKERY *) NULL) {
		clnt_perror (clnt, "call failed");
	}

	end_numb = clock();

	printf("Client %d get number %d\n", pid, result_1->number);

	sleep(rand() % 5 + 2);

	start_wait =clock();

	wait_service_1_arg.number = result_1->number;
	wait_service_1_arg.id = result_1->id;
	wait_service_1_arg.letter = (int)pid;

	result_2 = wait_service_1(&wait_service_1_arg, clnt);
	if (result_2 == (struct BAKERY *) NULL) {
		clnt_perror (clnt, "call failed");
	}

	end_wait = clock();

	sleep(rand() % 5 + 1);

	start_serv = clock();

	get_service_1_arg.number = result_1->number;
	get_service_1_arg.id = result_1->id;

	result_3 = get_service_1(&get_service_1_arg, clnt);
	if (result_3 == (struct BAKERY *) NULL) {
		clnt_perror (clnt, "call failed");
	}

	end_serv = clock();

	printf("Client %d get value '%c'\n", pid, result_3->letter);
	printf("Time: %lf\n", difftime(end_numb, start_numb)+difftime(end_wait, start_wait));

#ifndef	DEBUG
	clnt_destroy (clnt);
#endif	 /* DEBUG */
}


int
main (int argc, char *argv[])
{
	char *host;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}
	host = argv[1];
	bakery_program_1 (host);
exit (0);
}
