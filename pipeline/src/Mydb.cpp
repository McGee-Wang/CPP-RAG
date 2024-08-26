#include "../include/Mydb.h"
#include <stdexcept>
#include <sstream>
#include <queue> // 包含优先队列的头文件
#include <utility> // 包含 std::pair 的头文件
#include <functional> // 包含 std::greater 的头文件
#include <cassert>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <immintrin.h>

/*

输入：vector<string> knowledge_text, string target
分词： vector<string> knowledge - > vector<pair<sting,vector<string>>> splited_knowledge  vector里面装的说每个文件下被分割的知识
嵌入： vector<pair<string,vector<string>>> splited_knowledge -> vector<vector<string>> embedding 内层的是一个3容量的vector 0:文件名 1：原始文本：3：分割的部分文本  外层的大小等于所有知识被分割段落的大小
做一个转换：vector<vector<string>> embedding - > vector<pair<vector<string>,vector<float>>> trans_embedding  由一个容量为2的vector和一个float组成 

存入数据库： 表视图： （文件名（text），原始文本（text），向量（blob））


搜索：
1.1 string target -> vector<float> embed_target
1.2 数据库中所有数据存入index中 (key,value) key表示在数据库中的索引， value 是blob->vector<float> (.data方法)
1.3 根据余弦相似度搜索topk 输入 enbed_target ,k  返回result result是最相似的top个文本

1.4 result.key 是在数据库中的下表  
    返回召回文本：取数据库result.key行数据的第二列
    返回文件名：取数据库result.key行数据的第一列
    返回相似文本在文本中的偏移：找到该文件名出现的最早索引first，然后 result.key-first 然后计算前面所有的字符个数 得出在该文件中的便宜



相似文本检索？ 你这个target有可能也是长的啊
你这个target有可能也要分词啊
还需要一个target处理

*/




/*
 blob 数据转vector
*/
// static void blobToVector(const unsigned char* blobData, int blobSize, std::vector<float>& floatVector) {
//     floatVector.clear();
//     if (blobSize % sizeof(float) != 0) {
//         std::cerr << "Blob data size does not match float size." << std::endl;
//         return;
//     }
    
//     int numFloats = blobSize / sizeof(float);
//     const float* floatArray = reinterpret_cast<const float*>(blobData);
//     floatVector.assign(floatArray, floatArray + numFloats);
// }

// /*
// vector 转 blob
// */
// static void vectorToBlob(const std::vector<float>& vec, std::vector<unsigned char>& blobData) {
//     blobData.clear();
//     blobData.resize(vec.size() * sizeof(float));
//     std::memcpy(blobData.data(), vec.data(), blobData.size());
// }

// 辅助函数：将 vector<float> 转换为 BLOB
void vectorToBlob(const std::vector<float>& vec, std::vector<char>& blob) {
    blob.resize(vec.size() * sizeof(float));
    std::memcpy(blob.data(), vec.data(), blob.size());
}

// 辅助函数：将 BLOB 转换回 vector<float>
std::vector<float> blobToVector(const void* blob, int size) {
    std::vector<float> vec(size / sizeof(float));
    std::memcpy(vec.data(), blob, size);
    return vec;
}

