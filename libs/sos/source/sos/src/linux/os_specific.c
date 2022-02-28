/*******************************************************************************
 *
 * os_specific.c - Linux Initialization
 *
 ******************************************************************************/
#include "oskit.h"

/*******************************************************************************
 *
 * os_specific_init - Initializing Linux specific initialization
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
unsigned int os_specific_init(void)
{
	return OKAY;
}

/*******************************************************************************
 *
 * os_post_init - final initialization after all the resources inited.
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
unsigned int os_post_init(void)
{
	return OKAY;
}

/*******************************************************************************
 *
 * os_errno_printf -
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
int os_errno_printf(void)
{
	int err = errno;
	os_debug_printf(OS_DBG_LVL_ERR, "(%d) %s \n", err, strerror(err));
	return err;
}

/*******************************************************************************
 *
 * os_errno - os specific error code for linux
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
int os_errno(void)
{
	return errno;
}

/*******************************************************************************
 *
 * os_printf -
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 *
 * ERRORS
 *  N/A
 */
void os_printf(unsigned char *msg)
{
	printf("%s",msg);
}

/*******************************************************************************
 *
 * os_net_block_mode_set - block mode setting
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
unsigned int os_net_block_mode_set(int fd,int mode)
{
	int fl = fcntl(fd, F_GETFL);

	if (fl < 0) {
		os_debug_printf(OS_DBG_LVL_ERR,"net_blockmode_set (%d)::can not find the socket\n",fd);
		return ERROR;
	}

#define BLOCK_FLAG(s,m)  ((m==1) ? (s | O_NONBLOCK) : (s & ~O_NONBLOCK))

	/* fcntl */
	if (fcntl(fd, F_SETFL, BLOCK_FLAG(fl,mode)) < 0) {
		os_debug_printf(OS_DBG_LVL_ERR,"net_blockmode_set (%d,%d)::error\n", fd,mode);
		return ERROR;
	}

	return OKAY;
}

