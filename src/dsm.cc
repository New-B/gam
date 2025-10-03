#include "dsm.h"
#include <thread>
#include <mutex>
#include <unordered_map>

static const Conf* conf = nullptr;
static std::mutex init_lock;
static std::mutex map_lock; //用于保护映射表
GAlloc** alloc;
static int no_thread = 0;
static std::unordered_map<std::thread::id, int> thread_to_alloc_map; //线程ID到分配器索引的映射表

/*----------------------internal helper------------------------*/
// 获取当前线程对应的分配器索引
static int GetAllocIndexForThread() {
    std::thread::id thread_id = std::this_thread::get_id();
    {
        std::lock_guard<std::mutex> guard(map_lock);
        auto it = thread_to_alloc_map.find(thread_id);
        if (it != thread_to_alloc_map.end()) {
            // 如果映射已存在，返回对应的索引
            return it->second;
        }

        // 如果映射不存在，创建新的映射
        int new_index = thread_to_alloc_map.size();
        if (new_index >= no_thread) {
            throw std::runtime_error("Exceeded maximum number of threads");
        }
        thread_to_alloc_map[thread_id] = new_index;
        return new_index;
    }
}

/*----------------------API functions------------------------*/
void dsm_init(const Conf* c){
    std::lock_guard<std::mutex> guard(init_lock);
    GAllocFactory::InitSystem(c);
    sleep(10);
    no_thread = c->maxthreads; //获取线程数
    alloc = new GAlloc*[no_thread];
    for (int i = 0; i < no_thread; ++i) {
        alloc[i] = GAllocFactory::CreateAllocator();
    }
}

GAddr dsmMalloc(Size size, Node nid) {
    int index = GetAllocIndexForThread(); // 获取当前线程对应的分配器索引 
    return alloc[index]->Malloc(size, nid);
}

int dsmRead(GAddr addr, void* buf, Size count) {
    int index = GetAllocIndexForThread(); // 获取当前线程对应的分配器索引 
    return alloc[index]->Read(addr, buf, count);
}

int dsmWrite(GAddr addr, void* buf, Size count) {
    int index = GetAllocIndexForThread(); // 获取当前线程对应的分配器索引 
    return alloc[index]->Write(addr, buf, count);
}

void dsmFree(GAddr addr) {
    int index = GetAllocIndexForThread(); // 获取当前线程对应的分配器索引 
    alloc[index]->Free(addr);
}

void dsm_finalize() {
    std::lock_guard<std::mutex> guard(init_lock);
    //GAllocFactory::FreeResouce();
    for (int i = 0; i < no_thread; ++i) {
        delete alloc[i];
    }
    delete[] alloc;
    conf = nullptr;
}