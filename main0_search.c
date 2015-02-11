#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BF.h"
#include "HP.h"
#include "HT.h"
#define FILENAME_HEAP 		"heap"
#define NAME_SIZE 			15
#define SURNAME_SIZE		20
#define DATE_OF_BIRTH_SIZE 	10
#define FILENAME_HASH_ID "hash_id"

int main(void)
{
    	int fileDesc,heapFile;
        HT_info *hashId;
    	Record record;
    	BF_Init();

        /* Open fake hash file */
	if ((fileDesc = HP_OpenFile(FILENAME_HASH_ID)) < 0) 
	{ 
		fprintf(stderr, "Error opening heap file.\n");
	//	exit(EXIT_FAILURE);
	}	


	/* Open fake hash file*/
	if ((hashId = HT_OpenIndex(FILENAME_HEAP)) == NULL) 
	{ 
		fprintf(stderr, "Error opening hash index.\n");
		//HT_CloseIndex(hashId);
	//	exit(EXIT_FAILURE);
	}

	/* Open heap file */
	if ((heapFile = HP_OpenFile(FILENAME_HEAP)) < 0) 
	{ 
		fprintf(stderr, "Error opening heap file.\n");
	//	exit(EXIT_FAILURE);
	}

	int id=220;
	printf("\nSearch Heap file entries before inserts:\n");

	printf("search  id 220: \n");
	HP_GetAllEntries(heapFile, "id", &id);
	printf("search surname Papadopoulos:\n");
 	HP_GetAllEntries(heapFile, "surname", "Papadopoulos");
    	HP_GetAllEntries(heapFile, NULL, NULL);

	printf("Insert new records\n");

	while (!feof(stdin)) 
	{ /* read line, until eof */

		scanf("%d %s %s %c %s %d %c %d %d", &record.id, record.name, record.surname, record.status, record.dateOfBirth, &record.salary, record.section, &record.daysOff, &record.prevYears);
				
		/* Insert record in heap file */
		if (HP_InsertEntry(heapFile, record) < 0) 
		{ 
			fprintf(stderr, "Error inserting entry in heap file.\n");
			HP_CloseFile(heapFile);
			exit(EXIT_FAILURE);
		}

	}


	printf("\nHeap file after insert, search id 220 :\n");
	HP_GetAllEntries(heapFile, "id", &id);
	printf("Search surname papadopoulos:\n");
 	HP_GetAllEntries(heapFile, "surname", "Papadopoulos");
    	HP_GetAllEntries(heapFile, NULL, NULL);


	/* Close heap file */
	if (HP_CloseFile(heapFile) < 0) 
	{ 
		fprintf(stderr, "Error closing heap file.\n");
		exit(EXIT_FAILURE);
	}
	printf("\n\nHash Table Functions Checking\n\n");
	/* Open hash index based on id */ 
	if ((hashId = HT_OpenIndex(FILENAME_HASH_ID)) == NULL) 
	{ 
		fprintf(stderr, "Error opening hash index.\n");
		HT_CloseIndex(hashId);
		exit(EXIT_FAILURE);
   	}

 	// Get entries before insert
	printf("Before insert, search record with id 220: \n");
	HT_GetAllEntries(hashId, &id);
	printf("search  all\n");
    	HT_GetAllEntries(hashId, NULL);
	FILE * fp;
	fp = fopen("records2", "r");    	

	if ( fp != NULL )
   	{
      		char line [ 128 ]; /* or other suitable maximum line size */
		while ( fscanf(fp,"%d %s %s %c %s %d %c %d %d", &record.id, record.name, record.surname, record.status, record.dateOfBirth, &record.salary, record.section, &record.daysOff, &record.prevYears) != EOF) 
		{ 				
			/* Insert record in hash index based on id*/ 
			if (HT_InsertEntry(hashId, record) < 0) 
			{	 
				fprintf(stderr, "Error inserting entry in hash index\n");
				HT_CloseIndex(hashId);
				exit(EXIT_FAILURE);
			}
		}
	}
	// Get entries
	printf("\nSearch id 220 after insert:\n");
	HT_GetAllEntries(hashId, &id);
    	printf("Find all records:\n");
	HT_GetAllEntries(hashId, NULL);


  	/* Close id hash index */
	if (HT_CloseIndex(hashId) < 0) 
	{ 
		fprintf(stderr, "Error closing id hash index.\n");
		exit(EXIT_FAILURE);
	}


    return 0;
}


