#pragma once
#include <iostream>
#include <queue>
#include <pthread.h>


using callback = void(*)(void* arg);

//任务结构体
template <typename T>
struct Task
{
	Task<T>()
	{
		function = nullptr;
		arg = nullptr;
	}
	Task<T>(callback f,void* arg)
	{
		function = f;
		this->arg = (void*)arg;
	}
	callback function;
	void* arg;
};

template <typename T>
class TaskQueue
{
public:
	TaskQueue();
	~TaskQueue();

	//添加任务
	void addTask(Task<T> task);
	void addTask(callback f, void* arg);

	//取出任务
	Task<T> takeTask();

	//获取当前任务的个数
	inline size_t taskNumber()					//inline 内联函数：不会压栈，直接进行代码块的替换，代码块的替换
	{										//					比压栈效率高，函数内部一般只有三五行代码，
		//size()返回的是一个size_t  一个无符号整型
		return m_taskQ.size();				//					没有循环、条件判断，没有任何判定 的时候可以写成内联函数
	}
private:
	pthread_mutex_t m_mutex;
	std::queue<Task<T>> m_taskQ;
};
