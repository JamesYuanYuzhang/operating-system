//#include <cstdio>
//#include <cstring>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
//#include <io.h>
//#include <Windows.h> //Sleep¼ÆÁ¿µ¥Î»ÎªÎ¢Ãî linuxÖÐusleepÒ²ÊÇÒÔÎ¢Ãî¼Æ£¬include<system.h>¼´¿É

using namespace std;
#define MAX 100
enum direction {South, East, North, West};
struct Car
{
	direction dir;
	int id;
};

//Í¨¹ýÂ·¿ÚµÄ×ÊÔ´
pthread_mutex_t source_south;//South
pthread_mutex_t source_east;//East
pthread_mutex_t source_north;//North
pthread_mutex_t source_west;//West

pthread_mutex_t printlock;//Êä³öËø
//ÏÂ´ÎÍ¨ÐÐ³µÁ¾µÄÌõ¼þ±äÁ¿
pthread_cond_t firstSouth;
pthread_cond_t firstEast;
pthread_cond_t firstNorth;
pthread_cond_t firstWest;

//ËÀËø
pthread_cond_t deadlock;
pthread_mutex_t wait_deadlock;
//³µÁ¾¶ÓÁÐÌõ¼þ±äÁ¿
pthread_cond_t queueSouth;
pthread_cond_t queueEast;
pthread_cond_t queueNorth;
pthread_cond_t queueWest;
//³µÁ¾ÕýÔÚÍ¨ÐÐ»¥³â±äÁ¿
pthread_mutex_t  crossingSouth;
pthread_mutex_t  crossingEast;
pthread_mutex_t  crossingNorth;
pthread_mutex_t  crossingWest;

int count_South = 0;
int count_East = 0;
int count_North = 0;
int count_West = 0;
int total = 0;

bool waiting_South = false;
bool waiting_East  = false;
bool waiting_North = false;
bool waiting_West  = false;

direction lastdir;

void * check_deadlock(void* c)
{
	while(true)
	{
		pthread_mutex_lock(&wait_deadlock);
		pthread_cond_wait(&deadlock,&wait_deadlock);
		if(count_South>0 and count_East>0 and count_North>0 and count_West>0)
		{
			switch(lastdir)
			{
				case West:
				{
					pthread_mutex_lock(&printlock);
					printf("DEADLOCK: car jam detected, signalling West to go\n");
					pthread_mutex_unlock(&printlock);
					pthread_cond_signal(&firstWest);
					break;
				}
				case North:
				{
					pthread_mutex_lock(&printlock);
					printf("DEADLOCK: car jam detected, signalling North to go\n");
					pthread_mutex_unlock(&printlock);
					pthread_cond_signal(&firstNorth);
					break;
				}
				case South:
				{
					pthread_mutex_lock(&printlock);
					printf("DEADLOCK: car jam detected, signalling South to go\n");
					pthread_mutex_unlock(&printlock);
					pthread_cond_signal(&firstSouth);
					break;
				}
				case East:
				{
					pthread_mutex_lock(&printlock);
					printf("DEADLOCK: car jam detected, signalling East to go\n");
					pthread_mutex_unlock(&printlock);
					pthread_cond_signal(&firstEast);
					break;
				}
			}
		}
		pthread_mutex_unlock(&wait_deadlock);
	}
}

void *car_from_south(void *c)
{
	pthread_mutex_lock(&crossingSouth);
	Car *car = (Car*)c;//Ç¿ÖÆÀàÐÍ×ª»»
	//car->dir = South;
	//pthread_mutex_lock(&crossingSouth);
	//count_South += 1;
	
	pthread_cond_wait(&queueSouth, &crossingSouth);
	//pthread_mutex_unlock(&crossingSouth);//¿ªÊ¼×¼±¸Í¨ÐÐ
	pthread_mutex_lock(&printlock);
	printf("car %d from south arrives crossing\n", car->id);
	pthread_mutex_unlock(&printlock);
	count_South += 1;
	usleep(1000);
	pthread_mutex_unlock(&crossingSouth);//¿ªÊ¼×¼±¸Í¨ÐÐ
	pthread_mutex_lock(&source_south);//Õ¼¾Ý×ÊÔ´
	if (count_East > 0)//Èç¹ûÓÒ±ßÓÐ³µ
	{
		lastdir=South;
		pthread_cond_signal(&deadlock);
		pthread_cond_wait(&firstSouth, &source_south);
	}
	count_South--;
	pthread_mutex_lock(&printlock);
	printf("car %d from south leaving crossing\n", car->id);
	pthread_mutex_unlock(&printlock);
	pthread_mutex_unlock(&source_south);
	pthread_cond_signal(&firstWest);//·¢ËÍÐÅºÅ¸ø×ó²à³µ·ÀÖ¹¼¢¶ö
	pthread_cond_signal(&queueSouth);//·¢¸ø±¾·½ÏòÏÂÒ»Á¾³µ×¼±¸Í¨ÐÐ
	pthread_exit(NULL);
}

