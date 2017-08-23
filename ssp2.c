/* 차량번호와 운행시간을 구성된 운행내역 로그파일 형식 input01.dat

차량번호4자리@운행시간10자리
예 > 0015@0000019300
(단, 차량번호는 0000 ~ 0019 로 구성된)

1. 차량별 운행시간의 평균을 구하여 운행시간 평균의 내림차순으로 다음과 같이 output01.dat 파일에 기록
차량번호4자리,운행시간평균(소수점4자리)    예시 > 0015,50066.9897

2. 차량번호_log01.dat 파일에 각 차량별 운행내역 로그를 별도로 기록
3. 차량번호_log02.dat 파일에 차량번호를 conv.exe 프로그램을 통해 변환하여 각 운행내역 로그를 작성
4. 차량번호_log03.dat 파일에 차량번호 변환 ㅁ치 각 운행내역 로그기록을 thread로 수행
5. 차량번호_log04.dat 파일에 차량번호 변환 및 각 운행내역 로그기록 thread로 수행하도록 변경. 스레드 갯수 10개 제한
1번 형식으로 output02.dat 파일에 차량별 운행시간의 평균을 구하여 정렬없이 기록하되, 차량별로 thread를 이용하여 운행시간 평균 구하기
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define BUFF_SIZE 1024
typedef struct buffer
{
	char CarNo[4];
	char dot;
	char DriveTime[10];
	char cr[2];
} BUFFER;

typedef struct outbuffer
{
	int CarNo;
	double abAverageDriveTime;
} OUTBUFFER;

void INT2ASCWithFillLeft0(int nSrcINT,  unsigned char *abDesASC,  unsigned char bLengthDes);
void INT2ASCWithSize(int nSrcINT,  char *abDesASC, int *bLengthDes);
int compare_with_size( const void *a, const void *b);
void cleanup( void *arg);
int ExeConv( char *CarNo, char* OutputCarNo);
int *LogWriteCarNo(int ReqCarNo);
void *LogWriteCarNoThreadFunc(void *arg);
int LogWriteCarNoConv(int *ReqCarNo);
void *LogWriteCarNoConvThreadFunc(void *arg);
void *CalcAvgTime(void *arg);

// 뮤텍스 객체 선언
pthread_mutex_t mutex_lock;
static int gthread_running_cnt = 0;


int main()
{
	FILE *ptr_file, *out_file;
	BUFFER stTempBuf;
	OUTBUFFER stOutPutBuf[20];

	int nCarNo = 0;
	int nDriveTime = 0;
	int i = 0;
	int nReadSize;

	double abAverageDriveTime[20] = {0x00, };
    int abCarNoCnt[20] = {0x00, };

	ptr_file = fopen(".//src//input01.dat", "rb");

	if (!ptr_file)
	{
		printf("File Not Found\n");
		return 1;
	}

	out_file =fopen(".//src//output.dat", "wb");
	if (!out_file)
	{
		printf("File Not Found\n");
		return 1;
	}

	while(1)
	{
		nReadSize = fread(&stTempBuf,sizeof(stTempBuf),1,ptr_file);
		if ( nReadSize != 1 )
			break;

		nCarNo = atoi(stTempBuf.CarNo);
		nDriveTime = atoi(stTempBuf.DriveTime);

		abAverageDriveTime[nCarNo] += nDriveTime;
		abCarNoCnt[nCarNo] += 1;
	}

	for ( i = 0 ; i <= 20 ; i++ )
	{
		stOutPutBuf[i].abAverageDriveTime = (double)  ( abAverageDriveTime[i] / abCarNoCnt[i]);
		stOutPutBuf[i].CarNo = i;
	}

	qsort(stOutPutBuf, 20, sizeof(OUTBUFFER), compare_with_size);

	for ( i = 0 ; i <= 20 ; i++ )
	{
		fprintf(out_file, "%04d%c%09.4f\n", i, ',', stOutPutBuf[i].abAverageDriveTime);
	}

	fclose(out_file);

	fclose(ptr_file);

	return  0;
}

int main2()
{
	int i = 0;

	for ( i=0 ; i < 20 ; i++ )
		LogWriteCarNo(i);

	return 1;
}

int main3()
{
	int i = 0;
	for ( i = 0 ; i < 20 ; i++ )
		LogWriteCarNoConv( &i );

	return 1;
}

int main4()
{
	int thr_id;
	pthread_t p_thread[20];
	int status[20];
	int i = 0;

	// 뮤텍스 객체 초기화
	pthread_mutex_init( &mutex_lock, NULL );

	for ( i = 0 ; i < 20 ; i++ )
	{
		printf("thread id %d create start\n", i);
		thr_id = pthread_create( &p_thread[i], NULL, LogWriteCarNoThreadFunc, (void *)&i);
		printf("thread id %d : %s \n", i , (thr_id == 0 ? "success" : "fail"));
	}

	for ( i = 0 ; i < 20 ; i++)
	{
		pthread_join(p_thread[i], (void **) &status[i]);
	}

	printf("programming is end\n");
	return 0;
}

int main5()
{
	int thr_id;
	pthread_t p_thread[3];
	int status;

	int thread_exe_cnt = 0;

	// 뮤텍스 객체 초기화
	pthread_mutex_init( &mutex_lock, NULL );

	while (1)
	{
		if ( thread_exe_cnt == 3 )
			break;

		if ( gthread_running_cnt == 1 )
			continue;

		thr_id = pthread_create( &p_thread[thread_exe_cnt], NULL, LogWriteCarNoConvThreadFunc, (void *)&thread_exe_cnt);
		if ( thr_id == 0 )
		{
			thread_exe_cnt++;
			gthread_running_cnt++;
			printf("exe cnt : %d, running thread cnt : %d \n", thread_exe_cnt, gthread_running_cnt);
			pthread_join( p_thread[thread_exe_cnt], (void **)&status );
		}
	}

	printf("programming is end\n");
	return 0;
}

int main6()
{
	FILE *out_file;
	BUFFER stTempBuf;
	OUTBUFFER stOutPutBuf[20];
	int nCarNo = 0;
	int nDriveTime = 0;
	int i = 0;
	int nReadSize = 0;;

	double abAverageDriveTime = 0;

	int thr_id;
	pthread_t p_thread[20];
	int status;
	void *tret = NULL;


	for ( i = 0 ; i < 20 ; i++ )
	{
		printf("thread id %d create start\n", i);
		thr_id = pthread_create( &p_thread[i], NULL, CalcAvgTime, (void *)&i);
		if ( thr_id < 0 )
		{
			perror("thread create error: ");
			exit(0);
		}
		pthread_join(p_thread[i], &tret);

		printf("thread exit code %lf\n", *((double*)tret));
		abAverageDriveTime = * ((double*)tret);

		fprintf(out_file, "%04d%c%09.4f\n", i, ',', abAverageDriveTime);

	}

	fclose(out_file);

	return  0;

}
int compare_with_size( const void *a, const void *b)
{
	OUTBUFFER* ptr_a = (OUTBUFFER*)a;
	OUTBUFFER* ptr_b = (OUTBUFFER*)b;

	// 내림차순
	if ( ptr_a->abAverageDriveTime < ptr_b->abAverageDriveTime ) return -1;
	else if ( ptr_a->abAverageDriveTime == ptr_b->abAverageDriveTime ) return 0;
	else return -1;

}

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

void *CalcAvgTime( void *arg)
{
	FILE *ptr_file;
	BUFFER stTempBuf;
	int nCarNo = 0;
	int nDriveTime = 0;

	int nReadSize = 0;

	double nTotalDriveTime = 0;
	int nCarNoCnt = 0;
	static double dRetVal = 0.0;

	ptr_file =fopen(".//src//input01.dat", "rb");

	if (!ptr_file)
	{
		printf("File Not Found\n");
		return 1;
	}


	while(1)
	{
		nReadSize = fread(&stTempBuf,sizeof(stTempBuf),1,ptr_file);
		if ( nReadSize != 1 )
			break;

		nCarNo = atoi(stTempBuf.CarNo);
		nDriveTime = atoi(stTempBuf.DriveTime);

		if ( nCarNo == (*(int*)arg))
		{
			nTotalDriveTime += nDriveTime;
			nCarNoCnt += 1;
		}

	}

    dRetVal = (double) ( nTotalDriveTime / nCarNoCnt );


	fclose(ptr_file);

	pthread_exit((void*)&dRetVal);

	return  0;

}

void cleanup( void *arg)
{
	gthread_running_cnt--;

}
void *LogWriteCarNoConvThreadFunc(void *arg)
{
	FILE *ptr_file, *out_eachfile;
	BUFFER stTempBuf;

	int nCarNo = 0;
	int nDriveTime = 0;
	int i = 0;
	int nReadSize = 0;
	unsigned char ucDriveTime[11] = {0x00, };
	unsigned char ucOutPutCarNo[20] = {0x00, };
	unsigned char abPath[50] =  {0x00, };
	int retval = -1;

	pid_t pid;
	pthread_t tid;

	pid = getpid();
	tid = pthread_self();

	pthread_cleanup_push( cleanup, (void*)retval );

	ptr_file =fopen(".//src//input01.dat", "rb");

	if (!ptr_file)
	{
		printf("File Not Found\n");
		pthread_exit((void*)&retval);
	}

	while(1)
	{
		nReadSize = fread(&stTempBuf,sizeof(stTempBuf),1,ptr_file);
		if ( nReadSize != 1 )
			break;

		nCarNo = atoi(stTempBuf.CarNo);
		nDriveTime = atoi(stTempBuf.DriveTime);

		if ( nCarNo == (*(int *)arg))
		{
			sprintf(abPath, ".//04d_log03.dat", nCarNo);

			out_eachfile =fopen(abPath, "ab");
			if (!out_eachfile)
			{
				printf("File Not Found\n");
				pthread_exit((void*)&retval);
			}

			ExeConv( stTempBuf.CarNo, ucOutPutCarNo);

			memcpy( ucDriveTime, stTempBuf.DriveTime, sizeof(stTempBuf.DriveTime));
			ucDriveTime[10] = '\0';

			fprintf( out_eachfile, "%s%c%s%c", ucOutPutCarNo, ',', ucDriveTime, stTempBuf.cr[0]);
		}
	}

	fclose(out_eachfile);

	fclose(ptr_file);
	pthread_exit((void*)&retval);
	pthread_cleanup_pop(0);

	return  0;
}

int ExeConv( char *CarNo, char* OutPutCarNo)
{
	char buff[BUFF_SIZE];
	FILE *fp;

	char command[30] = {0x00, };
	char InputCarNo[5] = {0x00, };

	memcpy( InputCarNo, CarNo, 4);

	pthread_mutex_lock( &mutex_lock );

	sprintf( command, ".//conv %s", InputCarNo );
	fp = popen( command, "r");
	if ( fp == NULL )
	{
		perror("popen() 실패 ");
		return -1;
	}

	while ( fgets(buff, BUFF_SIZE, fp))
	{
		sprintf( OutPutCarNo, "%s", buff);
	}
	pclose( fp );
	pthread_mutex_unlock( &mutex_lock );

	return 0;

}


int *LogWriteCarNo(int ReqCarNo)
{
	FILE *ptr_file, *out_eachfile;
	BUFFER stTempBuf;

	int nCarNo = 0;
	int nDriveTime = 0;

	int nReadSize = 0;
	unsigned char ucDriveTime[11] = {0x00, };
	unsigned char ucOutPutCarNo[20] = {0x00, };
	unsigned char abPath[50] =  {0x00, };
	int retval = -1;


	ptr_file =fopen(".//src//input01.dat", "rb");
	if (!ptr_file)
	{
		printf("File Not Found\n");
		pthread_exit((void*)&retval);
	}

	while(1)
	{
		nReadSize = fread(&stTempBuf,sizeof(stTempBuf),1,ptr_file);
		if ( nReadSize != 1 )
			break;

		nCarNo = atoi(stTempBuf.CarNo);
		nDriveTime = atoi(stTempBuf.DriveTime);

		if ( nCarNo == ReqCarNo )
		{
			sprintf(abPath, ".//04d_log01.dat", nCarNo);

			out_eachfile =fopen(abPath, "ab");
			if (!out_eachfile)
			{
				printf("File Not Found\n");
				pthread_exit((void*)&retval);
			}

			memcpy( ucDriveTime, stTempBuf.DriveTime, sizeof(stTempBuf.DriveTime));
			ucDriveTime[10] = '\0';

			fprintf( out_eachfile, "%s%c%s%c", ucOutPutCarNo, ',', ucDriveTime, stTempBuf.cr[0]);
		}
	}

	fclose(out_eachfile);

	fclose(ptr_file);

	return  0;
}

int LogWriteCarNoConv(int *ReqCarNo)
{
	FILE *ptr_file, *out_eachfile;
	BUFFER stTempBuf;

	int nCarNo = 0;
	int nDriveTime = 0;
	int i = 0;
	int nReadSize = 0;
	unsigned char ucDriveTime[11] = {0x00, };
	unsigned char ucOutPutCarNo[20] = {0x00, };
	unsigned char abPath[50] =  {0x00, };


	ptr_file =fopen(".//src//input01.dat", "rb");
	if (!ptr_file)
	{
		printf("File Not Found\n");
		return 1;
	}

	while(1)
	{
		nReadSize = fread(&stTempBuf,sizeof(stTempBuf),1,ptr_file);
		if ( nReadSize != 1 )
			break;

		nCarNo = atoi(stTempBuf.CarNo);
		nDriveTime = atoi(stTempBuf.DriveTime);

		if ( nCarNo == *ReqCarNo )
		{
			sprintf(abPath, ".//04d_log03.dat", nCarNo);

			out_eachfile =fopen(abPath, "ab");
			if (!out_eachfile)
			{
				printf("File Not Found\n");
				return 1;
			}

			ExeConv( stTempBuf.CarNo, ucOutPutCarNo);

			memcpy( ucDriveTime, stTempBuf.DriveTime, sizeof(stTempBuf.DriveTime));
			ucDriveTime[10] = '\0';

			fprintf( out_eachfile, "%s%c%s%c", ucOutPutCarNo, ',', ucDriveTime, stTempBuf.cr[0]);
		}
	}

	fclose(out_eachfile);

	fclose(ptr_file);
	return  0;
}

void *LogWriteCarNoThreadFunc(void *arg)
{
	FILE *ptr_file, *out_eachfile;
	BUFFER stTempBuf;

	int nCarNo = 0;
	int nDriveTime = 0;
	int i = 0;
	int nReadSize = 0;
	unsigned char ucDriveTime[11] = {0x00, };
	unsigned char ucCarNo[5] = {0x00, };
	unsigned char abPath[50] =  {0x00, };


	ptr_file =fopen(".//src//input01.dat", "rb");
	if (!ptr_file)
	{
		printf("File Not Found\n");
		return 1;
	}

	while(1)
	{
		nReadSize = fread(&stTempBuf,sizeof(stTempBuf),1,ptr_file);
		if ( nReadSize != 1 )
			break;

		nCarNo = atoi(stTempBuf.CarNo);
		nDriveTime = atoi(stTempBuf.DriveTime);

		if ( nCarNo == (*(int*)arg) )
		{
			sprintf(abPath, ".//04d_log01.dat", nCarNo);

			out_eachfile =fopen(abPath, "ab");
			if (!out_eachfile)
			{
				printf("File Not Found\n");
				return 1;
			}

			memcpy( ucCarNo, stTempBuf.CarNo, sizeof(stTempBuf.CarNo));
			ucCarNo[5] = '\0';

			memcpy( ucDriveTime, stTempBuf.DriveTime, sizeof(stTempBuf.DriveTime));
			ucDriveTime[10] = '\0';

			fprintf( out_eachfile, "%s%c%s%c", ucCarNo, ',', ucDriveTime, stTempBuf.cr[0]);
		}
	}

	fclose(out_eachfile);

	fclose(ptr_file);
	return  0;

}


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


/*
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
	int nReadSize = 0;
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


