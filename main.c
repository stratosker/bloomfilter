#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "oracle.h"
#include "hash.h"
#include "list.h"
#include "sort.h"
#include <inttypes.h>
#include <math.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>

char *bloomFilter;
pthread_mutex_t *bloomMutex;
//pthread_mutex_t mtx2;
pthread_mutex_t wordFoundMtx;
pthread_mutex_t logMtx;
pthread_mutex_t rTmtx;
int sock;
int wordFound=0;
int remainingThreads;



void* serverJob(void* arg){
	int i, j=0;
	Data* data;

	data = malloc(sizeof(Data));

    data = (Data*) arg;

    if ( write (data->sock , data , sizeof(Data)) < 0){
		perror("write Server");
		exit(-1);
    }
    for(i=0; i<((data->size*8)/16834); i++){
		pthread_mutex_lock(&bloomMutex[i]);
	}

	while(( write_all(data->sock , bloomFilter+j, 2048) > 0)){
		j++;
	}
	
	for(i=0; i<((data->size*8)/16834); i++){
		pthread_mutex_unlock(&bloomMutex[i]);
	}

	pthread_exit(NULL);

}


void * f ( void * arg ) {

	int i, j=0, sameHash=0, tries = 0, noMoreWords = 0, bitPersection=16834, k, newSize, oracleCalls=0;
	double numOfWords = 0,numOfTriedWords=0;
	int seed = pthread_self();
	char seedWord[10] = {0};
	uint64_t *hashTo;
	int numOfHashFun=3;
	double size = 20971520;
	const char ** words;
	char **copy;
	struct ListNode *head;
	int *sections;
	Data* data;


	head = (struct ListNode*) sizeof(struct ListNode);
    head=NULL;

    sections = malloc(sizeof(int)*numOfHashFun);
    hashTo = malloc(sizeof(uint64_t)*numOfHashFun);

    data = malloc(sizeof(Data));

    data = (Data*) arg;
    size = data->size;
    numOfHashFun = data->numOfHashFun;
    

	//printf("THREADDDDDDDDDDDDDDDDDDDDDD %lf tries = %d\n", (double)pthread_self(), data->tries);
	while(tries<data->tries){
		head=NULL;
		tries++;
		noMoreWords=0;

		//if(is_empty(head)){
		//		printf("YES IS EMPTY\n");
		//}
		//printf("NEW TRY thread: %lf\n", (double)pthread_self());
		for(i=0; i<5; i++){
			seedWord[i] = rand_r(&seed) % ('W' - 'A') + 'A';
		}
		//printf("Seedword: %s\n", seedWord);

		//copy = malloc(sizeof(char*));


		if(oracleCalls%30==0){
			pthread_mutex_lock(&wordFoundMtx);
			if(wordFound==1){
				printf("Another thread found the word. Goodbye!\n");
				pthread_mutex_unlock(&wordFoundMtx);
				pthread_mutex_lock(&logMtx);
				fprintf(data->fp,"Thread id:%.0lf\nNumber of Words:%.0lf\nPercentage of Words already in Bloom Filter:%lf\n\n", (double)pthread_self(), numOfWords, numOfTriedWords/numOfWords);
				pthread_mutex_unlock(&logMtx);
				pthread_mutex_lock(&rTmtx);
				remainingThreads--;
				pthread_mutex_unlock(&rTmtx);
				if(remainingThreads==0){
					shutdown(sock,SHUT_RDWR);
				}
				pthread_exit(NULL);
			}
			pthread_mutex_unlock(&wordFoundMtx);
		}
		words = oracle(seedWord);
		oracleCalls++;
		//printf("I\n");
		if(words[0]==NULL){
				//printf("YES IS NULL1 thread: %lf\n", (double)pthread_self());
				//fflush(stdout);
				continue;
		}

		while(1){
			
			fflush(stdout);
			if(words==NULL){
				printf("IV\n");
			}
			while(words[0]==NULL){
				if(!is_empty(head)) {
					//printf("Hello5\n");
					fflush(stdout);
					//strcpy(parentName, head->data);


					if(oracleCalls%30==0){
						pthread_mutex_lock(&wordFoundMtx);
						if(wordFound==1){
							printf("Another thread found the word. Goodbye!\n");
							pthread_mutex_unlock(&wordFoundMtx);
							pthread_mutex_lock(&logMtx);
							fprintf(data->fp,"Thread id:%.0lf\nNumber of Words:%.0lf\nPercentage of Words already in Bloom Filter:%lf\n\n", (double)pthread_self(), numOfWords, numOfTriedWords/numOfWords);
							pthread_mutex_unlock(&logMtx);							
							pthread_mutex_lock(&rTmtx);
							remainingThreads--;
							pthread_mutex_unlock(&rTmtx);
							if(remainingThreads==0){
								shutdown(sock,SHUT_RDWR);
							}
							pthread_exit(NULL);
						}
						pthread_mutex_unlock(&wordFoundMtx);
					}
					words = oracle(head->data);
					oracleCalls++;
				}
				else{
					printf("NO MORE WORDS TO CHECK!!!\n");
					fflush(stdout);
					noMoreWords=1;
					break;
					//pthread_exit(NULL);
					//return;
				}
				//printf("Hello5\n");
				//printf("I CHECK %s\n",(char*)head->data );
				fflush(stdout);
				if(words==NULL){
					printf("HIDDEN WORD: %s\n", (char*)head->data);
					//printTree(root);
					pthread_mutex_lock(&wordFoundMtx);
					wordFound=1;
					pthread_mutex_unlock(&wordFoundMtx);
					pthread_mutex_lock(&logMtx);
					fprintf(data->fp,"Thread id:%.0lf\nNumber of Words:%.0lf\nPercentage of Words already in Bloom Filter:%lf\n\n", (double)pthread_self(), numOfWords, numOfTriedWords/numOfWords);
					pthread_mutex_unlock(&logMtx);
					pthread_mutex_lock(&rTmtx);
					remainingThreads--;
					pthread_mutex_unlock(&rTmtx);
					if(remainingThreads==0){
						shutdown(sock,SHUT_RDWR);
					}					
					pthread_exit(NULL);
					return;
				}
				delete_head(&head);
			}
			if(noMoreWords==1){
				break;
			}
			
			while(words[j]!=NULL){
				for(i=0; i<numOfHashFun; i++){
					hashTo[i] = hash_by(i, words[j]) %(uint64_t)size;
					//printf("Bit %" PRIu64" is in Section %"PRIu64"\n",hashTo, hashTo/bitPersection);
					sections[i] = hashTo[i]/bitPersection;
					
				}
				quick_sort(sections, numOfHashFun);
				newSize = removeDuplicates(sections,numOfHashFun);

				for(i=0; i<newSize; i++){
					pthread_mutex_lock(&bloomMutex[sections[i]]);
				}

				for(i=0; i<numOfHashFun; i++){
					if(( bloomFilter[hashTo[i]/8] >> (hashTo[i]%8) & 1) ==1){
						sameHash++;
					}
					else{
						bloomFilter[hashTo[i]/8] |= 1 << (hashTo[i]%8);
					}

				}


				for(i=0; i<newSize; i++){
					//printf("UNLOCK %d\n", sections[i]);
					//printf("%d: %d\n",i, sections[i] );
					pthread_mutex_unlock(&bloomMutex[sections[i]]);
				}


				numOfWords++;
				if(sameHash!=numOfHashFun){
					//if(nodeDepth(root, parentName)<depth){
					//	treeInsert(&root,parentName,copy[j]);
					//}
					listInsertBeginning(&head,(char*)words[j]);;
				}
				else{
					//printf("I HAVE SEARCH %s\n",copy[j]);
					numOfTriedWords++;
				}
					
				//free(copy[j]);
				//free((char*)words[j]);
				j++;
				sameHash=0;


			}
			//printf("Hello5\n");

			if(!is_empty(head)) {
				//strcpy(parentName, head->data);
				for(i=0; i<j; i++){
					free((void*)words[i]);
				}
				//free(words);

				if(oracleCalls%30==0){
					//printf("I WILL CHECK1\n");
					pthread_mutex_lock(&wordFoundMtx);
					if(wordFound==1){
						printf("Another thread found the word. Goodbye!\n");
						pthread_mutex_unlock(&wordFoundMtx);
						pthread_mutex_lock(&logMtx);
						fprintf(data->fp,"Thread id:%.0lf\nNumber of Words:%.0lf\nPercentage of Words already in Bloom Filter:%lf\n\n", (double)pthread_self(), numOfWords, numOfTriedWords/numOfWords);
						pthread_mutex_unlock(&logMtx);	
						pthread_mutex_lock(&rTmtx);
						remainingThreads--;
						pthread_mutex_unlock(&rTmtx);
						if(remainingThreads==0){
							shutdown(sock,SHUT_RDWR);
						}					
						pthread_exit(NULL);
					}
					pthread_mutex_unlock(&wordFoundMtx);
				}
				words = oracle(head->data);
				oracleCalls++;
				if(words==NULL){
					printf("HIDDEN WORD: %s\n", (char*)head->data);
					pthread_mutex_lock(&wordFoundMtx);
					wordFound=1;
					pthread_mutex_unlock(&wordFoundMtx);
					pthread_mutex_lock(&logMtx);
					fprintf(data->fp,"Thread id:%.0lf\nNumber of Words:%.0lf\nPercentage of Words already in Bloom Filter:%lf\n\n", (double)pthread_self(), numOfWords, numOfTriedWords/numOfWords);
					pthread_mutex_unlock(&logMtx);	
					pthread_mutex_lock(&rTmtx);
					remainingThreads--;
					pthread_mutex_unlock(&rTmtx);
					if(remainingThreads==0){
						shutdown(sock,SHUT_RDWR);
					}				
					pthread_exit(NULL);
					return;
				}
			}
			else{
				printf("NO MORE WORDS TO CHECK!!!\n");
				noMoreWords=1;
				break;
				//pthread_exit(NULL);
				//return;
			}

			delete_head(&head);
			j=0;
			//printf("Hello6\n");
		}
		noMoreWords=0;
	}
	printf("NO MORE WORDS TO CHECK!!!\n");
	pthread_mutex_lock(&logMtx);
	fprintf(data->fp,"Thread id:%.0lf\nNumber of Words:%.0lf\nPercentage of Words already in Bloom Filter:%lf\n\n", (double)pthread_self(), numOfWords, numOfTriedWords/numOfWords);
	pthread_mutex_unlock(&logMtx);
	pthread_mutex_lock(&rTmtx);
	remainingThreads--;
	pthread_mutex_unlock(&rTmtx);
	if(remainingThreads==0){
		shutdown(sock,SHUT_RDWR);
	}	
}	


