#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>


/*******************************
 * creation et initialisation de memoire partagee.
 * Si la zone existe deja, elle est simplement renvoyee
 * Par analogie avec malloc(), retourne 0 si erreur
 *******************************/
void * shmalloc(key_t key, int size);

/*******************************
 * liberation de memoire partagee.
 * Si la zone a deja ete liberee, ne fait rien (mais retourne -1)
 *******************************/
int shmfree(key_t key);



/*******************************
 * cree un MUTEX ou SEMAPHORE
 * La fonction renvoie le semid associe en cas de succes, -1 en cas
 *	d'echec.
 * Si le semaphore existe deja, la valeur initiale est ignoree.
 *******************************/
int mutalloc(key_t key);
int semalloc(key_t key, int valInit);

static struct sembuf sP = {0,-1,0};	/* decrementation du mutex */
static struct sembuf sV = {0, 1,0};	/* incrementation du mutex */

/*******************************
 * attend un MUTEX ou SEMAPHORE
 *******************************/
void P(int semid);
/*******************************
 * libere un MUTEX ou SEMAPHORE
 *******************************/
void V(int semid);

/*******************************
 * detruit un MUTEX ou SEMAPHORE
 * Si le semaphore a deja ete libere, retourne -1
 *******************************/
int mutfree(int mId);
