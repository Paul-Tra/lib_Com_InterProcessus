#ifndef H_MFILE
#define H_MFILE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <sys/stat.h> /* Pour les constantes « mode » */
#include <fcntl.h> /* Pour les constantes O_* */ 
#include <semaphore.h> 

/* mfifo type : */

typedef struct{
	const char * nom ;
	size_t capacity;
	size_t debut;
	size_t fin;
	pid_t pid;
	sem_t sem;
	/* semaphores , mutexes, conditions */
	char memory[];
} mfifo;



/* Associated fonctions  */

void Init(void);
mfifo * mfifo_connect( const char *nom, int options, mode_t permission, size_t capacite );
int write_addr(char *val , mfifo * fifo );
int mfifo_disconnect(mfifo *fifo);
int mfifo_unlink(const char*nom);

#endif
					

