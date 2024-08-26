#include<sqlite3.h>
#include<stdio.h>
#include<iostream>
#include<cstring>
#include<string>
#include<vector>
#include<utility>
#include<tuple>



//回调函数
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

/*
 表里应该存3列
 第一列： 文件名
 第二列： 原文本
 第三列： 嵌入向量
*/

class Mydb{
     
     private:

     std::string db_path;
     std::string target_table;
     sqlite3 *db;
    
     //metric_punned_t metric(this->dim, metric_kind_t::cos_k, scalar_kind_t::f32_k);
     //index_dense_t index;
     

     public:
     
     // 打开并链接db  并且创建表
     Mydb(std::string db_path,std::string target_table){
       /* this->dim = dim;
         int rc;
         this->db_path = db_path;
         this->target_table = target_table;


         rc = sqlite3_open(db_path.c_str(),&this->db);
         char *zErrMsg = 0;
         if(rc){
            std::string errmess(sqlite3_errmsg(db));
            throw std::runtime_error("Can't open database: "+errmess);
            // 会自动调用析构函数
         }

         std::string sql_create_table= "CREATE TABLE IF NOT EXISTS " + target_table +  " (filename TEXT,splited_text, data BLOB);";
         rc = sqlite3_exec(this->db, sql_create_table.c_str(), 0, 0, &zErrMsg);
         if(rc!=SQLITE_OK){
             
             std::cerr << "SQL error: " << zErrMsg << std::endl;
             sqlite3_free(zErrMsg);
             throw std::runtime_error("crate table error!");   
          }*/
         this->db_path = db_path;
         this->target_table = target_table;
         
         int rc = sqlite3_open(db_path.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::string errmess(sqlite3_errmsg(db));
            throw std::runtime_error("Can't open database: " + errmess);
        }

        char *zErrMsg = nullptr;
        rc = sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to begin transaction: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
            sqlite3_close(db);
            throw std::runtime_error("Failed to begin transaction!");
        }

        std::string sql_create_table = "CREATE TABLE IF NOT EXISTS " + target_table + " (filename TEXT, splited_text TEXT, data BLOB);";
        rc = sqlite3_exec(db, sql_create_table.c_str(), nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
            sqlite3_close(db);
            throw std::runtime_error("Create table error!");
        }

        rc = sqlite3_exec(db, "COMMIT", nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to commit transaction: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
            sqlite3_close(db);
            throw std::runtime_error("Failed to commit transaction!");
        }


     }
     ~Mydb(){
        sqlite3_close(this->db);
     }
     
     
     std::vector<std::tuple<std::string,std::string,double>> _search_top_k(std::string table_name,std::vector<float> target,int k);
     //插入数据
   
     bool insert(std::vector<std::vector<std::string>> data,std::vector<std::vector<float>> embed_vectors);
     
     std::string Get_db_path(){return this->db_path;}
     void Set_db_path(std::string db_path){this->db_path = db_path;}
     
     sqlite3 * Get_db(){return this->db;}
     
     std::string Get_target_table(){return this->target_table;}
     void Set_target_table(std::string target_table){this->target_table = target_table;}
     

     
};

