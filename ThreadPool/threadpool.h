#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

typedef struct ThreadPool ThreadPool;

//�����̳߳ز���ʼ��
ThreadPool* threadPoolCreate(int m, int max, int queueSize);

//�����̳߳�
int threadPoolDestory(ThreadPool* pool);
//���̳߳��������
void threadPoolAdd(ThreadPool* pool,void(*func)(void*),void* arg);

//��ȡ�̳߳��й������̵߳ĸ���
int threadPoolBusyNum(ThreadPool* pool);

//��ȡ�̳߳��л��ŵ��̵߳ĸ���
int threadPoolAliveNum(ThreadPool* pool);

void* worker(void* arg);
void* manager(void* arg);
void threadExit(ThreadPool* pool);

#endif // !_THREADPOOL_H
