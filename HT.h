#include "BF.h"

#define INIT_BLOCKS 8

typedef struct
{
	int fileDesc;
	char attrType;
	char *attrName;
	int attrLength;
	int depth;
}HT_info;




typedef struct
{
	int counter,local_depth , overflow ,previous;

}bucket_info;

int HT_CreateIndex( char * , char  , char * , int  , int  );

HT_info* HT_OpenIndex(char * );

int HT_CloseIndex( HT_info * );

int HT_InsertEntry(HT_info * , Record );

void HT_GetAllEntries( HT_info* ,  void *);

int hash_keyInt(int ,int) ;  //hash funtion apo to link http://www.cs.hmc.edu/~geoff/classes/hmc.cs070.200101/homework10/hashfuncs.html

int hash_keyChar(char * , int ) ;    //hash funtion apo to link http://stackoverflow.com/questions/7666509/hash-function-for-string

int hashing(HT_info * , Record , int);

int write_in_overflow(int  , int  , Record  );

void total_records( int , int  , Record * , int * );

void fill_array( int , int  , Record * , int * );

void empty_bucket(int , int  );

void create_and_write_to_overflow(int , int  ,Record );
