#include "BF.h"
#include "HT.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


int HT_CreateIndex( char *fileName , char attrType , char *attrName , int attrLength , int depth )     // ipothetoume oti to 2^depth >= 8(arxikoi kadoi )
{
	int fileDesc, counter=0, i ,  *array ,table_size=1 , mod ,j , sum , z;
	void *block;
	bucket_info BucketInfo;
	HT_info HTInfo;
	if ( BF_CreateFile( fileName ) )
	{
		BF_PrintError("Error creating file");
		return -1;
	}
	if ( (fileDesc = BF_OpenFile ( fileName )) < 0 )
	{
		BF_PrintError("Error opening file");
		return -1;
	}
	for(i=0 ; i <2 ; i++)	//one block will store hashing info , one will store hash_table 
	{

		if(BF_AllocateBlock(fileDesc))
		{
			BF_PrintError("Error allocating block");
			return -1;
		}
		counter=BF_GetBlockCounter(fileDesc) -1; 
		if( BF_ReadBlock(fileDesc,counter, &block) )
		{
			BF_PrintError("Error getting block");
			return -1;
		}

		if(i==0) //first block stores hashing info
		{
			HTInfo.fileDesc=fileDesc;
			HTInfo.attrType=attrType;
			HTInfo.attrName= malloc(sizeof(attrName) * sizeof(char));
			strcpy(HTInfo.attrName,attrName);
			HTInfo.attrLength=attrLength;
			HTInfo.depth=depth;
			memcpy(block,&HTInfo,sizeof(HT_info));
		}
		else //second block stores hash table
		{						
			for(z=0;z <depth ; z ++)
				table_size*=2;

			array=malloc( table_size * sizeof(int));
			mod=table_size / INIT_BLOCKS;
			sum=0;
			for(z=0 ; z < mod ; z ++)
				for(j=0 ; j < INIT_BLOCKS ; j++) //althought the actual buckets haven't been created yet, we make reference to their future bock address 
				{
					array[sum]=j+2;
					sum++;
				}
			memcpy(block,array,table_size*sizeof(int));		
		}
		
		if(BF_WriteBlock( fileDesc,counter) )
		{
			BF_PrintError("Error writing block back");
			return -1;
		}
		
	}
	for (i=0 ; i < INIT_BLOCKS   ; i ++)   //allocating first 8 buckets
	{

		if(BF_AllocateBlock(fileDesc))
		{
			BF_PrintError("Error allocating block");
			return -1;
		}
		counter=BF_GetBlockCounter(fileDesc) -1; 
		if( BF_ReadBlock(fileDesc,counter, &block) )
		{
			BF_PrintError("Error getting block");
			return -1;
		}
		BucketInfo.counter=0;    //they have no records
		if(table_size==INIT_BLOCKS){
			BucketInfo.local_depth=depth;
		}else{
			BucketInfo.local_depth=table_size/INIT_BLOCKS;
		}
		BucketInfo.previous=0;
		//BucketInfo.local_depth=table_size/INIT_BLOCKS;    //pointers to bucket  lathos!!!! arxika topiko vathos=oliko etsi den diplasiazetai pote
		//pointers to bucket == 2^table_size / BucketInfo.local_depth
		BucketInfo.overflow=0;
		memcpy(block,&BucketInfo ,sizeof(BucketInfo));
		if(BF_WriteBlock( fileDesc,counter) )
		{
			BF_PrintError("Error writing block back");
			return -1;
		}
		
	
	}
	free(array);

	if(BF_CloseFile( fileDesc) )
	{
		BF_PrintError("Error closing file");
		return-1;
	}

	return 0;
}

HT_info* HT_OpenIndex(char *fileName )
{
	HT_info* info;
	void *block;
	int fileDesc;
	if ( ( fileDesc = BF_OpenFile ( fileName ) ) < 0 )
	{
		BF_PrintError("Error opening file");
		return NULL;
	}
	if( BF_ReadBlock(fileDesc,0, &block) )
	{
		BF_PrintError("Error getting block");
		return NULL;
	}
	info= malloc( sizeof(HT_info) );
	memcpy(info,block,sizeof(HT_info));
	return info;

}


