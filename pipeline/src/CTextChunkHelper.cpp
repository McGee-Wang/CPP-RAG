#include "../include/CTextChunkHelper.h"
#include <algorithm>
#include<vector>
#include<string>
#include<unordered_map>
#include<mutex>
#include<tuple>
#include<future>
#include<iostream>
#include <cstdlib>
#include <chrono>
#include<cmath>

float CosineSimilarityDistance(const std::vector<float>& A, const std::vector<float>& B) {
    // Check if vectors are empty
    if (A.empty() || B.empty()) {
        throw std::invalid_argument("Vectors cannot be empty");
    }

    // Get the minimum length between the two vectors
    size_t minLength = std::min(A.size(), B.size());

    float dotProduct = 0.0;
    float normA = 0.0;
    float normB = 0.0;
    int validPairs = 0;

    for (size_t i = 0; i < minLength; ++i) {
        // Skip if either value is NaN
        if (std::isnan(A[i]) || std::isnan(B[i])) {
            continue;
        }

        // Handle zero values
        if (A[i] == 0.0 && B[i] == 0.0) {
            ++validPairs;  // Consider matching zeros as similar
            continue;
        }
        if (A[i] == 0.0 || B[i] == 0.0) {
            continue;  // Skip if only one value is zero
        }

        dotProduct += A[i] * B[i];
        normA += A[i] * A[i];
        normB += B[i] * B[i];
        ++validPairs;
    }

    // Check if we have any valid pairs
    if (validPairs == 0) {
        std::cout<<"validPairs == 0"<<std::endl;
        return 0.0;  // No valid comparison possible
    }

    // Avoid division by zero
    if (normA == 0.0 || normB == 0.0) {
        std::cout<<"norma or normb == 0"<<std::endl;
        return 0.0;
    }

    float similarity = dotProduct / (std::sqrt(normA) * std::sqrt(normB));

    // Handle potential floating-point inaccuracies
    // if (similarity > 1.0) {
    //     similarity = 1.0;
    // } else if (similarity < -1.0) {
    //     similarity = -1.0;
    // }

    return 1-similarity;
}



float percentile(const std::vector<float>& data, float percentile_value) {
    // 检查输入数据
    if (data.empty()) {
        throw std::invalid_argument("Input vector is empty");
    }
    
    if (percentile_value < 0.0f || percentile_value > 100.0f) {
        throw std::invalid_argument("Percentile value must be between 0 and 100");
    }

    // 复制并排序数据
    std::vector<float> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end());

    // 计算百分位数的位置
    size_t n = sorted_data.size();
    float index = (percentile_value / 100.0f) * (n - 1);

    // 处理边界情况
    if (index <= 0) return sorted_data.front();
    if (index >= n - 1) return sorted_data.back();

    // 计算插值
    size_t lower_index = static_cast<size_t>(std::floor(index));
    size_t upper_index = static_cast<size_t>(std::ceil(index));
    float fraction = index - static_cast<float>(lower_index);

    return sorted_data[lower_index] + fraction * (sorted_data[upper_index] - sorted_data[lower_index]);
}

std::vector<std::pair<std::string, std::vector<std::string>>> CTextChunkHelper::mergeShortTexts(const std::vector<std::pair<std::string, std::vector<std::string>>>& processed_splited_text) {
    
    std::vector<std::pair<std::string, std::vector<std::string>>> result;

    for (const auto& file_pair : processed_splited_text) {
        std::string filename = file_pair.first;
        std::vector<std::string> merged_texts;
        const auto& texts = file_pair.second;

        for (size_t i = 0; i < texts.size(); /* no increment here */) {
            std::string current_text = texts[i];

            // Keep merging until the text is long enough or we run out of texts to merge
            while (current_text.length() < this->min_text_length && i < texts.size() - 1) {
                ++i;
                current_text += texts[i];
            }

            merged_texts.push_back(current_text);
            ++i;
        }

        result.emplace_back(filename, merged_texts);
    }

    return result;
}


