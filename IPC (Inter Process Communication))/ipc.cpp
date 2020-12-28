#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<string>
#include<unistd.h>
#include<iostream>
#include<sstream>
using namespace std;

//utility functions
string int_to_string (int i) // convert int to string
{
    stringstream s;
    s << i;
    return s.str();
}

#define Q1_SIZE 5
#define Q2_SIZE 10
#define Q3_SIZE 10

//semaphore to control sleep and wakeup
sem_t empty_q1;
sem_t full_q1;

sem_t empty_q2;
sem_t full_q2;

sem_t empty_q3;
sem_t full_q3;

pthread_mutex_t lock1;
pthread_mutex_t lock2;
pthread_mutex_t lock3;
pthread_mutex_t consoleLock;

queue<string> q1;
queue<string> q2;
queue<string> q3;

void init_semaphore()
{
    sem_init(&empty_q1,0,Q1_SIZE);
	sem_init(&full_q1,0,0);
	pthread_mutex_init(&lock1,0);

    sem_init(&empty_q2,0,Q2_SIZE);
	sem_init(&full_q2,0,0);
	pthread_mutex_init(&lock2,0);

    sem_init(&empty_q3,0,Q3_SIZE);
	sem_init(&full_q3,0,0);
	pthread_mutex_init(&lock3,0);

    pthread_mutex_init(&consoleLock, 0);
}

void * ChefX(void * arg)
{
    pthread_mutex_lock(&consoleLock);
    printf("%s\n",(char*)arg);
    pthread_mutex_unlock(&consoleLock);
    int chocolate_cake_id = 0;
    while(1)
    {
        pthread_mutex_lock(&consoleLock);
        std::cout << "Chef X checking Queue-1. If full, takes a break." << '\n';
        pthread_mutex_unlock(&consoleLock);
        sem_wait(&empty_q1);

        //queue 1 not full, so produce cake
        string item = "Chocolate Cake " + int_to_string(++chocolate_cake_id);
        pthread_mutex_lock(&consoleLock);
        std::cout << "Chef X has made " << item << '\n';
        pthread_mutex_unlock(&consoleLock);

        //put in queue and print
        pthread_mutex_lock(&lock1);
        sleep(1);
        q1.push(item);
        pthread_mutex_lock(&consoleLock);
        std::cout << "Chef X puts "<< item << " in Queue-1" << " || ";
        std::cout << "Size of Queue-1: "<< q1.size() << '\n';
        pthread_mutex_unlock(&consoleLock);
        pthread_mutex_unlock(&lock1);

        sem_post(&full_q1);
        //sleep(1)
    }
}

void * ChefY(void * arg)
{
    pthread_mutex_lock(&consoleLock);
    printf("%s\n",(char*)arg);
    pthread_mutex_unlock(&consoleLock);
    int vanilla_cake_id = 0;
    while(1)
    {
        pthread_mutex_lock(&consoleLock);
        std::cout << "Chef Y checking Queue-1. If full, takes a break." << '\n';
        pthread_mutex_unlock(&consoleLock);
        sem_wait(&empty_q1);

        //queue 1 not full, so produce cake
        string item = "Vanilla Cake " + int_to_string(++vanilla_cake_id);
        pthread_mutex_lock(&consoleLock);
        std::cout << "Chef Y has made " << item << '\n';
        pthread_mutex_unlock(&consoleLock);

        //put in queue and print
        pthread_mutex_lock(&lock1);
        sleep(1);
        q1.push(item);
        pthread_mutex_lock(&consoleLock);
        std::cout << "Chef Y puts "<< item << " in Queue-1" <<  " || ";
        std::cout << "Size of Queue-1: "<< q1.size() << '\n';
        pthread_mutex_unlock(&consoleLock);
        pthread_mutex_unlock(&lock1);

        sem_post(&full_q1);
        //sleep(1)
    }
}

