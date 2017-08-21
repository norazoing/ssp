/*
 ============================================================================
 Name        : HelloWorld.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void INT2ASCWithFillLeft0(int nSrcINT,  unsigned char *abDesASC,  unsigned char bLengthDes)
{
	 unsigned char abBuf1[64];
	 unsigned char abBuf2[256];



	sprintf(abBuf1, "%%0%ud", bLengthDes);
	sprintf(abBuf2, abBuf1, nSrcINT);
	memcpy(abDesASC, abBuf2, bLengthDes);
}


void INT2ASCWithSize(int nSrcINT,  char *abDesASC, int *bLengthDes)
{
	char abBuf[256];


	printf("Start\n");

	sprintf(abBuf, "%d", nSrcINT);
	*bLengthDes = strlen(abBuf);
	memcpy(abDesASC, abBuf, (int)*bLengthDes);

	printf("End\n");
}



typedef struct buffer
{
	char CarNo[4];
	char dot;
	char DriveTime[10];
	char cr[2];
} BUFFER;



void *thread_func(void *arg);

void* do_loop(void *data)
{
    int i;

    int me = *((int *)data);
    for (i = 0; i < 10; i++)
    {
        printf("%d - Got %d\n", me, i);
        sleep(1);
    }
}

int main()
{
    int       thr_id;
    pthread_t p_thread[3];
    int status;
    int a = 1;
    int b = 2;
    int c = 3;

    thr_id = pthread_create(&p_thread[0], NULL, thread_func, (void *)&a);
    printf("thread id %d\n", thr_id);
    thr_id = pthread_create(&p_thread[1], NULL, do_loop, (void *)&b);
    printf("thread id %d\n", thr_id);
    thr_id = pthread_create(&p_thread[2], NULL, do_loop, (void *)&c);
    printf("thread id %d\n", thr_id);


    pthread_join(p_thread[0], (void **) &status);
    pthread_join(p_thread[1], (void **) &status);
    pthread_join(p_thread[2], (void **) &status);

    printf("programing is end\n");
    return 0;
}



void *thread_func(void *arg)
{
	FILE *ptr_file, *out_file, *out_eachfile;;
	BUFFER stTempBuf;
	int nCarNo = 0;
	int nDriveTime = 0;
	int i = 0;
	int nReadSize;
	unsigned char ucDriveTime[11];

	double abDriveTime[6] = {0x00, };
    int abCarNoCnt[6] = {0x00, };
    double dTimeAverage = 0;

    unsigned char abPath[50] = {0x00, };

	ptr_file =fopen(".//src//input.txt", "r");

	if (!ptr_file)
	{
		printf("File Not Found\n");
		return 1;
	}

	out_file =fopen(".//src//output.txt", "w");
	if (!out_file)
	{
		printf("File Not Found\n");
		return 1;
	}



	//unsigned char size = 0;
	//printf("INT2ASCWithSize Start\n");
	//INT2ASCWithSize(i, str2, (unsigned char*)&size);


	while(1)
	{

		nReadSize = fread(&stTempBuf,sizeof(stTempBuf),1,ptr_file);
		if ( nReadSize != 1 )
			break;

		nCarNo = atoi(stTempBuf.CarNo);
		nDriveTime = atoi(stTempBuf.DriveTime);

		abDriveTime[nCarNo] += nDriveTime;
        abCarNoCnt[nCarNo] += 1;

    	sprintf(abPath, ".//src//output_%02d.txt", nCarNo );
     	out_eachfile =fopen( abPath , "a");
    	if (!out_eachfile)
    	{
    		printf("File Not Found\n");
    		return 1;
    	}
		//printf("nCarNo =  %d, nDriveTime = %d\n", nCarNo, nDriveTime);
		//printf("CarNo[%d] = Average Time %0lf\n", i, dTimeAverage);
		memcpy( ucDriveTime, stTempBuf.DriveTime, sizeof(stTempBuf.DriveTime));
		ucDriveTime[10] = '\0';

		fprintf(out_eachfile, "%c%c%c%c%c%s%c%c", stTempBuf.CarNo[0], stTempBuf.CarNo[1] , stTempBuf.CarNo[2],
				stTempBuf.CarNo[3], '.', ucDriveTime, stTempBuf.cr[0],  stTempBuf.cr[1]);

		//nReadSize = fwrite(&stTempBuf,sizeof(stTempBuf),1,out_eachfile);
		//if ( nReadSize != 1 )
		//	break;

		//INT2ASCWithFillLeft0(x, str, 4);
		//printf("str = %x %x %x %x", str[0], str[1], str[2], str[3]);

	}

	for ( i = 0 ; i <= 5 ; i++ )
	{
		dTimeAverage = (double) ( abDriveTime[i] / abCarNoCnt[i] );
		fprintf(out_file, "%05d%c%0lf\n", i, '.', dTimeAverage);
	}


	fclose(out_eachfile);

	fclose(out_file);

	fclose(ptr_file);

	return  0;
}

/*
int main()
{
	FILE *ptr_file, *out_file, *out_eachfile;;
	BUFFER stTempBuf;
	int nCarNo = 0;
	int nDriveTime = 0;
	int i = 0;
	int nReadSize;
	unsigned char ucDriveTime[11];

	double abDriveTime[6] = {0x00, };
    int abCarNoCnt[6] = {0x00, };
    double dTimeAverage = 0;

    unsigned char abPath[50] = {0x00, };

	ptr_file =fopen(".//src//input.txt", "r");

	if (!ptr_file)
	{
		printf("File Not Found\n");
		return 1;
	}

	out_file =fopen(".//src//output.txt", "w");
	if (!out_file)
	{
		printf("File Not Found\n");
		return 1;
	}



	//unsigned char size = 0;
	//printf("INT2ASCWithSize Start\n");
	//INT2ASCWithSize(i, str2, (unsigned char*)&size);




	while(1)
	{

		nReadSize = fread(&stTempBuf,sizeof(stTempBuf),1,ptr_file);
		if ( nReadSize != 1 )
			break;

		nCarNo = atoi(stTempBuf.CarNo);
		nDriveTime = atoi(stTempBuf.DriveTime);

		abDriveTime[nCarNo] += nDriveTime;
        abCarNoCnt[nCarNo] += 1;

    	sprintf(abPath, ".//src//output_%02d.txt", nCarNo );
     	out_eachfile =fopen( abPath , "a");
    	if (!out_eachfile)
    	{
    		printf("File Not Found\n");
    		return 1;
    	}
		//printf("nCarNo =  %d, nDriveTime = %d\n", nCarNo, nDriveTime);
		//printf("CarNo[%d] = Average Time %0lf\n", i, dTimeAverage);
		memcpy( ucDriveTime, stTempBuf.DriveTime, sizeof(stTempBuf.DriveTime));
		ucDriveTime[10] = '\0';

		fprintf(out_eachfile, "%c%c%c%c%c%s%c%c", stTempBuf.CarNo[0], stTempBuf.CarNo[1] , stTempBuf.CarNo[2],
				stTempBuf.CarNo[3], '.', ucDriveTime, stTempBuf.cr[0],  stTempBuf.cr[1]);

		//nReadSize = fwrite(&stTempBuf,sizeof(stTempBuf),1,out_eachfile);
		//if ( nReadSize != 1 )
		//	break;

		//INT2ASCWithFillLeft0(x, str, 4);
		//printf("str = %x %x %x %x", str[0], str[1], str[2], str[3]);

	}

	for ( i = 0 ; i <= 5 ; i++ )
	{
		dTimeAverage = (double) ( abDriveTime[i] / abCarNoCnt[i] );
		fprintf(out_file, "%05d%c%0lf\n", i, '.', dTimeAverage);
	}


	fclose(out_eachfile);

	fclose(out_file);

	fclose(ptr_file);

	return  0;
}
*/

/*
int main()
{
	int i = 15;
	char abDesASC[4];
	int bLengthDes;
	char abBuf[256];


	INT2ASCWithSize(i, abDesASC, &bLengthDes );
	printf("str 2= %d %x %x %x %x", bLengthDes, abDesASC[0], abDesASC[1], abDesASC[2], abDesASC[3]);


	sprintf(abBuf, "%d", i );
	bLengthDes = strlen(abBuf);
	memcpy(abDesASC, abBuf, (int)bLengthDes);

	printf("str 2= %d %x %x %x %x", bLengthDes, abDesASC[0], abDesASC[1], abDesASC[2], abDesASC[3]);
	return 0;
}
*/

