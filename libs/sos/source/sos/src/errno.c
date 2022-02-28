/*******************************************************************************
 *
 * errno.c - Error number
 *
 ******************************************************************************/
#include "oskit.h"

/*
 * ERRNO
 */
static unsigned int _os_errno;

/*******************************************************************************
 *
 * os_errno_set - Setting up errno
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
 *   N/A
 *
 */
void os_errno_set(unsigned int num)
{
	_os_errno = num;
}

/*******************************************************************************
 *
 * os_errno_get - Retuning errno
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
 *   N/A
 *
 */
unsigned int os_errno_get(void)
{
	return _os_errno;
}