int HT_CloseIndex( HT_info * header_info)
{
	if(BF_CloseFile( header_info->fileDesc) )
	{
		BF_PrintError("Error closing file");
		return-1;
	}
	free(header_info);
	return 0;
}


int HT_InsertEntry(HT_info *header_info , Record record)
{
	
	void *block_evr, *block_hash, *block_alloc ;
	int table_size=1,i,index , counter ,size=0 , *array=NULL , *array_temp=NULL , flag=0, flag2=0 , index2 ,count;
	//int  i_o=2,totalBlocks_o=0,j_o,rpb_o=0;
	//Record record_o;
	//void *block;
	bucket_info BucketInfo, BucketInfo2 ;	
	Record *rec_array=NULL ;
	counter=BF_GetBlockCounter(header_info->fileDesc) -1;
	if(counter == 1)
		printf("There are no records in Hash File\n");
	return 1;
	for(i=0 ; i<header_info->depth ; i++)
		table_size *=2;
	if( BF_ReadBlock(header_info->fileDesc,1, &block_evr) ) //reading hash table block
	{
		BF_PrintError("Error getting block");
		return -1;
	}
	array=malloc(table_size * sizeof(int));
	memcpy(array,block_evr,(table_size * sizeof(int)));    //copying hash table from block to array	
	index=hashing( header_info , record , table_size);	 //key hashed to this bucket 
	if( BF_ReadBlock(header_info->fileDesc,array[index], &block_hash) ) 
	{
		BF_PrintError("Error getting block");
		return -1;
	}
	memcpy(&BucketInfo ,block_hash , sizeof(bucket_info));
	flag2=write_in_overflow(header_info->fileDesc , array[index] , record  ); //write in bucket ( overflow or actual bucket)
	if(!flag2)  // no space in "bucket", or in an overflow of "bucket"  to fit record.Overflow may or may not exists.
	{    
		total_records( header_info->fileDesc , array[index] , rec_array , &size);
		rec_array=malloc( (size) * sizeof(Record)); //copying all records of bucket to an array so that we can "re-hash" them
		fill_array( header_info->fileDesc , array[index] , rec_array , &size);	
		count=size;
		if(BucketInfo.local_depth == header_info->depth)    //only one pointer to bucket
		{			
			for(i=0 ; i<count ; i++)   //check if, in a possible double array , records will be hashed in more than one buckets 
			{
					index2=hashing( header_info , rec_array[i] , table_size*2);
					if(index!=index2)
					{
						flag=1;
						break;
					}
			}
			if(flag==0)
			{
				index2=hashing( header_info , record , table_size*2);
				if(index2!=index)
					flag=1; 
			}
	
			if(flag==1)//records will be distributed to more than one buckets , so we double the size of the hash table
			{		
				empty_bucket( header_info->fileDesc , array[index] );   //emptying block ("bucket") to so that we can rewrite from it's start
				header_info->depth++;
				BucketInfo.local_depth++;
			        memcpy(block_hash+sizeof(int),&(BucketInfo.local_depth),sizeof(bucket_info));
				if(BF_WriteBlock( header_info->fileDesc,array[index]) ) //saving buckets new local depth
				{
					BF_PrintError("Error writing block back");
					return -1;
				}
				array_temp=malloc(2 * table_size *sizeof(int)); //array is doubled
				for(i=0;i<table_size ; i ++)
					array_temp[i]=array[i];
				for(i=0 ; i <  table_size ; i++ )
					array_temp[i+table_size]=array[i];
				
				table_size *=2;

				if(BF_AllocateBlock(header_info->fileDesc))
				{
					BF_PrintError("Error allocating block");
					return -1;
				}
				counter=BF_GetBlockCounter(header_info->fileDesc) -1;
				if( BF_ReadBlock(header_info->fileDesc,counter, &block_alloc) ) //newly allocated block
				{
					BF_PrintError("Error getting block");
					return -1;
				}
				array_temp[index+(table_size/2)]=counter;
						
				memcpy(block_evr,array_temp, (2*table_size * sizeof(int)) );
				if(BF_WriteBlock( header_info->fileDesc,1) ) // save changed hash table
				{
					BF_PrintError("Error writing block back");
					return -1;
				}
				BucketInfo2.local_depth=BucketInfo.local_depth;
				BucketInfo2.overflow=0;
				BucketInfo2.counter=0;
				BucketInfo2.previous=0;
				memcpy(block_alloc,&BucketInfo2,sizeof(bucket_info));
				if(BF_WriteBlock( header_info->fileDesc,counter) ) // saving newly allocated block-info
				{
					BF_PrintError("Error writing block back");
					return -1;
				}
				index2=hashing( header_info , record , table_size);	 //rehashing all records that were in bucket and also the new one
				if(!( write_in_overflow(header_info-> fileDesc ,array_temp[index2] , record )))
				{
					create_and_write_to_overflow(header_info->fileDesc , array_temp[index2] , record );//if storing wasn't possible 			
				}											//create overflow bucket ( possible if there was already
				for(i=0 ; i < count ; i ++ )								//an overflow bucket)
				{
					index2=hashing( header_info , rec_array[i] , table_size);	 //dinei ti thesi sto euretirio				
					if(!(write_in_overflow(header_info-> fileDesc ,array_temp[index2] , rec_array[i] )))
					{
						create_and_write_to_overflow(header_info->fileDesc , array_temp[index2] , rec_array[i] );	

					}									
				}
				free(array_temp);						
			}
			else //if doubling the size of the array wasn't going to distribute the records 
			{	//create overflow block and store record there
				create_and_write_to_overflow(header_info->fileDesc , array[index] , record );
			}
		 }
		else //more than one pointers to the bucket
		{		
			for(i=0 ; i<count ; i++) //check if after creating new bucket
			{			//records will be distributed to both buckets
					index2=hashing( header_info , rec_array[i] , table_size);
					if(index!=index2)
					{
						flag=1;
						break;
					}
			}
			if(flag==0)
			{
				index2=hashing( header_info , record , table_size);
				if(index!=index2)
					flag=1; 
			}
	
			if(flag==1) //if records will be distributed in both buckets
			{
				empty_bucket( header_info->fileDesc , array[index] ); //create new bucket and rehash the records
				if(BF_AllocateBlock(header_info->fileDesc))
				{
					BF_PrintError("Error allocating block");
					return -1;
				}
				counter=BF_GetBlockCounter(header_info->fileDesc) -1;
				if( BF_ReadBlock(header_info->fileDesc,counter, &block_alloc) ) 
				{
					BF_PrintError("Error getting block");
					return -1;
				}
				array[index]=counter;
				memcpy(block_evr,array,table_size*sizeof(int)); //saving hash_table
				if(BF_WriteBlock( header_info->fileDesc,1) ) 
				{
					BF_PrintError("Error writing block back");
					return -1;
				}
				BucketInfo2.local_depth=BucketInfo.local_depth; //saving new bucket
				BucketInfo2.overflow=0;
				BucketInfo2.counter=0;
				BucketInfo2.previous=0;
				memcpy(block_alloc,&BucketInfo2,sizeof(bucket_info));
				if(BF_WriteBlock( header_info->fileDesc,counter) ) 
				{
					BF_PrintError("Error writing block back");
					return -1;
				}
				index2=hashing( header_info , record , table_size);	 //rehash records
				if(!( write_in_overflow(header_info-> fileDesc ,array[index2] , record )))	
					create_and_write_to_overflow(header_info->fileDesc , array[index2] , record );
				for(i=0 ; i < count ; i ++ )
				{
					index2=hashing( header_info , rec_array[i] , table_size);
					if(!(write_in_overflow(header_info-> fileDesc ,array[index2] , rec_array[i] )))
						create_and_write_to_overflow(header_info->fileDesc , array[index2] , rec_array[i] );
				}		

			}
			else //if creating new bucket wouldn't distribute the records better, create overflow
			{
				create_and_write_to_overflow(header_info->fileDesc , array[index] , record );
			}



		}
	free(rec_array);
	} 
 	free(array);
	return 0 ;
}



