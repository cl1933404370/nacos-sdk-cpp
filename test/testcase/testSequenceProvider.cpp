#include <iostream>
#include "src/thread/Thread.h"
#include "src/utils/SequenceProvider.h"
#include "src/utils/DirUtils.h"

using namespace std;
using namespace nacos;

constexpr auto NR_THREADS = 200;
constexpr auto GENERATION_PER_THREAD = 1000;

int64_t sequences[GENERATION_PER_THREAD * NR_THREADS];
int tid[NR_THREADS];

SequenceProvider<int64_t>* sequenceProvider;

void* SeqThreadFunc(void* param)
{
    const int* threadNo = static_cast<int*>(param);
    for (int i = 0; i < GENERATION_PER_THREAD; i++)
    {
        const int64_t res = sequenceProvider->next(); 
        sequences[*threadNo * GENERATION_PER_THREAD + i] = res;
    }

    return nullptr;
}

bool testSequenceProvider()
{
    cout << "in function testSequenceProvider" << endl;

    cout << "Generating SEQ..." << endl;

    sequenceProvider = new SequenceProvider<int64_t>(DirUtils::getCwd() + "/test_seq.dat", 20000, 100);

    Thread* threads[NR_THREADS] = {nullptr};
    for (int i = 0; i < NR_THREADS; i++)
    {
        const NacosString threadName = "SEQThread-" + NacosStringOps::valueOf(i);
        tid[i] = i;
        threads[i] = new Thread(threadName, SeqThreadFunc, static_cast<void*>(&tid[i]));
        threads[i]->start();
    }

    for (auto& thread : threads)
    {
        thread->join();
        delete thread;
    }
    cout << "Generated." << endl;
    for (int i = 0; i < NR_THREADS; i++)
    {
        for (int j = 0; j < GENERATION_PER_THREAD; j++)
        {
            cout << "Thread " << i << ": sequence =\t" << sequences[i * GENERATION_PER_THREAD + j] << endl;
        }
    }
    delete sequenceProvider;
    cout << "test end..." << endl;
    
    return true;
}
