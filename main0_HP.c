#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "HP.h"

#define FILENAME_HEAP 		"heap"
#define NAME_SIZE 			15
#define SURNAME_SIZE		20
#define DATE_OF_BIRTH_SIZE 	10


int main(int argc, char** argv) 
{
	int heapFile;

	BF_Init();
	
	/* Create heap file */
	if(HP_CreateFile(FILENAME_HEAP) < 0) 
	{
		fprintf(stderr, "Error creating heap file.\n");
		exit(EXIT_FAILURE);
	}
	
	/* Open heap file */
	if ((heapFile = HP_OpenFile(FILENAME_HEAP)) < 0) 
	{ 
		fprintf(stderr, "Error opening heap file.\n");
		exit(EXIT_FAILURE);
	}

	/*while (!feof(stdin)) 
	{ 

		scanf("%d %s %s %c %s %d %c %d %d", &record.id, record.name, record.surname, record.status, record.dateOfBirth, &record.salary, record.section, &record.daysOff, &record.prevYears);
				
		
		if (HP_InsertEntry(heapFile, record) < 0) 
		{ 
			fprintf(stderr, "Error inserting entry in heap file.\n");
			HP_CloseFile(heapFile);
			exit(EXIT_FAILURE);
		}
	}
	*/
	/* Close heap file */
	if (HP_CloseFile(heapFile) < 0) 
	{ 
		fprintf(stderr, "Error closing heap file.\n");
		exit(EXIT_FAILURE);
	}

	/* **Print blocks */ 

	return EXIT_SUCCESS;
}