void HT_GetAllEntries( HT_info* header_info,  void *value){
	int total_blocks,rpb,overflow=0,blocks_read=0,i=2,j=0,found=0, table_size=1,bucket;
	int * array=NULL;		
	void * block;
	clock_t begin, end;
	double time_spent;
	Record record;
	if((total_blocks=BF_GetBlockCounter(header_info->fileDesc))< 0)	/*posa blocks exei synolika to arxeio--xreiazetai gi atin seiriaki anazhthsh otan to value einai null*/
		BF_PrintError("HT_GetAllEntries// ");	
	
	printf("\n |  ID  |      NAME      |       SURNAME       |   STATUS   |   DATE OF BIRTH   |   SALARY   |  SECTION  |  DAYS OFF  |  PREVIOUS YEARS | \n");		
	if( value== NULL){ 			/*an value==NULL, ektypwnontai oles oi eggrafes tou arxeiou seiriaka*/
		begin = clock();											/*enarksi xronometrisis ektelesis entolwn*/
		for(;i<total_blocks;i++){				/*gia ola ta block eggrafwn (apo block 1 ews to teleytaio) */
			if(BF_ReadBlock(header_info->fileDesc,i,&block)< 0) 			/*pairnoume tin dieythinsi tou block*/
				BF_PrintError("HT_GetAllEntries3// ");
			memcpy(&rpb,block,sizeof(int));						/*diavazoume poses eggrafes exei to block*/
			memcpy(&overflow,block+(2*sizeof(int)),sizeof(int));			
			j=0;
			printf("EKTYPWNETAI TO BLOCK %d --SYNOLIKA %d EGGRAFES -- EXEI OVERFLOW TO BUCKET %d.\n",i,rpb,overflow);
			for(;j<rpb;j++){							/*gia oles tis eggrafes pou exei to block*/
				memcpy(&record,block+sizeof(bucket_info) +(j*sizeof(Record)),sizeof(Record));	/*diavazoume mia mia tin eggrafi*/
				printf(" |%6d|%16s|%21s|%12c|%19s|%12d|%11c|%12d|%17d|\n",record.id,record.name,record.surname,*record.status
						,record.dateOfBirth,record.salary,*record.section,record.daysOff,record.prevYears);/*ektypwnoume tin eggrafi*/		
				found++;
			}
			blocks_read++;
		}
		end = clock();												/*telos xronometrisis ektelesis entolwn*/
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;				/*evresi xronou ektelesis entolwn*/
	}
	else{
		/*vhma 1: kanw synarthsh katakermatismou gia timh ish me value
		  vhma 2: vriskw se poia thesi tou pinaka evrethriou me paei 
		  vhma 3: anazhtw se ayton ton pinaka seiriaka oles tiw eggrafes gia na dw an exw kapoia ish me ayto to value
		  vhma 4: elegxw ta overflow buckets-an exei kanw pali vhma 3 kai 4*/

/***leipei VHMA1+VHMA2 --> otan tha vrw tin thesi tou bucket tin apothikevw stin metavliti i gia na treksei o parakatw kwdikas*/		
		for(i=0 ; i<header_info->depth ; i++)
			table_size *=2;
		if( BF_ReadBlock(header_info->fileDesc,1, &block) ){			 //reading hash table block
			BF_PrintError("Error getting block");
		}
		array=malloc(table_size * sizeof(int));
		memcpy(array,block,(table_size * sizeof(int)));    //copying hash table from block to array
		if(header_info->attrType=='i'){			//choose key of hashing
			bucket=hash_keyInt(* (int *) value,table_size);
		}else{
			bucket=hash_keyChar(value,table_size);
		}
		begin = clock();
		bucket=array[bucket];
		do{					/*kanw tin epanalhpsh gia tosa buckets osa exw overflow*/
			if(BF_ReadBlock(header_info->fileDesc,bucket,&block)< 0) /*bucket einai to bucket pou tha anazhthsoume*/
				BF_PrintError("HT_GetAllEntries4// ");
			memcpy(&rpb,block,sizeof(int));				/*diavazoume poses eggrafes exei to block*/
			memcpy(&overflow,block+(2*sizeof(int)),sizeof(int));	/*diavazoume tin timi overflow*/
			j=0;
			for(;j<rpb;j++){				
				memcpy(&record,block+sizeof(bucket_info)+(j*sizeof(Record)),sizeof(Record));
				if( 	(!strcmp("id",header_info->attrName) && (record.id==* (int *) value)) /*elegxos an yparxei kapoia eggrafi symfwnh me ta krithria anazhthshs sto bucket poy ypedeikse to evretirio*/
					||	((!strcmp("name",header_info->attrName))&& (!strcmp(record.name,value)))
					||	((!strcmp("surname",header_info->attrName))&& (!strcmp(record.surname,value)))
					||	((!strcmp("status",header_info->attrName))&& (!strcmp(record.status,value)))
					||	((!strcmp("dateOfbirth",header_info->attrName))&& (!strcmp(record.dateOfBirth,value)))
					||	(!strcmp("salary",header_info->attrName) && (record.salary==* (int *) value)) 
					||	((!strcmp("section",header_info->attrName))&& (!strcmp(record.section,value)))
					||	(!strcmp("daysOff",header_info->attrName) && (record.daysOff==* (int *) value)) 	
					||	(!strcmp("prevYears",header_info->attrName) && (record.prevYears==* (int *) value))		)	{
						printf(" |%6d|%16s|%21s|%12c|%19s|%12d|%11c|%12d|%17d|\n",record.id,record.name,record.surname,*record.status
									,record.dateOfBirth,record.salary,*record.section,record.daysOff,record.prevYears);/*vrethike eggrafi-ektypwsh ths*/
						found++;										/*ayksisi metriti eggrafwn pou vrethikan me ayta ta krithria*/
					}
			}
			blocks_read++;
			if(overflow!=0)
				bucket=overflow;					/*pairnw tin timi tou epomenou block*/
		}while(overflow!=0);
		end = clock();
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		if(found==0)													/*den vrethike eggrafi me ta krithria anazhthshs*/
			printf("There are no records with this value. Please try again\n");
		
	}
	printf("\n\n					*******HASH/READ STATISTICS******\n");
	printf("					*                               *\n");
	printf("					* |  Blocks |   Time  | Found  |*\n");
	printf("					* |%9d|%9f|%8d|*\n",blocks_read, time_spent,found);
	printf("					*                               *\n");
	printf("					*********************************\n\n");
}








