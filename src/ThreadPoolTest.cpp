#include "ccc/ThreadPool.h"

#include <functional>
#include <iostream>

void ThreadMain(int i)
{
  std::cout << (i % 10);
}

int main()
{
    ThreadPool pool(8);
    for (int i = 0; i < 100; i++)
    {
        pool.Schedule(std::bind(&ThreadMain, i));
    }
    pool.WaitForAll();
    std::cout << std::endl;
}
