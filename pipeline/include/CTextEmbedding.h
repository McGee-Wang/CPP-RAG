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
#include <shared_mutex> 
std::vector<std::string> split_lines(const std::string & s, const std::string & separator = "\n");

typedef int (*EmbeddingFunc)(const char **,int,  float***,int*,int *,const char*,int32_t,int,int);
class CTextEmbedding{
      
       private:

       EmbeddingFunc embedding;
       //mutable std::mutex embedding_mutex;
       mutable std::shared_mutex embedding_mutex;
       int32_t n_batch = 8192;
       int embedding_vectors_size = 256;
       int layers;
       
       
       public:

       CTextEmbedding(){
           
           n_batch = 0;
           embedding = 0;
           layers = 0;
           embedding = NULL;
           
       }
       
       CTextEmbedding(std::string embedding_dll,int32_t n_batch,int embedding_vectors_size,int layers){
            
            this->n_batch = n_batch;
            this->embedding_vectors_size = embedding_vectors_size;
            this->layers = layers;

            HMODULE hDLL = LoadLibrary(embedding_dll.c_str());
            if (hDLL == NULL) {
                std::cerr << "无法加载embedding DLL. 错误码: " << GetLastError() << std::endl;
                return;
            }

            // 获取函数指针
            this->embedding = (EmbeddingFunc)GetProcAddress(hDLL, "embedding");
            if (this->embedding == NULL) {
                std::cerr << "无法找到embedding函数. 错误码: " << GetLastError() << std::endl;
                FreeLibrary(hDLL);
                return;
            }

       }

        // 拷贝构造函数
    CTextEmbedding(const CTextEmbedding& other) {
        //std::lock_guard<std::mutex> lock(other.embedding_mutex);
        std::unique_lock<std::shared_mutex> lock(other.embedding_mutex);
        embedding = other.embedding;
        n_batch = other.n_batch;
        embedding_vectors_size = other.embedding_vectors_size;
        layers = other.layers;
    }

    // 移动构造函数
    CTextEmbedding(CTextEmbedding&& other) noexcept {
        //std::lock_guard<std::mutex> lock(other.embedding_mutex);
        std::unique_lock<std::shared_mutex> lock(other.embedding_mutex);
        embedding = other.embedding;
        n_batch = other.n_batch;
        embedding_vectors_size = other.embedding_vectors_size;
        layers = other.layers;

        other.embedding = nullptr;
        other.n_batch = 0;
        other.embedding_vectors_size = 0;
        other.layers = 0;
    }

      CTextEmbedding& operator=(const CTextEmbedding& other) {
        if (this == &other) {
            return *this;
        }

       // std::lock_guard<std::mutex> lock_this(this->embedding_mutex);
        //std::lock_guard<std::mutex> lock_other(other.embedding_mutex);

        std::unique_lock<std::shared_mutex> lock_this(this->embedding_mutex);
        std::unique_lock<std::shared_mutex> lock_other(other.embedding_mutex);

        embedding = other.embedding;
        n_batch = other.n_batch;
        embedding_vectors_size = other.embedding_vectors_size;
        layers = other.layers;

        return *this;
    }
 

       
       //这个是用在在有合并操作的
       std::vector<std::vector<float>> GetEmbedding(std::vector<const char*> texts,const char* model_path){
            int total_combined_texts = texts.size();
            const char **no_embedding_combin_texts = texts.data();
            int res_size;
            int vector_size;
            float **combined_texts_embedding_vectors;

            {
                //std::lock_guard<std::mutex> lock(this->embedding_mutex);  // 线程安全调用
                std::unique_lock<std::shared_mutex> lock(this->embedding_mutex);
                int eres = embedding(no_embedding_combin_texts, total_combined_texts, &combined_texts_embedding_vectors, &res_size, &vector_size, model_path, this->n_batch, this->embedding_vectors_size,this->layers);

                if (eres != 1) {
                    for (int i = 0; i < res_size; i++) {
                        delete[] combined_texts_embedding_vectors[i];
                    }
                    delete[] combined_texts_embedding_vectors;

                    std::cerr << "embedding error!" << std::endl;
                    return {};
                }
            }

            std::vector<std::vector<float>> embedidng_vectors;
            for (int i = 0; i < res_size; i++) {
                std::vector<float> embedding_vector;
                for (int j = 0; j < vector_size; j++) {
                    embedding_vector.push_back(combined_texts_embedding_vectors[i][j]);
                }
                embedidng_vectors.push_back(embedding_vector);
            }

            for (int i = 0; i < res_size; i++) {
                delete[] combined_texts_embedding_vectors[i];
            }
            delete[] combined_texts_embedding_vectors;

            return embedidng_vectors;
             
       }
        
       //没有合并操作来的
       std::vector<std::vector<float>> GetEmbedding(std::vector<std::pair<std::string, std::vector<std::string>>> texts,const char* model_path){
           
            std::vector<const char*> total_texts;
            std::string new_str = "";
            int total_text_count = 0;
            std::vector<std::string> lines;

            for (auto& file : texts) {
                for (const auto& text : file.second) {
                    std::vector<std::string> templines = split_lines(text);
                    new_str = "";
                    for (auto& line : templines) {
                        new_str += line;
                    }
                    total_text_count++;
                    lines.push_back(new_str);
                }
            }

            for (auto& line : lines) {
              total_texts.push_back(line.c_str());
            }
             
            return this->GetEmbedding(total_texts,model_path);
             

       }


    std::vector<std::vector<float>> embedding_multithread(std::vector<std::pair<std::string, std::vector<std::string>>> splited_text,const std::vector<const char*>& model_paths) {
            const int num_threads = model_paths.size();
            size_t total_files = splited_text.size();
            size_t files_per_thread = (total_files + num_threads - 1) / num_threads;
            std::vector<std::future<std::pair<size_t, std::vector<std::vector<float>>>>> futures;
            
            for (int t = 0; t < num_threads; ++t) {
                futures.push_back(std::async(std::launch::async, [this,t, files_per_thread, &splited_text,&model_paths, total_files]()->std::pair<size_t, std::vector<std::vector<float>>> {
                    size_t start_index = t * files_per_thread;
                    size_t end_index = std::min(start_index + files_per_thread, total_files);

                    std::vector<std::pair<std::string, std::vector<std::string>>> sub_splited_text(
                        splited_text.begin() + start_index,
                        splited_text.begin() + end_index
                    );
            
            auto result = this->GetEmbedding(sub_splited_text, model_paths[t]);
            return std::make_pair(start_index, result);
        }));
    }

    std::vector<std::pair<size_t, std::vector<std::vector<float>>>> results;

    for (auto& future : futures) {
        results.push_back(future.get());
    }

    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    std::vector<std::vector<float>> final_result;

    for (const auto& result : results) {
        final_result.insert(final_result.end(), result.second.begin(), result.second.end());
    }

    return final_result;
}


};