int hash_keyInt(int key,int buckets)   //hash funtion apo to link http://www.cs.hmc.edu/~geoff/classes/hmc.cs070.200101/homework10/hashfuncs.html
{
	unsigned int hash;
	hash = (key*(key+2)%buckets) ; //0-(buckets-1)
	return hash;
}

int hash_keyChar(char * key , int  buckets)     //hash funtion apo to link http://stackoverflow.com/questions/2351087/what-is-the-best-32bit-hash-function-for-short-strings-tag-names
{
	unsigned int h;
        unsigned char *p;

	h = 0;
	for (p = (unsigned char*)key;
		*p != '\0'; p++)
	h = 31 * h + *p;
	return h % buckets; // or, h % ARRAY_SIZE;

}

int hashing(HT_info *header_info , Record record , int table_size)
{
	int keyInt=0 , bucket ;
	char * keyChar;
	if(header_info->attrType=='i')		//choose key of hashing
	{
		if(!(strcmp(header_info->attrName , "id")))
		{
			keyInt=record.id;
		}
		else if(!(strcmp(header_info->attrName , "salary")))
		{
			keyInt=record.salary;
		}
		else if(!(strcmp(header_info->attrName , "daysOff")))
		{
			keyInt=record.daysOff;
		}
		else if(!(strcmp(header_info->attrName , "prevYears")))
		{
			keyInt=record.prevYears;
		}
		bucket=hash_keyInt(keyInt,table_size);
	}
	else    //edo tha kalei tin hash gia string
	{
		keyChar=malloc( (header_info->attrLength ) *sizeof(char));
		if(!(strcmp(header_info->attrName , "name")))
		{		
			strcpy(keyChar,record.name);
		}
		else if(!(strcmp(header_info->attrName , "surname")))
		{
			strcpy(keyChar,record.surname);
		}
		else if(!(strcmp(header_info->attrName , "status")))
		{
			strcpy(keyChar,record.status);
		}
		else if(!(strcmp(header_info->attrName , "dateOfBirth")))
		{
			strcpy(keyChar,record.dateOfBirth);
		}
		else if(!(strcmp(header_info->attrName , "section")))
		{
			strcpy(keyChar,record.section);
		}
		bucket=hash_keyChar(keyChar,table_size);
		free(keyChar);
	}
	return bucket;
}



 
int write_in_overflow(int fileDesc , int bucket , Record record ) //returns 0 if writting wasn't possible,else returns 1
{
	void *block ,*block2;
	int i ,j;
	bucket_info BucketInfo , BucketInfo2  ;
	if( BF_ReadBlock(fileDesc,bucket, &block) ) 
	{
		BF_PrintError("Error getting block");
		return -1;
	}	
	memcpy(&BucketInfo ,block , sizeof(bucket_info));
	if(( (BLOCK_SIZE -sizeof(bucket_info))/sizeof(Record) ) > BucketInfo.counter) //if record fits in bucket
	{
		memcpy(block+sizeof(bucket_info)+BucketInfo.counter*sizeof(Record),&record,sizeof(Record));
		BucketInfo.counter++;
		memcpy(block,&BucketInfo,sizeof(bucket_info));
		if(BF_WriteBlock( fileDesc,bucket) )
		{
			BF_PrintError("Error ty block back");
			return -1;
		}
		
		return 1;
	}
	else //check if bucket has any previous
	{
		
		i=BucketInfo.previous;
		if(i!=0)
		{
			while(1) //go to the first bucket ( if list of overflows and given bucket is not the first one) or the firts previous block which is not full
			{
				
				if( BF_ReadBlock(fileDesc,i, &block2) ) 
				{
					BF_PrintError("Error getting block");
					return  -1;
				}
				memcpy(&BucketInfo2,block2,sizeof(bucket_info));
				if(( (BLOCK_SIZE -sizeof(bucket_info))/sizeof(Record) ) > BucketInfo2.counter)  // den einai gemato to oveflow
				{
					//i=BucketInfo2.overflow;
					break;
				}					
				if((BucketInfo2.previous==0 ))  //to overflow den exei diko tou overflow
				{
					i=0;				
					break;
				}				
				
				i=BucketInfo2.previous;
			}
			
		

		}
		if(i!=0) //chech if writting in previous  is possible
		{
			if(( (BLOCK_SIZE -sizeof(bucket_info))/sizeof(Record) ) > BucketInfo2.counter) 
			{

				memcpy(block2+sizeof(bucket_info)+BucketInfo2.counter*sizeof(Record),&record,sizeof(Record));
				BucketInfo2.counter++;
				memcpy(block2,&BucketInfo2,sizeof(bucket_info));
				if(BF_WriteBlock( fileDesc,i) )
				{
					BF_PrintError("Error writing block back");
					return -1;
				}
				return 1;
			}

		}
		j=BucketInfo.overflow;
		if(j!=0)
		{
			while(1) //go to the last overflow or the firts overflow block which is not full
			{
				
				if( BF_ReadBlock(fileDesc,j, &block2) ) 
				{
					BF_PrintError("Error getting block");
					return  -1;
				}
				memcpy(&BucketInfo2,block2,sizeof(bucket_info));
				if(( (BLOCK_SIZE -sizeof(bucket_info))/sizeof(Record) ) > BucketInfo2.counter)  // den einai gemato to oveflow
				{
					//i=BucketInfo2.overflow;
					break;
				}					
				if((BucketInfo2.overflow==0 ))  //to overflow den exei diko tou overflow
				{
					j=0;				
					break;
				}				
				
				j=BucketInfo2.overflow;
			}
		}
		if(j!=0) //chech if writting in overflow is possible
		{
			if(( (BLOCK_SIZE -sizeof(bucket_info))/sizeof(Record) ) > BucketInfo2.counter) 
			{
				
				
				memcpy(block2+sizeof(bucket_info)+BucketInfo2.counter*sizeof(Record),&record,sizeof(Record));
				BucketInfo2.counter++;
				memcpy(block2,&BucketInfo2,sizeof(bucket_info));
				if(BF_WriteBlock( fileDesc,j) )
				{
					BF_PrintError("Error writing block back");
					return -1;
				}
				return 1;
			}
				return 0;

		}
	}
	return 0;
}




