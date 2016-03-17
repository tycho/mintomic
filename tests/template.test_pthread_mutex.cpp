// This test is based on the blog post: http://preshing.com/20121019/this-is-why-they-call-it-a-weakly-ordered-cpu

#include <mintomic/mintomic.h>
#include <mintpack/timewaster.h>
#include <mintpack/threadsynchronizer.h>
#include <assert.h>
#include <stdio.h>


#cmakedefine01 TEST_FORCE_FAIL

static TimeWaster g_timeWasters[4];
static mint_atomic${TEST_INT_BITSIZE}_t g_flag;
static uint${TEST_INT_BITSIZE}_t g_sharedValue;
static pthread_mutex_t g_lock;

static void threadFunc(int threadNum)
{
    TimeWaster &tw = g_timeWasters[threadNum];

    int count = 0;
    while (count < 5000000)
    {
        tw.wasteRandomCycles();
        if (!pthread_mutex_trylock(&g_lock))
        {
            // Lock was successful
#if !TEST_FORCE_FAIL
            mint_thread_fence_acquire();
#endif
            g_sharedValue++;
#if !TEST_FORCE_FAIL
            mint_thread_fence_release();
#endif
            pthread_mutex_unlock(&g_lock);
            count++;
        }
    }
}

bool ${TEST_FUNC}(int numThreads)
{
    pthread_mutexattr_t attr;

    assert(numThreads <= 4);

    g_flag._nonatomic = 0;
    g_sharedValue = 0;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    pthread_mutex_init(&g_lock, &attr);

    ThreadSynchronizer threads(numThreads);
    threads.run(threadFunc);
    bool success = (g_sharedValue == (uint${TEST_INT_BITSIZE}_t) 5000000 * numThreads);
    //if (!success)
    //    printf(" g_sharedValue=%llu", (uint64_t) g_sharedValue);
    return success;
}