void *car_from_east(void *c)
{
	pthread_mutex_lock(&crossingEast);
	Car *car = (Car*)c;//Ç¿ÖÆÀàÐÍ×ª»»
	//car->dir = East;
	//pthread_mutex_lock(&crossingEast);
	//count_East += 1;
	
	pthread_cond_wait(&queueEast, &crossingEast);
	//pthread_mutex_unlock(&crossingEast);//¿ªÊ¼×¼±¸Í¨ÐÐ
	pthread_mutex_lock(&printlock);
	printf("car %d from east arrives crossing\n", car->id);
	pthread_mutex_unlock(&printlock);
	count_East += 1;
	usleep(1000);
	pthread_mutex_unlock(&crossingEast);//¿ªÊ¼×¼±¸Í¨ÐÐ
	pthread_mutex_lock(&source_east);//Õ¼¾Ý×ÊÔ´
	bool deadlock_flag = false;//ËÀËø±êÊ¶Î»
	if (count_North > 0)//Èç¹ûÓÒ±ßÓÐ³µ
	{
		lastdir=East;
		pthread_cond_signal(&deadlock);
		pthread_cond_wait(&firstEast, &source_east);
	}
	waiting_East = false;
	count_East--;
	pthread_mutex_lock(&printlock);
	printf("car %d from east leaving crossing\n", car->id);
	pthread_mutex_unlock(&printlock);
	pthread_mutex_unlock(&source_east);
	pthread_cond_signal(&firstSouth);//·¢ËÍÐÅºÅ¸ø×ó²à³µ·ÀÖ¹¼¢¶ö
	pthread_cond_signal(&queueEast);//·¢¸ø±¾·½ÏòÏÂÒ»Á¾³µ×¼±¸Í¨ÐÐ
	pthread_exit(NULL);
}

void *car_from_north(void *c)
{
	pthread_mutex_lock(&crossingNorth);
	Car *car = (Car*)c;//Ç¿ÖÆÀàÐÍ×ª»»
	//car->dir = North;
	//pthread_mutex_lock(&crossingNorth);
	//count_North += 1;
	pthread_cond_wait(&queueNorth, &crossingNorth);
	//pthread_mutex_unlock(&crossingNorth);//¿ªÊ¼×¼±¸Í¨ÐÐ
	pthread_mutex_lock(&printlock);
	printf("car %d from north arrives crossing\n", car->id);
	pthread_mutex_unlock(&printlock);
	count_North += 1;
	usleep(1000);
	pthread_mutex_unlock(&crossingNorth);//¿ªÊ¼×¼±¸Í¨ÐÐ
	pthread_mutex_lock(&source_north);//Õ¼¾Ý×ÊÔ´
	bool deadlock_flag = false;//ËÀËø±êÊ¶Î»
	if (count_West > 0)//Èç¹ûÓÒ±ßÓÐ³µ
	{
		lastdir=North;
		pthread_cond_signal(&deadlock);
		pthread_cond_wait(&firstNorth, &source_north);
	}
	waiting_North = false;
	count_North--;
	pthread_mutex_lock(&printlock);
	printf("car %d from north leaving crossing\n", car->id);
	pthread_mutex_unlock(&printlock);
	pthread_mutex_unlock(&source_north);
	pthread_cond_signal(&firstEast);//·¢ËÍÐÅºÅ¸ø×ó²à³µ·ÀÖ¹¼¢¶ö
	pthread_cond_signal(&queueNorth);//·¢¸ø±¾·½ÏòÏÂÒ»Á¾³µ×¼±¸Í¨ÐÐ
	pthread_exit(NULL);
}