void total_records( int fileDesc , int bucket , Record *rec_array , int* size ) //returns the number of recors a bucket has ( including overflow buckets)
{
	void *block , *block2;
	bucket_info BucketInfo, BucketInfo2 ;	
	int i ;
	if( BF_ReadBlock(fileDesc,bucket, &block) ) 
	{
		BF_PrintError("Error getting block");
		return ;
	}
	memcpy(&BucketInfo ,block , sizeof(bucket_info));
	(*size) += BucketInfo.counter;
	i=BucketInfo.overflow;
	while(i!=0) //for each overflow
	{
		if( BF_ReadBlock(fileDesc,i, &block2) ) 
		{
			BF_PrintError("Error getting block");
			return ;
		}
		memcpy(&BucketInfo2 ,block2 , sizeof(bucket_info));
		
		(*size) += BucketInfo2.counter;
		i=BucketInfo2.overflow;
	}
	i=BucketInfo.previous;
	while(i!=0) //for each previous
	{
		if( BF_ReadBlock(fileDesc,i, &block2) ) 
		{
			BF_PrintError("Error getting block");
			return ;
		}
		memcpy(&BucketInfo2 ,block2 , sizeof(bucket_info));
		
		(*size) += BucketInfo2.counter;
		i=BucketInfo2.previous;
	}

}

