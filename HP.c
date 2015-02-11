/*File HP.c*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "BF.h"
#include "HP.h"

int HP_CreateFile(char* filename){
	void* block;	
	int fd;															
	Info info;
	info.type=0;
	info.totalRecords=0;							
	if((BF_CreateFile(filename))<0){							/*dhmiourgia arxeiou*/
		BF_PrintError("HP_CreateFile//");
		return -1;
	}
	if((fd=BF_OpenFile(filename))<0){						/*anoigma arxeiou*/
		BF_PrintError("HP_CreateFile//");
		return -1;
	}
	if(BF_AllocateBlock(fd)<0){							/*desmeysh mnhmhs gia to block 0*/
		BF_PrintError("HP_CreateFile//");
		return -1;
	}
	if(BF_ReadBlock(fd, 0, &block) < 0) {					/*pairnoume tin dieythinsi tou block 0*/
		BF_PrintError("HP_CreateFile//");
		return -1;
	}
	memcpy(block, &info, sizeof(Info));							/*antigrafoume to struct pliroforias sto block*/
	if(BF_WriteBlock(fd,0) < 0){							/*grafoume tis allages pou kaname sto block 0 ston disko*/
		BF_PrintError("HP_CreateFile//");
		return -1;
	}
	if(BF_CloseFile(fd) < 0) {							/*kleinoume to arxeio*/
		BF_PrintError("HP_CreateFile//");
		return -1;
	}
	return 0;
}

/****************************************************************************************************/

int HP_OpenFile( char *fileName ){	
	int fd,fileType;	
	void* block;
	if((fd=BF_OpenFile(fileName))<0){							/*anoigma arxeiou*/
		BF_PrintError("HP_OpenFile//");
		return -1;
	}
	if(BF_ReadBlock(fd, 0, &block) < 0) {							/*pairnoume tin dieythinsi tou block 0*/
		BF_PrintError("HP_OpenFile//");
		return -1;
	}
	memcpy(&fileType,block,sizeof(int));							/*diavazoume ton typo arxeiou apo to mplok 0*/
	if(fileType!=0)										/*elegxoume an einai arxeio swrou*/
		return -1;
	return fd;
}

/****************************************************************************************************/

int HP_CloseFile( int fileDesk ){
	if(BF_CloseFile(fileDesk) < 0) {							/*kleinoume to arxeio*/
		BF_PrintError("HP_CloseFile//");
		return -1;
	}
	return 0;
}

/****************************************************************************************************/

int HP_InsertEntry (int fileDesc, Record record ) {
	void * block;
	int total_records,records,counter;	
	if (BF_ReadBlock( fileDesc,0,&block)<0)							/*pairnoume tin dieythinsi tou block 0*/
		return -1;
	memcpy(&total_records,block+sizeof(int),sizeof(int));				/*diavazoume ton arithmo twn synolikwn eggrafwn tou arxeiou*/
	if(total_records==0) {           								/*PRWTI EGGRAFI*/
		if(BF_AllocateBlock(fileDesc)<0)							/*desmeyoume mnhmh gia neo block*/
			return -1;	
		if(BF_ReadBlock(fileDesc,1,&block)<0)						/*pairnoume thn dieythinsi tou neou block pou desmeysame*/
			return -1;
		memcpy((block+sizeof(int)),&record,sizeof(Record));			/*grafoume tin eggrafi meta ton metriti eggrafon tou block*/		
		counter=1;											/*enhmerwnoume ton metrhth eggrafwn-efoson prwti eggrafi einai 1*/
		memcpy(block,&counter,sizeof(int));						/*grafoume  ton metrhth eggrafwn tou block*/			
	}
	else{
		records=total_records%((BLOCK_SIZE-sizeof(int))/(sizeof(Record)));   	/*vriskoume poses egrafes exei to teleuteo block*/
		if (records==0){										/*to teleuteo block einai gemato opote desmeuoume neo-idia diadikasia*/
			if(BF_AllocateBlock(fileDesc)<0)
				return -1;
			if(BF_ReadBlock(fileDesc,(BF_GetBlockCounter(fileDesc) -1),&block)<0)
				return -1;
			counter=1;
			memcpy(block,&counter,sizeof(int));
			memcpy((block+sizeof(int)),&record,sizeof(Record));
		}
		else{													/*to teleytaio block exei xwro-grafoume sto telos tou*/
			if(BF_ReadBlock(fileDesc,(BF_GetBlockCounter(fileDesc)-1),&block)<0)   
					return -1;
			memcpy((block+sizeof(int)+(records*sizeof(Record))),&record,sizeof(Record));/*to proto orisma deixnei ti thesi pou prepei na grapso*/
			records++;											/*ayksanoume kata mia ton arithmo twn eggrafwn tou block*/
			counter=records;
			memcpy(block,&counter,sizeof(int));						/*grafoume ton arithmo twn eggrafwn tou block sto block*/
		}
	}
	if( BF_ReadBlock(fileDesc,0,&block)<0)
		return -1;
	total_records++;												/*auksanoume tis sinolikes egrafes sto block 0( pliroforias)*/
	memcpy(block+sizeof(int),&total_records,sizeof(int));					/*grafoume ayton ton arithmo sto block0*/
	if(BF_WriteBlock(fileDesc,0)<0)									/*grafoume tis allages tou block 0 ston disko*/
		return -1;						
	if(BF_WriteBlock(fileDesc,(BF_GetBlockCounter(fileDesc)-1))<0)			/*grafoume tis allages tou teleytaiou block ston disko*/
		return -1;
	return 0;		

}

