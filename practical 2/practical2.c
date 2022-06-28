
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "cond.c"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condV = PTHREAD_COND_INITIALIZER;
int producer_running = 1;



int pnum;  // number updated when producer runs.
int csum;  // sum computed using pnum when consumer runs.

int (*pred)(int); // predicate indicating number to be consumed

int produceT() {
  scanf("%d",&pnum); // read a number from stdin
  return pnum;
}

void *Produce(void *a) {
  int p;

  p=1;
  while (p) {
    pthread_mutex_lock(&mutex);                //using mutex to lock the producer part
    printf("@P-STARTWORKING\n");               //set a print statement to make sure the producer starts executing
	printf("@P-READY\n");
    p = produceT();
    printf("@PRODUCED %d\n",p);
    pthread_cond_signal(&condV);               //initialize cond signal condV
    producer_running = 0;                      //after running the producer, set the producer running boolean register into 0 
    while (producer_running == 0)
      pthread_cond_wait(&condV, &mutex);       //check whether the consumer is running or not, 
	                                             //if it is running, then the producer part should be 
											                         //waited until consumer finish its procedure
    pthread_mutex_unlock(&mutex);              //unlock producer
  }
  printf("@P-EXIT\n");
  pthread_exit(NULL);
}


int consumeT() {
  if ( pred(pnum) ) { csum += pnum; }
  return pnum;
}

void *Consume(void *a) {
  int p;

  p=1;
  while (p) {
    pthread_mutex_lock(&mutex);                //using mutex to lock the consumer part
    while (producer_running == 1)
    	pthread_cond_wait(&condV, &mutex);       //check whether the producer is running or not, 
	                                             //if it is running, then the consumer part should be 
											                         //waited until producer finish its procedure
    printf("@C-STARTWORKING\n");               //set a print statement to make sure the consumer starts executing
    printf("@C-READY\n");
    p = consumeT();
    printf("@CONSUMED %d\n",csum);
    pthread_cond_signal(&condV);
    producer_running = 1;                      //after consumer executed, set the producer running boolean register to 1
                                               //in order to make producer starts executing
    pthread_mutex_unlock(&mutex);              //unlock consumer
  }
  printf("@C-EXIT\n");
  pthread_exit(NULL);
}


int main (int argc, const char * argv[]) {
  // the current number predicate
  static pthread_t prod,cons;
	long rc;

  pred = &cond1;
  if (argc>1) {
    if      (!strncmp(argv[1],"2",10)) { pred = &cond2; }
    else if (!strncmp(argv[1],"3",10)) { pred = &cond3; }
  }


  pnum = 999;
  csum=0;
  srand(time(0));

  printf("@P-CREATE\n");
 	rc = pthread_create(&prod,NULL,Produce,(void *)0);
	if (rc) {
			printf("@P-ERROR %ld\n",rc);
			exit(-1);
		}
  printf("@C-CREATE\n");
 	rc = pthread_create(&cons,NULL,Consume,(void *)0);
	if (rc) {
			printf("@C-ERROR %ld\n",rc);
			exit(-1);
		}

  printf("@P-JOIN\n");
  pthread_join( prod, NULL);
  printf("@C-JOIN\n");
  pthread_join( cons, NULL);


  printf("@CSUM=%d.\n",csum);

  return 0;
}