void * ChefZ(void * arg)
{
    pthread_mutex_lock(&consoleLock);
    printf("%s\n",(char*)arg);
    pthread_mutex_unlock(&consoleLock);
    while(1)
    {
        pthread_mutex_lock(&consoleLock);
        std::cout << "Chef Z checking Queue-1. If empty, takes a break." << '\n';
        pthread_mutex_unlock(&consoleLock);
        sem_wait(&full_q1);

        //take a look at queue-1
        string item;
        pthread_mutex_lock(&lock1);
        sleep(1);
        item = q1.front();
        pthread_mutex_lock(&consoleLock);
        std::cout << "Chef Z has "<< item << " at the front of Queue-1" << '\n';
        pthread_mutex_unlock(&consoleLock);
        pthread_mutex_unlock(&lock1);

        //putting it to q2 or q3
        if(item.find( "Chocolate Cake") != string::npos ){
            //need to move a chocolate_cake to q3
            //check q3 if full
            pthread_mutex_lock(&consoleLock);
            std::cout << "Chef Z checking Queue-3. If full, takes a break." << '\n';
            pthread_mutex_unlock(&consoleLock);
            sem_wait(&empty_q3);
            //take out from q1
            pthread_mutex_lock(&lock1);
            sleep(1);
            q1.pop();
            pthread_mutex_lock(&consoleLock);
            std::cout << "Chef Z takes "<< item << " from Queue-1" <<  " || ";
            std::cout << "Size of Queue-1: "<< q1.size() << '\n';
            pthread_mutex_unlock(&consoleLock);
            pthread_mutex_unlock(&lock1);
            sem_post(&empty_q1);
            //decorate the cake
            item += " Decorated";
            //put in q3
            pthread_mutex_lock(&lock3);
            sleep(1);
            q3.push(item);
            pthread_mutex_lock(&consoleLock);
            std::cout << "Chef Z puts "<< item << " in Queue-3" <<  " || ";
            std::cout << "Size of Queue-3: "<< q3.size() << '\n';
            pthread_mutex_unlock(&consoleLock);
            pthread_mutex_unlock(&lock3);
            sem_post(&full_q3);
            //sleep(1);
        }

        else{
            //need to move a vanilla_cake to q2
            //check q2 if full
            pthread_mutex_lock(&consoleLock);
            std::cout << "Chef Z checking Queue-2. If full, takes a break." << '\n';
            pthread_mutex_unlock(&consoleLock);
            sem_wait(&empty_q2);
            //take out from q1
            pthread_mutex_lock(&lock1);
            sleep(1);
            q1.pop();
            pthread_mutex_lock(&consoleLock);
            std::cout << "Chef Z takes "<< item << " from Queue-1" <<  " || ";
            std::cout << "Size of Queue-1: "<< q1.size() << '\n';
            pthread_mutex_unlock(&consoleLock);
            pthread_mutex_unlock(&lock1);
            sem_post(&empty_q1);
            //decorate the cake
            item += " Decorated";
            //put in q2
            pthread_mutex_lock(&lock2);
            sleep(1);
            q2.push(item);
            pthread_mutex_lock(&consoleLock);
            std::cout << "Chef Z puts "<< item << " in Queue-2" <<  " || ";
            std::cout << "Size of Queue-2: "<< q2.size() << '\n';
            pthread_mutex_unlock(&consoleLock);
            pthread_mutex_unlock(&lock2);
            sem_post(&full_q2);
            //sleep(1);
        }
        //sem_post(&empty_q1);
        //sleep(1)
    }
}

void * Waiter1(void * arg)
{
    pthread_mutex_lock(&consoleLock);
    printf("%s\n",(char*)arg);
    pthread_mutex_unlock(&consoleLock);
    while(1)
    {
        pthread_mutex_lock(&consoleLock);
        std::cout << "Waiter1 checking Queue-3. If empty, takes a break." << '\n';
        pthread_mutex_unlock(&consoleLock);
        sem_wait(&full_q3);

        //take from queue-3
        string item;
        pthread_mutex_lock(&lock3);
        sleep(1);
        item = q3.front();
        q3.pop();
        pthread_mutex_lock(&consoleLock);
        std::cout << "Waiter1 takes "<< item << " from Queue-3" <<  " || ";
        std::cout << "Size of Queue-3: "<< q3.size() << '\n';
        pthread_mutex_unlock(&consoleLock);
        pthread_mutex_unlock(&lock3);

        sem_post(&empty_q3);
        //sleep(15);
    }
}


void * Waiter2(void * arg)
{
    pthread_mutex_lock(&consoleLock);
    printf("%s\n",(char*)arg);
    pthread_mutex_unlock(&consoleLock);
    while(1)
    {
        pthread_mutex_lock(&consoleLock);
        std::cout << "Waiter2 checking Queue-2. If empty, takes a break." << '\n';
        pthread_mutex_unlock(&consoleLock);
        sem_wait(&full_q2);

        //take from queue-1
        string item;
        pthread_mutex_lock(&lock2);
        sleep(1);
        item = q2.front();
        q2.pop();
        pthread_mutex_lock(&consoleLock);
        std::cout << "Waiter2 takes "<< item << " from Queue-2" <<  " || ";
        std::cout << "Size of Queue-2: "<< q2.size() << '\n';
        pthread_mutex_unlock(&consoleLock);
        pthread_mutex_unlock(&lock2);

        sem_post(&empty_q2);
        //sleep(15);
    }
}


int main(void) {
    pthread_t th_chefX;
    pthread_t th_chefY;
    pthread_t th_chefZ;
    pthread_t th_waiter1;
    pthread_t th_waiter2;

    init_semaphore();

    char * messageX = "I am Chef X - Chocolate Cake Producer.";
    char * messageY = "I am Chef Y - Vanilla Cake Producer.";
    char * messageZ = "I am Chef Z - Cake Decorator.";
    char * message1 = "I am Waiter 1 - Chocolate Cake Server.";
    char * message2 = "I am Waiter 2 - Vanilla Cake Server.";

    pthread_create(&th_chefX,NULL,ChefX,(void*)messageX);
    pthread_create(&th_chefY,NULL,ChefY,(void*)messageY);
    pthread_create(&th_chefZ,NULL,ChefZ,(void*)messageZ);
    pthread_create(&th_waiter1,NULL,Waiter1,(void*)message1);
    pthread_create(&th_waiter2,NULL,Waiter2,(void*)message2);

    while(1);
    return 0;
}
