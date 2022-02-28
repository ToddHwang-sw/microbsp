#include "oskit.h"

#define FALSE       (0)
#define TRUE        (1)
#define MAX_TEST    (10)

typedef struct _thr_var_t {
	unsigned int id;
	char name[32];
	int quit;
	int done;
} thr_var_t;

thr_var_t g_thr_var[10];

void thr_loop(void* arg)
{
	thr_var_t* v = (thr_var_t *) arg;

	//while (v->quit == FALSE) {
	sleep(1);
	//}

	v->done = TRUE;

	fprintf(stderr, "\t\texit!!\n");
}


int create_thr(thr_var_t* p_v, int detach)
{
	pthread_attr_t thr_attr;

	strcpy(p_v->name, "thr");
	p_v->done = FALSE;
	p_v->quit = FALSE;
	if (detach) {
		if (pthread_attr_init(&thr_attr) != 0) {
			perror("Attribute creation failed!!");
			return -1;
		}

		if (pthread_attr_setdetachstate(&thr_attr, PTHREAD_CREATE_DETACHED) != 0) {
			perror("Setting detached attribute failed!!");
			return -1;
		}

		if ((p_v->id=os_thread_create("test...", thr_loop, p_v, NULL, 0)) == ERROR) {
			perror("Thread creation failed!!");
			return -1;
		}

		pthread_attr_destroy(&thr_attr);
	} else {
		if ((p_v->id=os_thread_create("test...", thr_loop, p_v, NULL, 0)) == ERROR) {
			perror("Thread creation failed!!");
			return -1;
		}
	}

	fprintf(stderr, "\nnew thread!!\n");

	return 0;
}

int join_thr(thr_var_t* p_v)
{
	void* result;
	//if (pthread_join(p_v->id, &result) != 0) {
	if (pthread_join(p_v->id, NULL) != 0) {
		perror("Thread join failed");
		return -1;
	}
	fprintf(stderr, "\tjoin!!\n");

	return 0;
}

int main(int argc, char* argv[])
{
	int i=0;

	if (argc < 2)
		return 0;

	os_init();

	for (i=0; i<MAX_TEST; i++) {

		memset(&g_thr_var[0], 0, sizeof(thr_var_t));
		system("free");
		if (argv[1][0] == 'j') {
			create_thr(&g_thr_var[0], 0);

			join_thr(&g_thr_var[0]);
		} else if (argv[1][0] == 'd') {
			create_thr(&g_thr_var[0], 1);

			while (g_thr_var[0].done == FALSE) {
				usleep(500*1000);
			}
		} else if (argv[1][0] == 'n') {
			create_thr(&g_thr_var[0], 0);

			while (g_thr_var[0].done == FALSE) {
				usleep(500*1000);
			}
		}
	}

	return 0;
}

