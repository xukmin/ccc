#include "ccc/ThreadPool.h"

#include <functional>

ThreadPool::ThreadPool(int numThreads)
    : m_shouldStop(false), m_numUnfinishedTasks(0)
{
    for (int i = 0; i< numThreads; i++)
    {
        m_threads.push_back(
            boost::thread(std::bind(&ThreadPool::ThreadMain, this)));
    }
}

ThreadPool::~ThreadPool()
{
    {
        boost::mutex::scoped_lock lock(m_mutex);
        m_shouldStop = true;
    }
    m_shouldWakeUpThreads.notify_all();

    for (int i = 0; i < m_threads.size(); i++)
    {
        m_threads[i].join();
    }
}

bool ThreadPool::Schedule(boost::function<void ()> task)
{
    {
        boost::mutex::scoped_lock lock(m_mutex);
        if (m_shouldStop)
        {
            return false;
        }
        m_tasks.push(task);
        m_numUnfinishedTasks++;
    }
    m_shouldWakeUpThreads.notify_all();
    return true;
}

void ThreadPool::ThreadMain()
{
    while (true)
    {
        {
            boost::mutex::scoped_lock lock(m_mutex);
            if (m_numUnfinishedTasks == 0 && m_shouldStop)
            {
                break;
            }
        }

        boost::function<void ()> task;
        bool hasNewTask = false;

        {
            boost::mutex::scoped_lock lock(m_mutex);
            while (m_tasks.empty() && !m_shouldStop)
            {
                m_shouldWakeUpThreads.wait(lock);
            }

            if (!m_tasks.empty())
            {
              task = m_tasks.front();
              m_tasks.pop();
              hasNewTask = true;
            }
        }

        if (hasNewTask)
        {
            task();
            boost::mutex::scoped_lock lock(m_mutex);
            if (--m_numUnfinishedTasks == 0)
            {
                m_allTasksFinished.notify_all();
            }
        }
    }
}

void ThreadPool::WaitForAll()
{
    boost::mutex::scoped_lock lock(m_mutex);
    while (m_numUnfinishedTasks > 0)
    {
        m_allTasksFinished.wait(lock);
    }
}
