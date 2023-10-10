#include "TaskQueue.h"
#include <pthread.h>
#include <stdlib.h>
#include <iostream>

template <typename T>
TaskQueue<T>::TaskQueue()
{
	pthread_mutex_init(&m_mutex, NULL);		//��ʼ��������
}

template <typename T>
TaskQueue<T>::~TaskQueue()
{
	pthread_mutex_destroy(&m_mutex);						//���ٻ�����
}

template <typename T>
void TaskQueue<T>::addTask(Task<T> task)
{
	pthread_mutex_lock(&m_mutex);
	m_taskQ.push(task);
	pthread_mutex_unlock(&m_mutex);
}

template <typename T>
void TaskQueue<T>::addTask(callback f, void* arg)
{
	pthread_mutex_lock(&m_mutex);
	m_taskQ.push(Task<T>(f,arg));
	pthread_mutex_unlock(&m_mutex);
}

template <typename T>
Task<T> TaskQueue<T>::takeTask()
{
	Task<T> t;
	pthread_mutex_lock(&m_mutex);
	if (!m_taskQ.empty())
	{
		t = m_taskQ.front();	//ȡ����ͷԪ�ص�ֵ����δ����	Ŀ����Ϊ�˴�ס��ͷԪ�ص�ֵ�Ա㷵��
		m_taskQ.pop();
	}
	pthread_mutex_unlock(&m_mutex);
	return t;
}
