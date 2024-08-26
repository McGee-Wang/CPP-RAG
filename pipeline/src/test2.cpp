#include<string>
#include<iostream>
#include <stdexcept>
#include <stdio.h>
#include <vector>
#include <iterator>
#include <thread>
#include <chrono>
#include <future>
#include <unistd.h>
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <tuple>
#include <mutex>

typedef int (*EmbeddingFunc)(const char **,int,  float***,int*,int *,const char*,int32_t,int,int);


int main(){
     
          HMODULE hDLL = LoadLibrary("llama-embedding.dll");
            if (hDLL == NULL) {
                std::cerr << "无法加载embedding DLL. 错误码: " << GetLastError() << std::endl;
                return 0;
            }

            // 获取函数指针
          EmbeddingFunc embedding = (EmbeddingFunc)GetProcAddress(hDLL, "embedding");
            if (embedding == NULL) {
                std::cerr << "无法找到embedding函数. 错误码: " << GetLastError() << std::endl;
                FreeLibrary(hDLL);
                return 0;
            }

            return 1;
}