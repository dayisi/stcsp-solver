#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include "util.h"

//#define DOMAIN_SIZE 16 

#if MEMORY
unsigned long mallocCount = 0;
unsigned long freeCount = 0;
#endif
using namespace std;

/* exported */
void *myMalloc2(size_t size, const char *func) {
    void *ptr;

    ptr = malloc(size);
    if (ptr == NULL) {
        myLog(LOG_ERROR, "Error: not enough memory! Malloc-ed in %s\n", func);
        exit(1);
    }
    myLog(LOG_DETAILED_TRACE, "Malloc-ed in %s\n", func);
#if MEMORY
    mallocCount++;
#endif

    return ptr;
}

/* exported */
void myFree2(void *ptr, const char *func) {
    if (ptr != NULL) {
#if MEMORY
        freeCount++;
#endif
        free(ptr);
        myLog(LOG_DETAILED_TRACE, "Free-ed in %s\n", func);
    }
}

/*****
 * Backup and Restore
 */

#define CHUNK_SIZE 1024
#define DOMAIN_SIZE 16

typedef struct _Chunk {
    int **addr;
    int *data;
    int ptr;
    struct _Chunk *prev;
    struct _Chunk *next;
} Chunk;

Chunk *memory = NULL;

typedef struct _Chunk_dm{
	vector<int> **addr;
	//vector<int> data[CHUNK_SIZE];
	//vector< vector<int> > data(CHUNK_SIZE, vector<int>(DOMAIN_SIZE, 0))
	vector< vector<int> >* data;
	int ptr;
	struct _Chunk_dm *prev;
	struct _Chunk_dm *next;
}Chunk_dm;

Chunk_dm *memory_dm = NULL;
int level = 0;

Chunk *newChunk() {
    Chunk *chunk;

    chunk = (Chunk *)myMalloc(sizeof(Chunk));
    chunk->addr = (int **)myMalloc(sizeof(int *) * CHUNK_SIZE);
    chunk->data = (int *)myMalloc(sizeof(int) * CHUNK_SIZE);
    chunk->ptr = 0;
    chunk->prev = NULL;
    chunk->next = NULL;

    return chunk;
}

Chunk_dm *newChunkDm(){
	//cout<<"allocate memory for memory_dm"<<endl;
	Chunk_dm *chunk;

	chunk = (Chunk_dm *)myMalloc(sizeof(Chunk));
	/*chunk = (Chunk_dm*)myMalloc(sizeof(Chunk_dm) + sizeof(int)  * DOMAIN_SIZE * CHUNK_SIZE + sizeof(vector<int>) * CHUNK_SIZE + (sizeof(vector<int>*) *CHUNK_SIZE));
	
	//cout<<"allocated memory size is "<<sizeof(Chunk_dm) + sizeof(int)  * DOMAIN_SIZE * CHUNK_SIZE + sizeof(vector<int>) * CHUNK_SIZE + (sizeof(vector<int>*) *CHUNK_SIZE)<<endl;
	//cout<<"size of vector<int> "<<sizeof(vector<int>)<<endl;
	//cout<<"size of integer "<<sizeof(int)<<endl;
	//cout<<" size of vector<int>* "<<sizeof(vector<int>*)<<endl;
	//cout<<"size of Chunk_dm "<<sizeof(Chunk_dm)<<endl;
    */
	if(chunk == NULL){
		//cout<<"ERROR: out of memory!"<<endl;
		exit(1);
	}
	//cout<<"allocate memory for address"<<endl;
	chunk->addr = (vector <int> **)myMalloc(sizeof(vector<int>*) *CHUNK_SIZE);
	if(chunk->addr == NULL){
		//cout<<"ERROR: out of memory"<<endl;
		exit(1);
	}
	//cout<<"build vector of vector"<<endl;
    chunk->data = new vector<vector<int> >(CHUNK_SIZE, vector<int>(DOMAIN_SIZE));
    for(int i = 0; i < CHUNK_SIZE; i++){
        for(int j = 0; j < DOMAIN_SIZE; j++){
            chunk->data->at(i).pop_back();
            //cout<<"capacity is "<<chunk->data->at(i).capacity()<<endl;
        } 
    }
      
	chunk->ptr = 0;
	chunk->prev = NULL;
	chunk->next = NULL;
	//cout<<"end of newChunkDm"<<endl;
	return chunk;
}

