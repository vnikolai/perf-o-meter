/* Copyright 2020 Volodymyr Nikolaichuk

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#include <perfometer/perfometer.h>
#include <perfometer/helpers.h>

#include <iostream>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <functional>
#include <random>

const int num_threads = std::thread::hardware_concurrency();

class thread_pool
{
public:
    thread_pool( )
    {
    }

    ~thread_pool( )
    {
        stop();
    }

    void run( )
    {
		PERFOMETER_LOG_WORK_FUNCTION();

        if ( m_running ) return;

        m_running = true;

        for ( int i = 0; i < num_threads; ++i )
        {
            m_threads.push_back( std::thread( &thread_pool::worker_thread, this ) );
        }

    }

    void stop( )
    {
        if ( !m_running ) return;

        wait();

        m_running = false;
        m_task_ready.notify_all( );

        for ( auto& thread : m_threads )
        {
            if ( thread.joinable( ) )
            {
                thread.join( );
            }
        }
    }

    void push( std::function< void(void) > function )
    {
        if ( function )
        {
            {
                std::unique_lock< std::mutex > lock( m_task_queue_lock );
                m_task_queue.push( function );
            }

            m_task_ready.notify_one( );
        }
    }

    void wait( )
    {
        if ( !m_running )
        {
            return;
        }

        while ( true )
        {
            std::unique_lock< std::mutex > lock( m_task_queue_lock );
            if ( m_task_queue.empty( ) )
            {
                break;
            }

            lock.unlock( );
            std::this_thread::yield( );
        }
    }

private:
    bool fetch_task( std::function<void(void)>& function )
    {
        if (!m_task_queue.empty())
        {
            function = std::move(m_task_queue.front());
            m_task_queue.pop();
            return true;
        }

        return false;
    }

    void worker_thread( )
    {
		PERFOMETER_LOG_THREAD_NAME("WORKER_THREAD");

		PERFOMETER_LOG_WORK_FUNCTION();
        while ( m_running )
        {
            std::unique_lock< std::mutex > guard( m_task_queue_lock );
            m_task_ready.wait( guard, [&]( ) {
                return !m_task_queue.empty( ) || !m_running;
            } );

            if ( m_running )
            {
                std::function<void(void)> task;
                if ( fetch_task( task ) )
                {
                    guard.unlock( );

                    task( );
                }
            }
        }
    }

private:
    std::atomic_bool m_running = false;

    std::mutex m_task_queue_lock;
    std::queue< std::function<void(void)> > m_task_queue;
    std::condition_variable m_task_ready;

    std::vector< std::thread > m_threads;
};

int random(int max)
{
    // return (std::rand() ) / ((RAND_MAX + 1u) / max);
    
    static std::random_device rd;  //Will be used to obtain a seed for the random number engine
    static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    static std::uniform_int_distribution<> distrib(0, max);

    return distrib(gen);
}

void task()
{
	PERFOMETER_LOG_WORK_FUNCTION();

    std::this_thread::sleep_for(std::chrono::milliseconds(random(5) * 100));
}

int main(int argc, const char** argv)
{
	auto result = perfometer::initialize();
	std::cout << "perfometer::initialize() returned " << result << std::endl;

	PERFOMETER_LOG_THREAD_NAME("MAIN_THREAD");

    thread_pool pool;

    for (int i = 0; i < 132; i++)
    {
        pool.push(std::function<void(void)>(task));
    }

    pool.run();
    pool.stop();

	result = perfometer::shutdown();
	std::cout << "perfometer::shutdown() returned " << result << std::endl;

	return 0;
}
