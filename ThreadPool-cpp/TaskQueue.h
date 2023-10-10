#pragma once
#include <iostream>
#include <queue>
#include <pthread.h>


using callback = void(*)(void* arg);

//����ṹ��
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

	//�������
	void addTask(Task<T> task);
	void addTask(callback f, void* arg);

	//ȡ������
	Task<T> takeTask();

	//��ȡ��ǰ����ĸ���
	inline size_t taskNumber()					//inline ��������������ѹջ��ֱ�ӽ��д������滻���������滻
	{										//					��ѹջЧ�ʸߣ������ڲ�һ��ֻ�������д��룬
		//size()���ص���һ��size_t  һ���޷�������
		return m_taskQ.size();				//					û��ѭ���������жϣ�û���κ��ж� ��ʱ�����д����������
	}
private:
	pthread_mutex_t m_mutex;
	std::queue<Task<T>> m_taskQ;
};
