#include "ThreadPool.h"
#include "TaskQueue.h"
#include <iostream>
#include <string>		//C++里的string类
#include <string.h>		//C语言里的头文件 比如用到的api函数 memset、strcpy、strcompare、memcpy...
#include <unistd.h>


using namespace std;

template <typename T>
ThreadPool<T>::ThreadPool(int min, int max)
{
	//实例化任务队列
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
		liveNum = min;		//和最小个数相等
		exitNum = 0;

		if (pthread_mutex_init(&mutexPool, NULL) != 0 ||
			pthread_cond_init(&notEmpty, NULL) != 0 )
		{
			cout << ("mutex and condition init fail... ") <<endl;
			break;
		}

		shutdown = false;

		//创建线程
		//pthread_create(&pool->managerID, NULL, manager, NULL);	
		//！！！！此处第四个参数是传递给manger的参数
		//此处将pool整块堆内存作为参数都传递给manager，应该为pool，而非NULL
		pthread_create(&managerID, NULL, manager, this);						//创建管理者线程
		for (int i = 0; i < min; i++)
		{
			pthread_create(&threadIDs[i], NULL, worker, this);					//创建工作线程
			//cout << "创建子线程, ID: " << to_string(threadIDs[i]) << endl;
		}
		
	} while (0);

	
	
	//释放资源															//！！！构造函数不需要放释放资源
	//if (threadIDs)
	//{
	//	//cout << "threadIDs = " << threadIDs << endl;					//debug语句
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
	//关闭线程池
	shutdown = true;
	//阻塞回收管理者线程
	pthread_join(managerID, NULL);
	//唤醒阻塞的消费者线程
	for (int i = 0; i < liveNum; i++)
	{
		pthread_cond_signal(&notEmpty);
	}

	//释放堆内存
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

	//return ;	//析构函数没有返回值
}

template <typename T>
void ThreadPool<T>::addTask(Task<T> task)
{
	if (shutdown)
	{
		return;
	}

	//添加任务
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
	ThreadPool* pool = static_cast<ThreadPool*>(arg);		//stattic_cast<>()相当于C的强制类型转换

	while (1)
	{
		pthread_mutex_lock(&pool->mutexPool);
		//当前任务队列是否为空
		while (pool->taskQ->taskNumber() == 0 && !pool->shutdown)		//任务数量为0 且线程池没有被关闭
		{
			//阻塞工作线程
			pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);

			//判断是不是要销毁线程
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

		//判断线程池是否被关闭了
		if (pool->shutdown)
		{
			pthread_mutex_unlock(&pool->mutexPool);
			pool->threadExit();
		}

		//从任务队列中取出一个任务
		Task<T> task = pool->taskQ->takeTask();

		//移动头结点
		//解锁
		pool->busyNum++;
		pthread_mutex_unlock(&pool->mutexPool);

		//执行任务
		cout << "thread " << to_string(pthread_self()) <<" start working... " << endl;

		task.function(task.arg);
		delete task.arg;
		task.arg = nullptr;

		//任务结束
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
		//每隔3s检测一次
		sleep(3);

		// 取出线程池中任务的数量和当前线程的数量
		pthread_mutex_lock(&pool->mutexPool);
		int queueSize = pool->taskQ->taskNumber();
		int liveNum = pool->liveNum;
		int busyNum = pool->busyNum;					//取出忙的线程数量
		pthread_mutex_unlock(&pool->mutexPool);

		


		//添加线程
		//任务的个数 > 存活的线程个数 && 存活的线程数 < 最大线程数
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

		//销毁线程
		//忙的线程*2 < 存活的线程 && 存活的线程 > 最小线程数
		if (busyNum * 2 < liveNum && liveNum > pool->minNum)
		{
			pthread_mutex_lock(&pool->mutexPool);
			pool->exitNum = NUMBER;								//设定要销毁的线程数
			pthread_mutex_unlock(&pool->mutexPool);
			//  让工作的线程自杀
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