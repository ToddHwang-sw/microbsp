/*******************************************************************************
 *
 * time.c - OS Porting layer
 *
 ******************************************************************************/
#include "os.h"

static os_time_t init_t;

/*******************************************************************************
 *
 * os_time_init  - Initializing OS time
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 * ERRORS
 *  N/A
 */
unsigned int os_time_init(void)
{
	os_gettime(&init_t);
	return OKAY;
}

/*******************************************************************************
 *
 * os_time_get - Current OS time
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 * ERRORS
 *  N/A
 */
unsigned long os_time_get(void)
{
	os_time_t tv;

	os_gettime(&tv);
	return (unsigned long)(os_make_longtime(tv) /*-os_make_longtime(init_t)*/);
}