void freeMemory() {
    Chunk *chunk, *temp;

    chunk = memory;
    if(chunk != NULL){
        while (chunk->prev != NULL) {
            chunk = chunk->prev;
        }

        while (chunk != NULL) {
            myFree(chunk->addr);
            myFree(chunk->data);
            temp = chunk->next;
            myFree(chunk);
            chunk = temp;
        }
    }
	Chunk_dm *chunk_dm, *temp_dm;
	chunk_dm = memory_dm;
	if(chunk_dm != NULL){
        while (chunk_dm->prev != NULL) {
            chunk_dm = chunk_dm->prev;
        }

        while (chunk_dm != NULL) {
            //while(!chunk_dm->addr->empty()){
            //    chunk_dm->pop_back();
            //}
            myFree(chunk_dm->addr);
            while(!chunk_dm->data->empty()){
                while(!chunk_dm->data->back().empty()){
                    chunk_dm->data->back().pop_back();
                }
                chunk_dm->data->pop_back();
            }
            myFree(chunk_dm->data);
            temp_dm = chunk_dm->next;
            myFree(chunk_dm);
            chunk_dm = temp_dm;
        }
    }
}

/* exported */
//backup? backup the address of data ?
void backup(int *addr) {
    if (memory == NULL) {
        memory = newChunk();
    }

    if (addr == NULL) {
        memory->addr[memory->ptr] = NULL;
        memory->data[memory->ptr] = 0;
    } else {
        memory->addr[memory->ptr] = addr;
        memory->data[memory->ptr] = *addr;
    }
    memory->ptr++;

    if (memory->ptr == CHUNK_SIZE) { /* chunk full, get another chunk */
        if (memory->next == NULL) {
            memory->next = newChunk();
            memory->next->prev = memory;
        }
        memory = memory->next;
    }
}

void backup_dm(vector<int> *addr){
	if(memory_dm==NULL){
		memory_dm = newChunkDm();
	}
	if(addr == NULL){
		memory_dm->addr[memory_dm->ptr] = NULL;
		memory_dm->data->at(memory_dm->ptr) = vector<int>();
		//memory_dm->data[memory->ptr].reserve(DOMAIN_SIZE);
	} else{
		////cout<<"backup address"<<endl;
		////cout<<"memory_dm->ptr is "<<memory_dm->ptr<<endl;
		memory_dm->addr[memory_dm->ptr] = addr;
		////cout<<"backup data"<<endl;
		memory_dm->data->at(memory_dm->ptr) = *addr;
		//cout<<"end of backup domain"<<endl;
	}
	memory_dm->ptr++;
	if(memory_dm->ptr == CHUNK_SIZE){
		//cout<<"memory_dm full"<<endl;
		if(memory_dm->next == NULL){
			//cout<<"next is NULL, malloc new memory"<<endl;
			memory_dm->next = newChunkDm();	
			memory_dm->next->prev = memory_dm;
		}
		memory_dm = memory_dm->next;
		//cout<<"succeed to change another chunk"<<endl;
	}

}

void myPrintLevel(){
	//cout<<"level is "<<level<<endl;
}
void printLevel() {
    int i;

    myLog(LOG_TRACE, "%02d:", level);
    for (i = 0; i < level; i++) {
        myLog(LOG_TRACE, "        ");
    }
}

/* exported */
void levelUp() {
    level++;
    backup(NULL);
	backup_dm(NULL);
}

/* exported */
void levelDown() {
   // //cout<<"start level down..."<<endl;
    level--;
    if (memory->ptr == 0) {
        memory = memory->prev;
    }
    memory->ptr--;
    while (memory->addr[memory->ptr] != NULL) {
        *memory->addr[memory->ptr] = memory->data[memory->ptr];
        if (memory->ptr == 0) {
            memory = memory->prev;
        }
        memory->ptr--;
    }

	if(memory_dm->ptr == 0){
		memory_dm = memory_dm->prev;
	}
	memory_dm->ptr--;
	while(memory_dm != NULL && memory_dm->addr[memory_dm->ptr] != NULL){
		*(memory_dm->addr[memory_dm->ptr]) = memory_dm->data->at(memory_dm->ptr);	
		if(memory_dm->ptr == 0){
			memory_dm = memory_dm->prev;
		}
		if(memory_dm != NULL){
			memory_dm->ptr--;
		}
	}
    //freeMemory();
}

/* Time */
double cpuTime() {
    static struct tms buf;

    times(&buf);

    return ((double)(buf.tms_utime + buf.tms_stime + buf.tms_cutime + buf.tms_cstime)) / ((double)sysconf(_SC_CLK_TCK));
}

// int logLevel = LOG_DEBUG;
int logLevel = LOG_ERROR;
int color[] = { 0, 31, 33, 37, 35, 32, 36 };

void myLog(int level, char *format, ...) {
    va_list args;
    FILE *stream;
    if (level <= logLevel) {
        stream = fopen("error.txt", "a+"); //(level == LOG_ERROR ? stderr : stdout);
        va_start(args, format);
        //if (isatty(1)) {
        //    fprintf(stream, "%c[%d;%d;%dm", 0x1b, 0, color[level], 40);
        //}
        vfprintf(stream, format, args);
        //if (isatty(1)) {
        //    fprintf(stream, "%c[%dm", 0x1b, 0);
        //}
        va_end(args);
        fflush(stream);
        fclose(stream);

    }
	
}
