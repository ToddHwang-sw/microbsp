/*******************************************************************************
 *
 * error.h - ERROR code
 *
 ******************************************************************************/

#ifndef _OS_ERROR_CODE
#define _OS_ERROR_CODE

#ifdef __cplusplus
extern "C" {
#endif

/* base */
#define E_NOERROR       (0xFFFF0000)

/* generic */
#define E_INVALID_PARAM     (E_NOERROR+1)
#define E_NOMEM         (E_NOERROR+2)
#define E_INVALID_POOL      (E_NOERROR+3)
#define E_NOSPC         (E_NOERROR+4)
#define E_NODATA        (E_NOERROR+5)

/* network error */
#define E_NET_PORT_UNESTAB  (E_NOERROR+100+1)
#define E_NET_PORT_RX       (E_NOERROR+100+2)
#define E_NET_PORT_TX       (E_NOERROR+100+3)
#define E_NET_CANNOTCONN    (E_NOERROR+100+4)
#define E_NET_CONNTIMEOUT   (E_NOERROR+100+5)


#ifdef __cplusplus
}
#endif

#endif /* UTVSW_OS_ERROR_CODE */
