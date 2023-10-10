#pragma once
#include <iostream>
#include <pthread.h>
#include "TaskQueue.h"

template <typename T>
class ThreadPool
{
public:
	//创建线程池并初始化
	ThreadPool(int min, int max);

	//销毁线程池
	~ThreadPool();

	//给线程池添加任务
	void addTask(Task<T> task);

	

	//获取线程池中工作的线程的个数
	int getBusyNum();

	//获取线程池中活着的线程的个数
	int getAliveNum();

private:
	static void* worker(void* arg);				//工作的线程（消费者线程）任务函数
	static void* manager(void* arg);				//管理者线程任务函数
	void threadExit();						//单个线程退出

private:
	//任务队列
	TaskQueue<T>* taskQ;

	pthread_t managerID;			//管理者线程ID
	pthread_t* threadIDs;			//工作线程ID
	int minNum;						//最小线程数量
	int maxNum;						//最大线程数量
	int busyNum;					//忙的线程个数
	int liveNum;					//存活的线程个数
	int exitNum;					//退出的线程个数
	pthread_mutex_t mutexPool;		//锁住整个线程池
	pthread_cond_t notEmpty;		//任务队列是否为空	（这里不需要考虑是否为满，因为交给了任务队列TaskQueue管理，他是一个动态数组）
	static const int NUMBER = 2;

	bool	shutdown;				//1销毁， 0不销毁
};