std::vector<std::pair<std::string, std::vector<std::string>>> CTextChunkHelper::_merge_texts(std::vector<std::pair<std::string, std::vector<std::string>>> splited_text,const char* model_path) {
    
    std::vector<std::pair<std::string, std::vector<std::string>>> processed_splited_text;
    processed_splited_text.reserve(2000);

    std::vector<std::pair<std::string, std::vector<std::unordered_map<std::string, std::string>>>> unpossed_texts;

    std::vector<const char *> all_combin_texts;
    std::vector<std::string> all_combin_texts_strs;
    all_combin_texts.reserve(8000);

    for (auto &file : splited_text) {  // 遍历每个文件
        std::vector<std::unordered_map<std::string, std::string>> pre_processec_texts;

        for (int i = 0; i < file.second.size(); i++) {
            std::unordered_map<std::string, std::string> store_combined_sentences;
            std::string temp = "";

            if (i == 0 && i + 1 < file.second.size()) {
                temp = file.second.at(i) + file.second.at(i + 1);
            } else if (i == file.second.size() - 1 && i > 0) {
                temp = file.second.at(i - 1) + file.second.at(i);
            } else if (i > 0 && i < file.second.size() - 1) {
                temp = file.second.at(i - 1) + file.second.at(i) + file.second.at(i + 1);
            } else {
                temp = file.second.at(i);
            }

            store_combined_sentences["sentence"] = file.second.at(i);
            store_combined_sentences["combined_sentence"] = temp;

            std::vector<std::string> lines = split_lines(temp);
            std::string ts = "";

            for (auto &line : lines) {
                ts += line;
            }

            all_combin_texts_strs.push_back(ts);
            pre_processec_texts.push_back(store_combined_sentences);
        }

        unpossed_texts.push_back(std::make_pair(file.first, pre_processec_texts));
    }

    for (auto &text : all_combin_texts_strs) {
        all_combin_texts.push_back(text.c_str());
    }
    
    std::vector<std::vector<float>> embedidng_vectors = this->ctextembedding.GetEmbedding(all_combin_texts,model_path);
    
    int file_index = 0;
    int index_start = 0;
    int last_file_length = 0;

    for (auto &file : unpossed_texts) {
        if (file.second.size() < 2) {
            std::vector<std::string> chunks;
            for (int i = 0; i < file.second.size(); i++) {
                chunks.push_back(file.second[i]["sentence"]);
            }
            processed_splited_text.push_back(std::make_pair(file.first, chunks));
            continue;
        }

        std::vector<float> distances;
        int file_range = file.second.size();

        for (int i = 0; i < file_range - 1; i++) {
            if (index_start + i + 1 >= embedidng_vectors.size()) {
                throw std::runtime_error("Embedding vector index out of range");
            }
            float dis = CosineSimilarityDistance(embedidng_vectors[index_start + i], embedidng_vectors[index_start + i + 1]);
            distances.push_back(dis);
        }

        float breakpoint_distance_threshold = percentile(distances, this->breakpoint_percentile_threshold);
        std::vector<int> indices_above_threshold;

        for (int i = 0; i < distances.size(); i++) {
            if (distances.at(i) > breakpoint_distance_threshold) {
                indices_above_threshold.push_back(i);
            }
        }

        std::vector<std::string> chunks;
        int start_merge_index = 0;

        for (auto &index : indices_above_threshold) {
            int end_merge_index = index;
            std::string combined_chunk = "";
            for (int i = start_merge_index; i <= end_merge_index; i++) {
                combined_chunk += file.second[i]["sentence"];
            }
            start_merge_index = end_merge_index + 1;
            chunks.push_back(combined_chunk);
        }

        if (start_merge_index < file.second.size()) {
            std::string combined_text = "";
            for (int i = start_merge_index; i < file.second.size(); i++) {
                combined_text += file.second[i]["sentence"];
            }
            chunks.push_back(combined_text);
        }

        processed_splited_text.push_back(std::make_pair(file.first, chunks));

        last_file_length += file_range;
        index_start += file_range;
    }

  
    return mergeShortTexts(processed_splited_text);
}


std::vector<std::pair<std::string, std::vector<std::string>>> CTextChunkHelper::merge_texts_multithread(std::vector<std::pair<std::string, std::vector<std::string>>> splited_text,const std::vector<const char*> &model_paths){
     
      
    const int num_threads = model_paths.size();
    size_t total_files = splited_text.size();
    size_t files_per_thread = (total_files + num_threads - 1) / num_threads;

    std::vector<std::future<std::pair<size_t, std::vector<std::pair<std::string, std::vector<std::string>>>>>> futures;
    
    for (int t = 0; t < num_threads; ++t) {
        futures.push_back(std::async(std::launch::async, [this,t, files_per_thread, &splited_text, &model_paths, total_files]()->std::pair<size_t, std::vector<std::pair<std::string, std::vector<std::string>>>> {
            size_t start_index = t * files_per_thread;
            size_t end_index = std::min(start_index + files_per_thread, total_files);

            std::vector<std::pair<std::string, std::vector<std::string>>> sub_splited_text(
                splited_text.begin() + start_index,
                splited_text.begin() + end_index
            );
            auto result = this->_merge_texts(sub_splited_text, model_paths[t]);
            return std::make_pair(start_index, result);
        }));
    }

    std::vector<std::pair<std::string, std::vector<std::string>>> merged_texts(total_files);
    for (auto &fut : futures) {
        auto result = fut.get();
        size_t start_index = result.first;
        auto &sub_result = result.second;

        std::copy(sub_result.begin(), sub_result.end(), merged_texts.begin() + start_index);
    }

    return merged_texts;      
        
}