void fill_array( int fileDesc , int bucket , Record *rec_array , int* size ) // takes a pointer to a record array and fills it with
{									//all the records( including overflow) from bucket
	void *block , *block2;
	int sum=0;
	bucket_info BucketInfo, BucketInfo2 ;	
	int i ;
	if( BF_ReadBlock(fileDesc,bucket, &block) ) 
	{
		BF_PrintError("Error getting block");
		return ;
	}
	memcpy(&BucketInfo ,block , sizeof(bucket_info));
	memcpy(rec_array, ( block+sizeof(bucket_info)) ,(BucketInfo.counter)*sizeof(Record));
	sum+=BucketInfo.counter;
	i=BucketInfo.overflow;//overflow
	while(i!=0)
	{
		if( BF_ReadBlock(fileDesc,i, &block2) ) 
		{
			BF_PrintError("Error getting block");
			return ;
		}
		memcpy(&BucketInfo2 ,block2 , sizeof(bucket_info));
		memcpy(rec_array+ sum ,block2 + sizeof(bucket_info) ,(BucketInfo2.counter*sizeof(Record)));
				
		sum += BucketInfo2.counter;
		i=BucketInfo2.overflow;
	}
	i=BucketInfo.previous;//previous
	while(i!=0)
	{
		if( BF_ReadBlock(fileDesc,i, &block2) ) 
		{
			BF_PrintError("Error getting block");
			return ;
		}
		memcpy(&BucketInfo2 ,block2 , sizeof(bucket_info));
		memcpy(rec_array+ sum ,block2 + sizeof(bucket_info) ,(BucketInfo2.counter*sizeof(Record)));
				
		sum += BucketInfo2.counter;
		i=BucketInfo2.previous;
	}
}



