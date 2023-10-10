#include "ThreadPool.h"
#include "TaskQueue.h"
#include <iostream>
#include <string>		//C++���string��
#include <string.h>		//C�������ͷ�ļ� �����õ���api���� memset��strcpy��strcompare��memcpy...
#include <unistd.h>


using namespace std;

template <typename T>
ThreadPool<T>::ThreadPool(int min, int max)
{
	//ʵ�����������
	do
	{
		taskQ = new TaskQueue<T>;
		if (taskQ == nullptr)
		{
			cout << ("malloc taskQ fail... ") << endl;
			break;
		}

		threadIDs = new pthread_t[max];
		if (threadIDs == nullptr)
		{
			cout <<("malloc threadIDs fail... ") << endl;
			break;
		}

		memset(threadIDs, 0, sizeof(pthread_t) * max);
		minNum = min;
		maxNum = max;
		busyNum = 0;
		liveNum = min;		//����С�������
		exitNum = 0;

		if (pthread_mutex_init(&mutexPool, NULL) != 0 ||
			pthread_cond_init(&notEmpty, NULL) != 0 )
		{
			cout << ("mutex and condition init fail... ") <<endl;
			break;
		}

		shutdown = false;

		//�����߳�
		//pthread_create(&pool->managerID, NULL, manager, NULL);	
		//���������˴����ĸ������Ǵ��ݸ�manger�Ĳ���
		//�˴���pool������ڴ���Ϊ���������ݸ�manager��Ӧ��Ϊpool������NULL
		pthread_create(&managerID, NULL, manager, this);						//�����������߳�
		for (int i = 0; i < min; i++)
		{
			pthread_create(&threadIDs[i], NULL, worker, this);					//���������߳�
			//cout << "�������߳�, ID: " << to_string(threadIDs[i]) << endl;
		}
		
	} while (0);

	
	
	//�ͷ���Դ															//���������캯������Ҫ���ͷ���Դ
	//if (threadIDs)
	//{
	//	//cout << "threadIDs = " << threadIDs << endl;					//debug���
	//	//delete[] threadIDs;
	//}
	//if (taskQ)
	//{
	//	//cout << "taskQ = " << taskQ << endl;
	//	//delete taskQ;
	//}
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
	//�ر��̳߳�
	shutdown = true;
	//�������չ������߳�
	pthread_join(managerID, NULL);
	//�����������������߳�
	for (int i = 0; i < liveNum; i++)
	{
		pthread_cond_signal(&notEmpty);
	}

	//�ͷŶ��ڴ�
	if (taskQ)
	{
		delete taskQ;
	}
	if (threadIDs)
	{
		delete[] threadIDs;
	}
	
	pthread_mutex_destroy(&mutexPool);
	pthread_cond_destroy(&notEmpty);

	//return ;	//��������û�з���ֵ
}

template <typename T>
void ThreadPool<T>::addTask(Task<T> task)
{
	if (shutdown)
	{
		return;
	}

	//�������
	taskQ->addTask(task);

	pthread_cond_signal(&notEmpty);
}

template <typename T>
int ThreadPool<T>::getBusyNum()
{
	int busyNum = 0;
	pthread_mutex_lock(&mutexPool);
	busyNum = this->busyNum;
	pthread_mutex_unlock(&mutexPool);
	return busyNum;
}

template <typename T>
int ThreadPool<T>::getAliveNum()
{
	int aliveNum = 0;
	pthread_mutex_lock(&mutexPool);
	aliveNum = this->liveNum;
	pthread_mutex_unlock(&mutexPool);
	return aliveNum;
}

template <typename T>
void* ThreadPool<T>::worker(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);		//stattic_cast<>()�൱��C��ǿ������ת��

	while (1)
	{
		pthread_mutex_lock(&pool->mutexPool);
		//��ǰ��������Ƿ�Ϊ��
		while (pool->taskQ->taskNumber() == 0 && !pool->shutdown)		//��������Ϊ0 ���̳߳�û�б��ر�
		{
			//���������߳�
			pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);

			//�ж��ǲ���Ҫ�����߳�
			if (pool->exitNum > 0)
			{
				pool->exitNum--;
				if (pool->liveNum > pool->minNum)
				{
					pool->liveNum--;
					pthread_mutex_unlock(&pool->mutexPool);
					pool->threadExit();
				}
			}
		}

		//�ж��̳߳��Ƿ񱻹ر���
		if (pool->shutdown)
		{
			pthread_mutex_unlock(&pool->mutexPool);
			pool->threadExit();
		}

		//�����������ȡ��һ������
		Task<T> task = pool->taskQ->takeTask();

		//�ƶ�ͷ���
		//����
		pool->busyNum++;
		pthread_mutex_unlock(&pool->mutexPool);

		//ִ������
		cout << "thread " << to_string(pthread_self()) <<" start working... " << endl;

		task.function(task.arg);
		delete task.arg;
		task.arg = nullptr;

		//�������
		cout << "thread " << to_string(pthread_self()) << " .end working... " << endl;

		pthread_mutex_lock(&pool->mutexPool);
		pool->busyNum--;
		pthread_mutex_unlock(&pool->mutexPool);
	}
	return NULL;
}

template <typename T>
void* ThreadPool<T>::manager(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);
	while (!pool->shutdown)
	{
		//ÿ��3s���һ��
		sleep(3);

		// ȡ���̳߳�������������͵�ǰ�̵߳�����
		pthread_mutex_lock(&pool->mutexPool);
		int queueSize = pool->taskQ->taskNumber();
		int liveNum = pool->liveNum;
		int busyNum = pool->busyNum;					//ȡ��æ���߳�����
		pthread_mutex_unlock(&pool->mutexPool);

		


		//����߳�
		//����ĸ��� > �����̸߳��� && �����߳��� < ����߳���
		if (queueSize > liveNum && liveNum < pool->maxNum)
		{
			pthread_mutex_lock(&pool->mutexPool);
			int counter = 0;
			for (int i = 0; i < pool->maxNum && counter < NUMBER && pool->liveNum < pool->maxNum; i++)
			{
				if (pool->threadIDs[i] == 0)
				{
					pthread_create(&pool->threadIDs[i], NULL, worker, pool);
					counter++;
					pool->liveNum++;
				}
			}
			pthread_mutex_unlock(&pool->mutexPool);
		}

		//�����߳�
		//æ���߳�*2 < �����߳� && �����߳� > ��С�߳���
		if (busyNum * 2 < liveNum && liveNum > pool->minNum)
		{
			pthread_mutex_lock(&pool->mutexPool);
			pool->exitNum = NUMBER;								//�趨Ҫ���ٵ��߳���
			pthread_mutex_unlock(&pool->mutexPool);
			//  �ù������߳���ɱ
			for (int i = 0; i < NUMBER; i++)
			{
				pthread_cond_signal(&pool->notEmpty);
			}
		}
	}
	return NULL;
}

template <typename T>
void ThreadPool<T>::threadExit()
{
	pthread_t tid = pthread_self();
	for (int i = 0; i < maxNum; i++)
	{
		if (threadIDs[i] == tid)
		{
			threadIDs[i] = 0;
			cout <<"threadExit() called , "<< to_string(tid)<<" .exiting..." <<endl;
			break;
		}
	}
	pthread_exit(NULL);
}