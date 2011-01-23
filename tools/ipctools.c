#include "../headers/ipctools.h"


/*******************************
 * creation et initialisation de memoire partagee.
 * Si la zone existe deja, elle est simplement renvoyee
 * Par analogie avec malloc(), retourne 0 si erreur
 *******************************/
void * shmalloc(key_t key, int size) {
	void * res;
	
	/* on essaie d'abord une zone existante */
	int shmid = shmget(key,1,0600);
	if (shmid == -1) 
		shmid = shmget(key,size,IPC_CREAT|IPC_EXCL|0600);
	if (shmid == -1) 
		return 0;
	
	res =  shmat(shmid, 0, 0);
	if (res == (void*)-1) {
		shmctl(shmid, IPC_RMID, 0);
		return 0;
	}
	return res;
}

/*******************************
 * liberation de memoire partagee.
 * Si la zone a deja ete liberee, ne fait rien (mais retourne -1)
 *******************************/
int shmfree(key_t key) {
	int shmid = shmget(key,1,0);
	return shmctl(shmid, IPC_RMID, 0);
}



/*******************************
 * cree un MUTEX ou SEMAPHORE
 * La fonction renvoie le semid associe en cas de succes, -1 en cas
 *	d'echec.
 * Si le semaphore existe deja, la valeur initiale est ignoree.
 *******************************/
int mutalloc(key_t key) { return semalloc(key, 1); }
int semalloc(key_t key, int valInit) {
  int semid = semget(key,1,0);
  if (semid == -1){
    semid = semget(key,1,IPC_CREAT|IPC_EXCL|0600);
    if (semid == -1)
      return -1;
    if (semctl(semid,0,SETVAL,valInit) == -1) {
      /* il faut detruire le semaphore */
      semctl(semid,0,IPC_RMID,0);
      return -1;
    }  
  }
  return semid;
}

/*******************************
 * attend un MUTEX ou SEMAPHORE
 *******************************/
void P(int semid){
  semop(semid,&sP,1);
}

/*******************************
 * libere un MUTEX ou SEMAPHORE
 *******************************/
void V(int semid) {
  semop(semid,&sV,1);
}

/*******************************
 * detruit un MUTEX ou SEMAPHORE
 * Si le semaphore a deja ete libere, retourne -1
 *******************************/
int mutfree(int mId) { return semfree(mId); }
int semfree(int semid) {
  return semctl(semid,0,IPC_RMID,0);
}
