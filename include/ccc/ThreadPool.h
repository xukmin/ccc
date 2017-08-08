#include <queue>
#include <vector>

#include "boost/function.hpp"
#include "boost/thread/condition_variable.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/thread.hpp"

class ThreadPool
{
public:
    explicit ThreadPool(int numThreads);
    ~ThreadPool();
    bool Schedule(boost::function<void ()> task);
    void WaitForAll();

private:
    void ThreadMain();

    std::vector<boost::thread> m_threads;
    std::queue<boost::function<void ()> > m_tasks;
    int m_numUnfinishedTasks;
    bool m_shouldStop;

    boost::mutex m_mutex;
    boost::condition_variable m_shouldWakeUpThreads;
    boost::condition_variable m_allTasksFinished;
};