int main(int argc, char* argv[]){

	int numOfThreads = 2, err, i, a=0, j, numOfBloomMutex, k=0;
	int port,newsock;
	double size=20971520;
	pthread_t * tids;
	pthread_t serverTid;
	pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
	Data* data, *newData;
	char address[100] = {0};
	struct sockaddr_in server, client;
	socklen_t clientlen;
	struct sockaddr * serverptr =(struct sockaddr *)&server;
	struct sockaddr * clientptr =(struct sockaddr *)&client;
	struct hostent * rem;
	//pthread_mutex_t *bloomMutex;
	
	srand((unsigned int) time(NULL));

	setEasyMode();
	//setHardMode();
	initSeed(100);
	initAlloc(malloc);
	

	pthread_mutex_init(&wordFoundMtx,NULL);
	pthread_mutex_init(&logMtx,NULL);
	pthread_mutex_init(&rTmtx, NULL);

	numOfBloomMutex = (size*8)/16834;

	bloomFilter = malloc(sizeof(char) * size);

	bloomMutex = malloc(sizeof(pthread_mutex_t)*numOfBloomMutex);

	data = malloc(sizeof(Data));
	newData  = malloc(sizeof(Data));

	if(argc>10||argc<6){
		printf("Wrong arguments!!!\n");
		return -1;
	}

	data->size = atof(argv[1]);
	data->tries = atoi(argv[3]);
	data->fp = fopen(argv[5], "w");
	numOfThreads = atoi(argv[2]);
	data->numOfThreads = numOfThreads;
	port = atoi(argv[4]);
	data->port = port;
	data->logFile = malloc((sizeof(char)*strlen(argv[5])) +1);
	strcpy(data->logFile,argv[5]);

	
	if(argc==8){
		if(strcmp(argv[6], "-k")==0){
			data->numOfHashFun = atoi(argv[7]);
		}
		else if(strcmp(argv[6], "-h")==0){
			strcpy(address, argv[7]);
			data->numOfHashFun = 3;
		}
	}
	else if(argc==10){
		data->numOfHashFun = atoi(argv[7]);
		strcpy(address, argv[9]);

	}
	else{
		data->numOfHashFun = 3;
	} 

	remainingThreads = numOfThreads;
	

	//printf("size = %lf threads = %d tries = %d port = %d log = %s k = %d address = %s\n",data->size, numOfThreads, data->tries, port, argv[5], data->numOfHashFun, address );

	for(i=0; i<numOfBloomMutex; i++){
		pthread_mutex_init(&bloomMutex[i],NULL);
		//bloomMutex[i] = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
	}

	if(((argc==8)&&(strcmp(argv[6], "-h")==0)) || ((argc==10)&&(strcmp(argv[8], "-h")==0))){
		if (( sock = socket ( PF_INET , SOCK_STREAM , 0) ) < 0){
			perror("socket");
			return -1;
		}
		if (( rem = gethostbyname(address)) == NULL ) {
			perror("gethostbyname");
			exit (1) ;
		}

		server.sin_family = AF_INET;
		memcpy (&server.sin_addr ,rem->h_addr,rem->h_length);
		server.sin_port = htons(port);

		if ( connect ( sock , serverptr , sizeof ( server ) ) < 0){
			perror("connect ");
			return -1;
		}
		if (read (sock , newData, sizeof(Data)) < 0){
			perror ("read") ;
		}
		if(data->size!=newData->size){
			printf("Wrong new arguments\n");
			return -1;
		}
		
		while ((read(port,bloomFilter+j,2048)) > 0){
			k++;
		}
		close(sock);
	}

	else{
		for(i=0; i<size; i++){//bloomfilter full of 0
			for(j=0; j<CHAR_BIT; j++){
				bloomFilter[i] &= ~(1 << j+1);
			}
		}
	}

	if (( tids = malloc(numOfThreads * sizeof (pthread_t))) == NULL ) {
		perror ("malloc") ;
		return -1;
	}

	for(i=0; i<numOfThreads; i++){
		if ( err = pthread_create (tids+i , NULL , f , data)) {
			perror("pthread_create") ;
			exit (1);
		}
		else{
			//printf("CREATE\n");
		}
	}
	

	//Server Code
	if (( sock = socket ( PF_INET , SOCK_STREAM , 0) ) < 0){
		perror("socket");
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);

	if (bind(sock, serverptr , sizeof(server))< 0){
		perror("bind");
		return -1;
	}

	if (listen(sock, 5)<0){
		perror("listen");
		return -1;
	}

	
	while (1) {
		if (( newsock = accept (sock, clientptr, & clientlen)) < 0) {
			if(errno==EINVAL){
				break;
			}
			perror("accept");
			printf("%s\n", strerror(errno));
			return -1;
		}

		
		
		close (sock); 
		//c h i l d _ s e r v e r ( newsock ) ;
		data->sock = newsock;
		if ( err = pthread_create (&serverTid , NULL , serverJob , data)) {
			perror("pthread_create") ;
			exit (1);
		}
		if (err = pthread_join (serverTid , NULL)) {
			perror("pthread_join") ;
			exit (1) ;
		}
		//exit(0);
		
		close (newsock) ; 
	}

	//End of Server Code

	for(i=0; i<numOfThreads; i++){
		if (err = pthread_join (*(tids + i) , NULL)) {
		perror("pthread_join") ;
		exit (1) ;
		}
	}

	fclose(data->fp);
	//printf("HELLO\n");
	return 0;
}