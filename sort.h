void quick_sort (int *a, int size);
void q_sort(int *a, int left, int right);
int removeDuplicates(int *array, int length);
int write_all(int fd ,void * buff ,size_t size);

typedef struct Data{
	double size;
	int numOfThreads;
	int tries;
	int port;
	int numOfHashFun;
	FILE* fp;
	char *logFile;
	int sock;
}Data;