void *car_from_west(void *c)
{
	pthread_mutex_lock(&crossingWest);
	Car *car = (Car*)c;//Ç¿ÖÆÀàÐÍ×ª»»
	//car->dir = West;
	
	//count_West += 1;
	pthread_cond_wait(&queueWest, &crossingWest);
	//pthread_mutex_unlock(&crossingWest);//¿ªÊ¼×¼±¸Í¨ÐÐ
	pthread_mutex_lock(&printlock);
	printf("car %d from west arrives crossing\n", car->id);
	pthread_mutex_unlock(&printlock);
	count_West += 1;
	usleep(1000);
	pthread_mutex_unlock(&crossingWest);//¿ªÊ¼×¼±¸Í¨ÐÐ
	pthread_mutex_lock(&source_west);//Õ¼¾Ý×ÊÔ´
	bool deadlock_flag = false;//ËÀËø±êÊ¶Î»
	if (count_South > 0)//Èç¹ûÓÒ±ßÓÐ³µ
	{
		lastdir=West;
		pthread_cond_signal(&deadlock);
		pthread_cond_wait(&firstWest, &source_west);
	}
	waiting_West = false;
	count_West--;
	pthread_mutex_lock(&printlock);
	printf("car %d from west leaving crossing\n", car->id);
	pthread_mutex_unlock(&printlock);
	pthread_mutex_unlock(&source_west);
	pthread_cond_signal(&firstNorth);//·¢ËÍÐÅºÅ¸ø×ó²à³µ·ÀÖ¹¼¢¶ö
	pthread_cond_signal(&queueWest);//·¢¸ø±¾·½ÏòÏÂÒ»Á¾³µ×¼±¸Í¨ÐÐ
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	pthread_mutex_init(&source_south,NULL);
	pthread_mutex_init(&source_east, NULL);
	pthread_mutex_init(&source_north, NULL);
	pthread_mutex_init(&source_west, NULL);

	pthread_mutex_init(&crossingSouth, NULL);
	pthread_mutex_init(&crossingEast, NULL);
	pthread_mutex_init(&crossingNorth, NULL);
	pthread_mutex_init(&crossingWest, NULL);

	pthread_cond_init(&firstSouth, NULL);
	pthread_cond_init(&firstEast, NULL);
	pthread_cond_init(&firstNorth, NULL);
	pthread_cond_init(&firstWest, NULL);

	pthread_cond_init(&queueSouth, NULL);
	pthread_cond_init(&queueEast, NULL);
	pthread_cond_init(&queueNorth, NULL);
	pthread_cond_init(&queueWest, NULL);

	pthread_cond_init(&deadlock, NULL);
	pthread_mutex_init(&wait_deadlock,NULL);
	pthread_mutex_init(&printlock,NULL);
	Car cars[MAX];
	pthread_t car_threads[MAX];//´´½¨½ø³ÌÁ¿	
	//printf("123");
	pthread_t check;
	pthread_create(&check,NULL,check_deadlock,NULL);
	for (int i = 0; i < strlen(argv[1]); i++)
	{
		cars[i].id = i + 1;
		switch (argv[1][i]) 
		{
			case 's':
			{
				cars[i].dir = South;
				pthread_create(&car_threads[i], NULL,car_from_south,(void *)&cars[i]);
				break;
			}
			case 'e':
			{
				cars[i].dir = East;
				pthread_create(&car_threads[i], NULL, car_from_east, (void*)&cars[i]);
				break;
			}
			case 'n':
			{
				cars[i].dir = North;
				pthread_create(&car_threads[i], NULL, car_from_north, (void*)&cars[i]);
				break;
			}
			case 'w':
			{
				cars[i].dir = West;
				pthread_create(&car_threads[i], NULL, car_from_west, (void*)&cars[i]);
				break;
			}
			default:
				printf("input can not be identified\n");
				exit;
		}
		usleep(100);
	}
	usleep(1000);
	pthread_cond_signal(&queueWest);
	pthread_cond_signal(&queueEast);
	pthread_cond_signal(&queueNorth);
	pthread_cond_signal(&queueSouth);
	//printf("arrive");
	for (int i = 0; i < strlen(argv[1]); i++) 
	{
		pthread_join(car_threads[i], NULL);
	}

	pthread_mutex_destroy(&source_south);
	pthread_mutex_destroy(&source_east);
	pthread_mutex_destroy(&source_north);
	pthread_mutex_destroy(&source_west);

	pthread_mutex_destroy(&crossingSouth);
	pthread_mutex_destroy(&crossingEast);
	pthread_mutex_destroy(&crossingNorth);
	pthread_mutex_destroy(&crossingWest);

	pthread_cond_destroy(&firstSouth);
	pthread_cond_destroy(&firstEast);
	pthread_cond_destroy(&firstNorth);
	pthread_cond_destroy(&firstWest);

	pthread_cond_destroy(&queueSouth);
	pthread_cond_destroy(&queueEast);
	pthread_cond_destroy(&queueNorth);
	pthread_cond_destroy(&queueWest);

	pthread_cond_destroy(&deadlock);
	pthread_mutex_destroy(&wait_deadlock);
	pthread_mutex_destroy(&printlock);
	//pthread_exit(NULL);

	
}
