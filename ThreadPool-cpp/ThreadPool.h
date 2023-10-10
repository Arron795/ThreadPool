#pragma once
#include <iostream>
#include <pthread.h>
#include "TaskQueue.h"

template <typename T>
class ThreadPool
{
public:
	//�����̳߳ز���ʼ��
	ThreadPool(int min, int max);

	//�����̳߳�
	~ThreadPool();

	//���̳߳��������
	void addTask(Task<T> task);

	

	//��ȡ�̳߳��й������̵߳ĸ���
	int getBusyNum();

	//��ȡ�̳߳��л��ŵ��̵߳ĸ���
	int getAliveNum();

private:
	static void* worker(void* arg);				//�������̣߳��������̣߳�������
	static void* manager(void* arg);				//�������߳�������
	void threadExit();						//�����߳��˳�

private:
	//�������
	TaskQueue<T>* taskQ;

	pthread_t managerID;			//�������߳�ID
	pthread_t* threadIDs;			//�����߳�ID
	int minNum;						//��С�߳�����
	int maxNum;						//����߳�����
	int busyNum;					//æ���̸߳���
	int liveNum;					//�����̸߳���
	int exitNum;					//�˳����̸߳���
	pthread_mutex_t mutexPool;		//��ס�����̳߳�
	pthread_cond_t notEmpty;		//��������Ƿ�Ϊ��	�����ﲻ��Ҫ�����Ƿ�Ϊ������Ϊ�������������TaskQueue��������һ����̬���飩
	static const int NUMBER = 2;

	bool	shutdown;				//1���٣� 0������
};

