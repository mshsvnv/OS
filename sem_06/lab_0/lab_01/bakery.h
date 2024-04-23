/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _BAKERY_H_RPCGEN
#define _BAKERY_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


struct BAKERY {
	int number;
	int id;
	int letter;
};
typedef struct BAKERY BAKERY;

#define BAKERY_PROG 0x20000001
#define BAKERY_VER 1

#if defined(__STDC__) || defined(__cplusplus)
#define GET_NUMBER 1
extern  struct BAKERY * get_number_1(struct BAKERY *, CLIENT *);
extern  struct BAKERY * get_number_1_svc(struct BAKERY *, struct svc_req *);
#define WAIT_SERVICE 2
extern  struct BAKERY * wait_service_1(struct BAKERY *, CLIENT *);
extern  struct BAKERY * wait_service_1_svc(struct BAKERY *, struct svc_req *);
#define GET_SERVICE 3
extern  struct BAKERY * get_service_1(struct BAKERY *, CLIENT *);
extern  struct BAKERY * get_service_1_svc(struct BAKERY *, struct svc_req *);
extern int bakery_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define GET_NUMBER 1
extern  struct BAKERY * get_number_1();
extern  struct BAKERY * get_number_1_svc();
#define WAIT_SERVICE 2
extern  struct BAKERY * wait_service_1();
extern  struct BAKERY * wait_service_1_svc();
#define GET_SERVICE 3
extern  struct BAKERY * get_service_1();
extern  struct BAKERY * get_service_1_svc();
extern int bakery_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_BAKERY (XDR *, BAKERY*);

#else /* K&R C */
extern bool_t xdr_BAKERY ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_BAKERY_H_RPCGEN */