#ifndef DSM_H_
#define DSM_H_

#include "gallocator.h"

#ifdef __cplusplus
extern "C" {
#endif

// static void InitSystem(const std::string& conf_file);

void dsm_init(const Conf* c = nullptr);

// 分配内存
GAddr dsm_malloc(Size size, Node nid = 0);

// 读取数据
int dsm_read(GAddr addr, void* buf, Size count);

// 写入数据
int dsm_write(GAddr addr, void* buf, Size count);

// 释放内存
void dsm_free(GAddr addr);

void dsm_finalize();

#ifdef __cplusplus
}
#endif

#endif // DSM_H_