void empty_bucket(int fileDesc , int bucket )  //"empties" bucket (including overflow)
{						//what it really does is setting record counters to 0
	void *block , *block2;
	bucket_info BucketInfo, BucketInfo2 ;	
	int i ;
	if( BF_ReadBlock(fileDesc,bucket, &block) ) 
	{
		BF_PrintError("Error getting block");
		return ;
	}
	memcpy(&BucketInfo ,block , sizeof(bucket_info));
	BucketInfo.counter=0;
	memcpy(block,&BucketInfo,sizeof(bucket_info));
	if(BF_WriteBlock( fileDesc,bucket) )
	{
		BF_PrintError("Error writing block back");
		return ;
	}
	i=BucketInfo.overflow;
	while(i!=0)
	{
		
		if( BF_ReadBlock(fileDesc,i, &block2) ) 
		{
			BF_PrintError("Error getting block");
			return ;
		}
		memcpy(&BucketInfo2 ,block2 , sizeof(bucket_info));
		BucketInfo2.counter=0;
		memcpy(block2,&BucketInfo2,sizeof(bucket_info));
		if(BF_WriteBlock( fileDesc,i) )
		{
			BF_PrintError("Error writing block back");
			return ;
		}
		i=BucketInfo2.overflow;
	}
	i=BucketInfo.previous;//previous
	while(i!=0)
	{
		
		if( BF_ReadBlock(fileDesc,i, &block2) ) 
		{
			BF_PrintError("Error getting block");
			return ;
		}
		memcpy(&BucketInfo2 ,block2 , sizeof(bucket_info));
		BucketInfo2.counter=0;
		memcpy(block2,&BucketInfo2,sizeof(bucket_info));
		if(BF_WriteBlock( fileDesc,i) )
		{
			BF_PrintError("Error writing block back");
			return ;
		}
		i=BucketInfo2.previous;
	}

}


