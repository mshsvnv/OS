/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "bakery.h"

bool_t
xdr_BAKERY (XDR *xdrs, BAKERY *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->number))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->id))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->letter))
		 return FALSE;
	return TRUE;
}