//
bool Mydb::insert(std::vector<std::vector<std::string>> data,std::vector<std::vector<float>> embed_vectors) {  //都是一一对应的
    // 准备sql语句
    sqlite3_stmt *stmt = nullptr;
    std::string sql_insert = "INSERT INTO " + this->Get_target_table() + " (filename, splited_text, data) VALUES (?, ?, ?);";
    std::cout<<sql_insert<<std::endl;

   // int rc = sqlite3_prepare_v2(Mydb::Get_db(), sql_insert.c_str(), -1, &stmt, nullptr);
   int rc = sqlite3_prepare_v2(Mydb::Get_db(), sql_insert.c_str(), sql_insert.size(), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(Mydb::Get_db()) << std::endl;
        return false;
    }
    std::cout<<"1"<<std::endl;
    // 开始事务
    rc = sqlite3_exec(Mydb::Get_db(), "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to begin transaction: " << sqlite3_errmsg(Mydb::Get_db()) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
     std::cout<<"2"<<std::endl;

     
     for(int i=0;i<data.size();i++){
         
          sqlite3_bind_text(stmt, 1, data.at(i).at(0).c_str(), -1, SQLITE_STATIC); //文件名
          sqlite3_bind_text(stmt, 2, data.at(i).at(1).c_str(), -1, SQLITE_STATIC);  //原文

          std::vector<char> blobData;
          vectorToBlob(embed_vectors.at(i),blobData);
          sqlite3_bind_blob(stmt, 3, blobData.data(), blobData.size(), SQLITE_STATIC);

          rc = sqlite3_step(stmt);
          if (rc != SQLITE_DONE) {
             std::cerr << "Error executing statement: " << sqlite3_errmsg(Mydb::Get_db()) << std::endl;
             sqlite3_exec(Mydb::Get_db(), "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr); // 回滚事务
            sqlite3_finalize(stmt);
             return false;
          }
          //     // 重置语句，以便下一次使用
         sqlite3_reset(stmt);
         sqlite3_clear_bindings(stmt);
          
     }


    std::cout<<"3"<<std::endl;
    // 提交事务
    rc = sqlite3_exec(Mydb::Get_db(), "COMMIT TRANSACTION;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Commit failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }
    std::cout<<"4"<<std::endl;
    sqlite3_finalize(stmt);
    return true;
}




float cosineSimilarity_for_sqlite(const std::vector<float>& v1, const std::vector<float>& v2) {
    float dot = 0.0, mag1 = 0.0, mag2 = 0.0;
    for (size_t i = 0; i < v1.size(); ++i) {
        dot += v1[i] * v2[i];
        mag1 += v1[i] * v1[i];
        mag2 += v2[i] * v2[i];
    }
    mag1 = std::sqrt(mag1);
    mag2 = std::sqrt(mag2);
    return (mag1 * mag2 == 0) ? 0 : dot / (mag1 * mag2);
}



float cosineSimilarity_for_sqlite_simd(const std::vector<float>& v1, const std::vector<float>& v2) {
    __m128 sum_dot = _mm_setzero_ps();
    __m128 sum_sq1 = _mm_setzero_ps();
    __m128 sum_sq2 = _mm_setzero_ps();

    size_t i = 0;
    size_t size = v1.size();
    size_t simd_size = size - (size % 4);

    for (; i < simd_size; i += 4) {
        __m128 x = _mm_loadu_ps(&v1[i]);
        __m128 y = _mm_loadu_ps(&v2[i]);
        
        sum_dot = _mm_add_ps(sum_dot, _mm_mul_ps(x, y));
        sum_sq1 = _mm_add_ps(sum_sq1, _mm_mul_ps(x, x));
        sum_sq2 = _mm_add_ps(sum_sq2, _mm_mul_ps(y, y));
    }

    float dot = _mm_cvtss_f32(_mm_hadd_ps(_mm_hadd_ps(sum_dot, sum_dot), sum_dot));
    float mag1 = _mm_cvtss_f32(_mm_hadd_ps(_mm_hadd_ps(sum_sq1, sum_sq1), sum_sq1));
    float mag2 = _mm_cvtss_f32(_mm_hadd_ps(_mm_hadd_ps(sum_sq2, sum_sq2), sum_sq2));

    // Handle remaining elements
    for (; i < size; ++i) {
        dot += v1[i] * v2[i];
        mag1 += v1[i] * v1[i];
        mag2 += v2[i] * v2[i];
    }

    mag1 = std::sqrt(mag1);
    mag2 = std::sqrt(mag2);

    return (mag1 * mag2 == 0) ? 0 : dot / (mag1 * mag2);
}



void sqliteCosineSimilarity(sqlite3_context* context, int argc, sqlite3_value** argv) {
    if (argc != 2) {
        sqlite3_result_error(context, "Cosine similarity requires two arguments", -1);
        return;
    }

    const void* blob1 = sqlite3_value_blob(argv[0]);
    int size1 = sqlite3_value_bytes(argv[0]);
    const void* blob2 = sqlite3_value_blob(argv[1]);
    int size2 = sqlite3_value_bytes(argv[1]);

    if (size1 != size2) {
        sqlite3_result_error(context, "Vector dimensions do not match", -1);
        return;
    }

    std::vector<float> v1 = blobToVector(blob1, size1);
    std::vector<float> v2 = blobToVector(blob2, size2);

    float similarity = cosineSimilarity_for_sqlite_simd(v1, v2);
    sqlite3_result_double(context, similarity);
}


std::vector<std::tuple<std::string,std::string,double>> Mydb::_search_top_k(std::string table_name,std::vector<float> target,int k){
       
        sqlite3 *db = this->db;
        char *errMsg = 0;
        int rc;

        // 注册余弦相似度函数
        sqlite3_create_function(db, "cosine_similarity", 2, SQLITE_UTF8, nullptr, sqliteCosineSimilarity, nullptr, nullptr);


        std::vector<char> queryblob;

        vectorToBlob(target,queryblob);


        const char* querySQL = "SELECT filename, splited_text, cosine_similarity(data, ?) as similarity "
                           "FROM testtable "
                           "ORDER BY similarity DESC "
                           "LIMIT ?;";
        sqlite3_stmt* stmt;
        rc = sqlite3_prepare_v2(db, querySQL, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
           std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
           return {};
        }

        rc = sqlite3_bind_blob(stmt, 1, queryblob.data(), queryblob.size(), SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding vector parameter: " << sqlite3_errmsg(db) << std::endl;
            return {};
        }
        
        rc = sqlite3_bind_int(stmt,2,k);

        if (rc != SQLITE_OK) {
            std::cerr << "Error binding LIMIT parameter: " << sqlite3_errmsg(db) << std::endl;
            return {};
        }
        

        std::vector<std::tuple<std::string,std::string,double>> search_res;

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            const char* filename = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* splited_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            double similarity = sqlite3_column_double(stmt, 2);
            std::string filename_str = (filename != nullptr) ? filename : "";
            std::string splited_text_str = (splited_text!=nullptr)?splited_text:"";
            search_res.push_back(std::make_tuple(filename_str,splited_text_str,similarity));
        }
         
        return search_res;
}