void create_and_write_to_overflow(int fileDesc , int bucket ,Record record ) //create an overflow to this bucket
{	
										//note that if there already is an overflow
	int counter,i,flag=0, previous ;							//the newly added overflow bucket will be stored in the end
	void * block_alloc , *block_alloc_new =NULL, *block_hash;
	bucket_info BucketInfo3 , BucketInfo2 ,BucketInfo ;
	previous=bucket;
	if( BF_ReadBlock(fileDesc,bucket, &block_hash) ) //diabazo to block 
	{
		BF_PrintError("Error getting block");
		return ;
	}
	memcpy(&BucketInfo,block_hash,sizeof(bucket_info));

	if(BF_AllocateBlock(fileDesc))
	{
		BF_PrintError("Error allocating block");
		return ;
	}

	counter=BF_GetBlockCounter(fileDesc) -1; 
	

	
	if( BF_ReadBlock(fileDesc,counter, &block_alloc) ) //diabazo to block p molis ekana allocate
	{
		BF_PrintError("Error getting block");
		return ;
	}

	i=BucketInfo.overflow;
	if(i!=0)
	{	flag=1;

		while(1)  //find last overflow
		{
			if( BF_ReadBlock(fileDesc,i, &block_alloc_new) ) //diabazo to block p molis ekana allocate
			{
				BF_PrintError("Error getting block");
				return ;
			}	
			memcpy(&BucketInfo3,block_alloc_new,sizeof(bucket_info));
			if((BucketInfo3.overflow==0))
				break;
			previous=i;
			i=BucketInfo3.overflow;
			
		}
	}
	if(flag==0) //first overflow in this bucket
	{

		BucketInfo.overflow=counter;
		BucketInfo2.overflow=0;	
		BucketInfo2.counter=1;
		BucketInfo2.previous=previous;
		BucketInfo2.local_depth=BucketInfo.local_depth;	
		memcpy(block_hash,&BucketInfo,sizeof(bucket_info));
		memcpy(block_alloc,&BucketInfo2,sizeof(bucket_info));
		memcpy(block_alloc+sizeof(bucket_info),&record, sizeof(Record));
		if(BF_WriteBlock( fileDesc,counter) ) 
		{
			BF_PrintError("Error writing block back");
			return ;
		}
		if(BF_WriteBlock( fileDesc,bucket) ) 
		{
			BF_PrintError("Error writing block back");
			return ;
		}
		
	}
	else //one or more overflow blocks exist.Create new overflow and set it to be the last one
	{	
		BucketInfo3.overflow=counter;
		BucketInfo2.overflow=0;	
		BucketInfo2.counter=1;
		BucketInfo2.previous=previous;
		BucketInfo2.local_depth=BucketInfo.local_depth;	
		memcpy(block_alloc_new,&BucketInfo3,sizeof(bucket_info));
		memcpy(block_alloc,&BucketInfo2,sizeof(bucket_info));
		memcpy(block_alloc+sizeof(bucket_info),&record, sizeof(Record));
		if(BF_WriteBlock( fileDesc,counter) ) 
		{
			BF_PrintError("Error writing block back");
			return ;
		}
		if(BF_WriteBlock( fileDesc,i) ) 
		{
			BF_PrintError("Error writing block back");
			return ;
		}
				
	}
			
	
}



