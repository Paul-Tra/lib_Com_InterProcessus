
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>
#include "mfile.h"
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/stat.h> /* Pour les constantes « mode » */
#include <fcntl.h> /* Pour les constantes O_* */ 
#define LEN 512


void Init(void)
{
	printf("Projet System 2020 , TRAORE - CADIOU :\n\n");
}

mfifo *mfifo_connect( const char *nom, int options, mode_t permission, size_t capacite ){

	if ( !capacite || capacite == 0 ){
		perror("Capacité NULL ou = 0 , Erreur");
		return NULL;
	}
	mfifo * fifo = malloc(sizeof(mfifo)) ;
	/* debut fifo correspondant au retour de malloc */
	if ( nom != NULL ){
		fifo->nom = nom ;
	}
	else {
		/* mFifo anonyme */
		void * addr = mmap(NULL, capacite, PROT_READ | PROT_WRITE,  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (addr == MAP_FAILED){
            perror("mmap");
           	return NULL;
        }
		fill_mfifo(fifo,(size_t) addr, capacite);
	    return fifo ;
	}
	if ( nom != NULL ){
		int fd ;
		char * name = malloc(sizeof(char)*(sizeof(nom)+2));
		strcat(name, "/");
		strcat(name, nom);
		switch(options){
			case 0 :
				printf("\nCONNEXION ....\n");
				fd = shm_open(name, O_RDWR, permission);
				if( fd == -1 ){
					perror("shm open ");
					return NULL;
				}
				struct stat buf_stat;
    	    	if (fstat(fd, &buf_stat) == -1) {
    	    		perror(" fstat ");
    	        	exit(1);
    	    	}
				
    	    	//*fifo->memory = mmap( NULL, sysconf(_SC_PAGESIZE) , PROT_READ | PROT_WRITE , MAP_SHARED , fd, 0);
    	    	int segment_id = shmget (fd, getpagesize(),  IPC_CREAT );

    	    	printf("segment id : %d\n", segment_id );

				*fifo->memory = shmat (segment_id, NULL, 0);
				/*
				int r = msync(*fifo->memory, buf_stat.st_size, MS_SYNC) ;

				printf("Connexion a un Fifo existant : \nres mysinc : %d \n", r);

    	    	printf("0 - On a cree la map a l'adresse du memory : %p\n",  fifo->memory);
				*/
    	    	close(fd);
				fill_mfifo(fifo,(size_t) fifo->memory, sysconf(_SC_PAGESIZE));

				printf("Content du fifo dans la connexion : \n>%s\n", &fifo->memory );

    			return fifo ;

			case O_CREAT :
				if ( permission != 0  ){
					fd = shm_open(name, O_CREAT | O_RDWR , permission );// cree avec permission
					if ( fd != -1 ){
						//void *addr = mmap(NULL, capacite , PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
						struct stat buf_stat;
						/*int res = ftruncate(fd, capacite ) ;
						if ( res == -1 ) {
							perror("0 - ftruncate");
							return NULL ;
						}*/
		    	    	if (fstat(fd, &buf_stat) == -1) {
		    	    		perror(" fstat ");
		    	        	exit(1);
		    	    	}

						*fifo->memory = mmap(NULL, sysconf(_SC_PAGESIZE) , PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd, 0);
						if ( fifo->memory == MAP_FAILED){
				            perror("O_creat , mmap");
				           	return NULL;
			        	}
			        	printf("\nCREATION ............\n");
		    			close(fd);
		    			fill_mfifo(fifo,(size_t) fifo->memory, sysconf(_SC_PAGESIZE));
		    			init_memory_mfifo(fifo);
						
						//printf("O_create : addr : %p\nLen : %ld ",fifo->memory ,sysconf(_SC_PAGESIZE));
						//printf("O_create : fifo->memory : %p\n", &fifo->memory[0]);

						return fifo ;
		    		}
		    		else{
		    			fd = shm_open(name, O_RDWR, permission );
		    			if(fd == -1){
		    				perror(" shm open ");
		    				return NULL;
		    			}
						void *addr = mmap(NULL, capacite , PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
						if (addr == MAP_FAILED){
				            perror("mmap");
				           	return NULL;
				        }
		    			close(fd);
		    			fill_mfifo(fifo,(size_t) addr, capacite);
		    		}
				}				
				break;
			case O_CREAT|O_EXCL :
				fd = shm_open(name, O_RDWR, 0);
				if ( fd != -1 ){ // mfifo existe deja donc Connect() doit echouer
					perror("mfifo_connect() echoue car l'objet existe deja .\n");
					break;
				}
				else{
					if ( permission != 0 ){
						fd = shm_open(name, O_CREAT, permission );// cree avec permission
						if ( fd == -1 ){
							perror("mfifo_connect() echoue car creation echoue .\n");
							return NULL;
						}else{
							void *addr = mmap(NULL, capacite , PROT_READ | PROT_WRITE, MAP_SHARED, fd, permission);
			    			close(fd);
			  				fill_mfifo(fifo,(size_t) addr, capacite);
			  				init_memory_mfifo(fifo);
			    			return fifo ;
						}
					}
					else {
						fd = shm_open(name, O_CREAT, 0 ); // cree sans permission
						if ( fd == -1 ){
							perror("mfifo_connect() echoue car creation echoue .\n");
							return NULL;
						}else{
							void *addr = mmap(NULL, capacite , PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			    			close(fd);
			    			fill_mfifo(fifo,(size_t) addr, capacite);
			    			init_memory_mfifo(fifo);
			    			return fifo ;
						}
					}
				}
		}
	}
	return fifo ;
}

/**
* Remplie un objet mfifo par diverses 
*
* @param fifo 		objet mfifo à remplir
* @param addr 		adresse de début de fifo
* @param capacite 	capacité totale de fifo
*/
void fill_mfifo(mfifo * fifo, size_t addr, size_t capacite){
	fifo->debut = addr ;
	fifo->fin = fifo->debut + capacite ;
	fifo->capacity = capacite;
	fifo->pid = -1 ;
	//printf("fill_fifo : memory : %p\n", fifo->memory );

	//*(fifo->memory) = &addr ;
	if(sem_init(&fifo->sem,1,1) == -1 ){
		perror("sem init 210 ");
	}
}

/**
* Initialise la memoire d'un mfifo
*
* @param fifo objet fifo dont il faut initialiser la memoire
*/
void init_memory_mfifo(mfifo * fifo){
	memset(fifo->memory, 0, strlen(fifo->memory));
}

	/*
	mfifo_write() bloque le processus appelant jusqu’à ce que len octets soient écrits dans
	fifo. Les octets écrits ne doivent pas être mélangés avec les octets écrits par d’autres
	processus, et le processus appelant reste donc bloqué tant qu’il n’y a pas de place pour
	len octets dans fifo.
	*/
int mfifo_write(mfifo *fifo, const void *val, size_t len){
	size_t cpt = mfifo_free_memory(fifo);
	// On test que LEN est bien < fifo->capacite .

	if ( len > fifo->capacity || cpt < len ){
		perror("Erreur , Len > fifo->capacite\n");
		errno = EMSGSIZE;
		return -1;
	}

	int fd = shm_open(fifo->nom, O_RDWR, 0777);
	if( fd == -1 ){
		perror("shm open ");
		return NULL;
	}
	struct stat buf_stat;
	if (fstat(fd, &buf_stat) == -1) {
		perror(" fstat ");
    	exit(1);
	}

	int count = write(fd,val,len) ;
	/*
	cpt = fifo->capacity - cpt ;
    snprintf(fifo->memory + strlen(fifo->memory), fifo->capacity - strlen(fifo->memory),"%s",(char *) val);
	//memcpy(fifo->memory + strlen(fifo->memory) , val , len) ;
	printf("Write :\nl : %ld octets a l'adresse : %p\n", sysconf(_SC_PAGESIZE), fifo->memory);
    printf("Content : %s\n",fifo->memory );
    */

	return count;
}

/*
Supposons qu’un mfifo contient n octets et l = min(len, n). La fonction mfifo_read()
copie l octets de mfifo à l’adresse buf. Les octets copiés sont supprimés du mfifo et
mfifo_read retourne le nombre d’octets copiés.
S’il y plusieurs processus lecteurs qui tentent de lire en même temps, tous sauf un seront
bloqués en attendant la fin de la lecture du seul processus autorisé à lire. Chaque lecture
lit donc un segment contigu d’octets stockés dans le mfifo.
*/
ssize_t mfifo_read(mfifo *fifo, void *buf, size_t len){
	//printf("Read à : %p\n",fifo->memory );
	//memcpy(buf,fifo->memory,len);
	//printf("Buf : %s\n", buf );
	printf("En Lecture : \n> ");
	/*
	for ( int i = 0 ; i < (int)len ; i++ ){
		memcpy( buf+i, &fifo->memory[i] , 1 );
		count++ ;
	}
	*/
	int fd = shm_open(fifo->nom, O_RDWR, 0777);
	if( fd == -1 ){
		perror("shm open ");
		return NULL;
	}
	struct stat buf_stat;
	if (fstat(fd, &buf_stat) == -1) {
		perror(" fstat ");
    	exit(1);
	}

	int count = read(fd,buf,len) ;
	printf("%s\n", buf);
	/*


	//printf("Buf : %s\n", buf );
	memset(fifo->memory,0,len);
	// inserer ICI la fonction de decalage de memoire
	printf("\nContent restant dans Fifo->memory : %s\n", &fifo->memory[len]);

	mfifo_unlock(fifo);
	*/
    return count;

}

/**
* Verrouille le mfifo pour la lecture, fonction bloquante, attend tend que 
* le verrou n'a pas été posé sur l'objet mfifo
*
* @param fifo objet mfifo sur lequel nous voulons poser un verrou
* @return -1 en cas d'echec , 0 sinon
*/
int mfifo_lock(mfifo *fifo){
	/*ajout du pid du processus appelant dans la queue de la semaphore*/
	if(sem_wait(&fifo->sem) == -1){
		perror("sem wait ");
		return -1;
	}
	return 0;
}

/**
*	Essais de vérouiller l'objet mfifo
*
* @param 	fifo objet mfifo sur lequel nous voulons essayer de poser un verrou
* @return 	revoie 0 si l'obtention du verrou réussi, -1 sinon
*/
int mfifo_trylock(mfifo *fifo){
	if(sem_trywait(&fifo->sem) == -1){
		perror("sem trywait");
		return -1;
	}
	return 0;
}

/**
* Déverrouille l’accès au mfifo en lecture
*
* @param fifo 	objet mfifo dont il faut lever un verrou
* @return     	renvoie 0 lors de la libération du verrou , -1 en cas d'échec
*/
int mfifo_unlock(mfifo *fifo){
	if(sem_post(&fifo->sem) == -1 ){
		perror("sem post  ");
		return -1;
	}
	return 0;
}

/*
* Informe de la capacité totale de l'objet mfifo
*
* @param fifo 	objet mfifo dont l'on savoir la capacité de stockage
* @return 		renvoie la capicité totale de stockage de fifo
*/
size_t mfifo_capacity(mfifo *fifo){
	return fifo->capacity;
}

/*
* Libere la mémoire allouer par l'objet fifo et ses diverses champs
*
* @param 	fifo ojbet mfifo dont l'on veut libérer la mémoire
* @return 	renvoie la taille du fifo apres libération de mémoire
*/
int free_mfifo(mfifo *fifo){
	printf("dans free mfifo\n");
	free(&fifo->nom);
	if( strlen(fifo->memory) !=0 ){
		free(fifo->memory);
	}
	if(sem_destroy(&fifo->sem) == -1){
		perror("destroy semaphore ");
		return -1;
	}
	return sizeof(fifo);
}

/**
* Renvoie la quantié de mémoire libre de l'objet mfifo
*
* @param fifo 	objet mfifo dont l'on veut savoir la quantité de mémoire libre
* @return 		renvoie la quantité de mmémoire libre de fifo
*/
size_t mfifo_free_memory(mfifo *fifo){
	return (fifo->capacity - strlen(fifo->memory) );
}

/**
* Cette fonction déconnect, rend inutilisable, un objet mfifo, retourn -1 en
* cas d'erreur sinon 0
*
* @param fifo	objet mfifo à rendre ne plus utiliser
* @return 		renvoie 0 en cas de succès, -1 sinon
*/
int mfifo_disconnect(mfifo *fifo){
	if( munmap(fifo->memory , fifo->capacity) == -1){
		perror("munmap ");
		return -1;
	}
	return 0;
}

/**
* Supprime un objet mfifo par son nom 
*
* @param nom 	nom de l'objet mfifo à supprimer
* @return 		retourne -1 en cas d'erreur sinon 0
*/
int mfifo_unlink(const char * nom){
	printf("dans unlink : nom := %s \n",nom );
	int r = shm_unlink(nom);
	if( r == -1){
		perror("shm unlink");
	}
	return r;
}