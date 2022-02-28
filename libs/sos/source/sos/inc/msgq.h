/*******************************************************************************
 *
 * msgq.h - Message queue header
 *
 ******************************************************************************/
#ifndef MESSAGE_QUEUE_HEADER
#define MESSAGE_QUEUE_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/* constants */
#define OS_MESG_COMMAND(s)  ((s)->command)
#define OS_MESG_DATA(s) ((s)->data)
#define OS_MESG_SIZE(s) ((s)->size)

/* message structure */
typedef struct {
	unsigned int command; /* command */
	unsigned char *data; /* message block */
	unsigned int size;  /* size */
}__attribute__((packed)) OS_MESG;

/* message q header */
typedef struct {
	unsigned int id;        /* name */
	unsigned int qbuff;     /* queue */
	unsigned int cond;      /* conditional variable */
	unsigned int counter;   /* total messages */
}__attribute__((packed)) OS_MESG_HEAD;

/* prototype */
extern unsigned int os_msgq_init(void);
extern unsigned int os_msgq_create(const char *name,unsigned int pool,os_delfunc_t f);
extern unsigned int os_msgq_delete(unsigned int id);
extern unsigned int os_msgq_flush(unsigned int id);
extern unsigned int os_msgq_message(OS_MESG *m,unsigned int command,unsigned char *buff,unsigned int len);
extern unsigned int os_msgq_send(unsigned int id,OS_MESG *msg);
extern OS_MESG *os_msgq_recv(unsigned int id,unsigned int bmode);
extern OS_MESG *os_msgq_recv_timeout(unsigned int id,unsigned long tm);

#ifdef __cplusplus
}
#endif

#endif /* MESSAGE_QUEUE_HEADER */