/**************************************************************************************************************************/

void HP_GetAllEntries(  int fileDesc,  char* fieldName,  void *value){
	int total_records,total_blocks,rpb,blocks_read=0,records_read=0,i=1,j=0,found=0;
	void * block;
	clock_t begin, end;
	double time_spent;
	Record record;
	if( BF_ReadBlock(fileDesc,0,&block)< 0) 
		BF_PrintError("HP_GetAllEntries1// ");
	memcpy(&total_records,(block+sizeof(int)),sizeof(int));	 			/* diavazoume apo to block 0 poses eggrafes exei to arxeio*/
	if((total_blocks=BF_GetBlockCounter(fileDesc))< 0) 					/* vriskoume ta synolika blocks tou arxeiou*/
		BF_PrintError("HP_GetAllEntries2// ");		
	printf("\nSearching record .... 	(in Total records:%d - Total blocks of information:%d)\n ",total_records,total_blocks-1);	
	if(total_records==0){											/*to arxeio den exei kamia eggrafi akoma*/
		printf("File has not any records yet...Please insert some and try again.\n");
		return;
	}			
	printf("\n |  ID  |      NAME      |       SURNAME       |   STATUS   |   DATE OF BIRTH   |   SALARY   |  SECTION  |  DAYS OFF  |  PREVIOUS YEARS | \n");		
	if ( (fieldName==NULL) && ( value== NULL) ) { 						/*h timh value==NULL, ektypwnontai oles oi eggrafes tou arxeiou*/
		begin = clock();											/*enarksi xronometrisis ektelesis entolwn*/
		for(;i<total_blocks;i++){									/*gia ola ta block eggrafwn (apo block 1 ews to teleytaio) */
			if(BF_ReadBlock(fileDesc,i,&block)< 0) 						/*pairnoume tin dieythinsi tou block*/
				BF_PrintError("HP_GetAllEntries3// ");
			memcpy(&rpb,block,sizeof(int));							/*diavazoume poses eggrafes exei to block*/			j=0;
			for(;j<rpb;j++){										/*gia oles tis eggrafes pou exei to block*/		
				memcpy(&record,(block+sizeof(int)+(j*sizeof(Record))),sizeof(Record));	/*diavazoume mia mia tin eggrafi*/
				records_read++;										
				printf(" |%6d|%16s|%21s|%12s|%19s|%12d|%11s|%12d|%17d|\n",record.id,record.name,record.surname,record.status
						,record.dateOfBirth,record.salary,record.section,record.daysOff,record.prevYears);/*ektypwnoume tin eggrafi*/		
				found++;
			}
			blocks_read++;
		}
		end = clock();												/*telos xronometrisis ektelesis entolwn*/
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;				/*evresi xronou ektelesis entolwn*/
	}
	else{
		begin = clock();
		for(;i<total_blocks;i++){									/*gia ola ta block eggrafwn (apo block 1 ews to teleytaio)*/
			if(BF_ReadBlock(fileDesc,i,&block)< 0) 
				BF_PrintError("HP_GetAllEntries4// ");
			memcpy(&rpb,block,sizeof(int));							/*diavazoume poses eggrafes exei to block*/			
			j=0;
			for(;j<rpb;j++){	
				memcpy(&record,(block+sizeof(int)+(j*sizeof(Record))),sizeof(Record));
				records_read++;
				if( 	(!strcmp("id",fieldName) && (record.id==* (int *) value)) /*elegxos an yparxei kapoia eggrafi symfwnh me ta krithria anazhthshs*/
					||	((!strcmp("name",fieldName))&& (!strcmp(record.name,value)))
					||	((!strcmp("surname",fieldName))&& (!strcmp(record.surname,value)))
					||	((!strcmp("status",fieldName))&& (!strcmp(record.status,value)))
					||	((!strcmp("dateOfbirth",fieldName))&& (!strcmp(record.dateOfBirth,value)))
					||	(!strcmp("salary",fieldName) && (record.salary==* (int *) value)) 
					||	((!strcmp("section",fieldName))&& (!strcmp(record.section,value)))
					||	(!strcmp("daysOff",fieldName) && (record.daysOff==* (int *) value)) 	
					||	(!strcmp("prevYears",fieldName) && (record.prevYears==* (int *) value))		)	
					{
						printf(" |%6d|%16s|%21s|%12s|%19s|%12d|%11s|%12d|%17d|\n",record.id,record.name,record.surname,record.status
									,record.dateOfBirth,record.salary,record.section,record.daysOff,record.prevYears);/*vrethike eggrafi-ektypwsh ths*/
						found++;										/*ayksisi metriti eggrafwn pou vrethikan me ayta ta krithria*/
					}
			}
			blocks_read++;												
		}
		end = clock();
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		if(found==0)													/*den vrethike eggrafi me ta krithria anazhthshs*/
			printf("There are no results with this value. Please try again\n");
		
	}
	printf("\n\n					***********HEAP/READ STATISTICS************\n");
	printf("					*                                         *\n");
	printf("					* | Records |  Blocks |   Time  | Found  |*\n");
	printf("					* |%9d|%9d|%9f|%8d|*\n",records_read, blocks_read, time_spent,found);
	printf("					*                                         *\n");
	printf("					*******************************************\n\n");
}



