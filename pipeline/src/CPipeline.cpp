#include "../include/CPipeline.h"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <sstream>
#include <limits>
#include <fstream>
#include <thread>
#include <future>
#include <memory>
#include <tuple>
#include <chrono>
#include <cstdio>

namespace fs = std::filesystem;

bool readline_simple(std::string & line, bool multiline_input) {
#if defined(_WIN32)
        std::wstring wline;
        if (!std::getline(std::wcin, wline)) {
            // Input stream is bad or EOF received
            line.clear();
            GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
            return false;
        }

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wline[0], (int)wline.size(), NULL, 0, NULL, NULL);
        line.resize(size_needed);
        WideCharToMultiByte(CP_UTF8, 0, &wline[0], (int)wline.size(), &line[0], size_needed, NULL, NULL);
#else
        if (!std::getline(std::cin, line)) {
            // Input stream is bad or EOF received
            line.clear();
            return false;
        }
#endif
        if (!line.empty()) {
            char last = line.back();
            if (last == '/') { // Always return control on '/' symbol
                line.pop_back();
                return false;
            }
            if (last == '\\') { // '\\' changes the default action
                line.pop_back();
                multiline_input = !multiline_input;
            }
        }
        line += '\n';

        // By default, continue input if multiline_input is set
        return multiline_input;
 }


void reset_stdin() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //std::freopen("/dev/tty", "r", stdin); // For UNIX-like systems
     std::freopen("CON", "r", stdin); // For Windows systems
}


std::vector<std::string> get_all_files_in_directory(const std::string& directory_path) {
    std::vector<std::string> file_paths;

    try {
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (fs::is_regular_file(entry.status())) {
                file_paths.push_back(entry.path().string());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }

    return file_paths;
}

void CPipeline::bind_params(std::string config_path){
    std::string chunksize;
    std::string chunkoverlap;
    std::string files_path;
    std::string n_batch;
    std::string embedding_vectors_size;
    std::string embedding_layers;
    std::string embedding_models_dirctory_path;
    std::string min_text_length;
    std::string breakpoint_percentile_threshold;
    std::string database_path;
    std::string table_name;
    std::string llm_layers;
    std::string llm_path;
    std::string chat_template_path;
    std::string prefix;
    std::string suffix;

    Config configSettings(config_path);

    chunksize = configSettings.Read("chunksize",chunksize);
    chunkoverlap = configSettings.Read("chunkoverlap",chunkoverlap);
    files_path = configSettings.Read("files_path",files_path);
    n_batch = configSettings.Read("n_batch",n_batch);
    embedding_vectors_size = configSettings.Read("embedding_vectors_size",embedding_vectors_size);
    embedding_layers = configSettings.Read("embedding_layers",embedding_layers);
    embedding_models_dirctory_path = configSettings.Read("embedding_models_directory_path",embedding_models_dirctory_path);
    min_text_length = configSettings.Read("min_text_length",min_text_length);
    breakpoint_percentile_threshold = configSettings.Read("breakpoint_percentile_threshold",breakpoint_percentile_threshold);
    database_path = configSettings.Read("database_path",database_path);
    table_name = configSettings.Read("table_name",table_name);
    llm_layers = configSettings.Read("llm_layers",llm_layers);
    llm_path = configSettings.Read("llm_path",llm_path);
    chat_template_path = configSettings.Read("chat_template_path",chat_template_path);
    prefix = configSettings.Read("prefix",prefix);
    suffix = configSettings.Read("suffix",suffix);
    
    this->params["chunksize"] = chunksize;
    this->params["chunkoverlap"] = chunkoverlap;
    this->params["files_path"] = files_path;
    this->params["n_batch"] = n_batch;
    this->params["embedding_vectors_size"] = embedding_vectors_size;
    this->params["embedding_layers"] = embedding_layers;
    this->params["embedding_models_directory_path"] = embedding_models_dirctory_path;
    this->params["min_text_length"] = min_text_length;
    this->params["breakpoint_percentile_threshold"] = breakpoint_percentile_threshold;
    this->params["database_path"] = database_path;
    this->params["table_name"] = table_name;
    this->params["llm_layers"] = llm_layers;
    this->params["llm_path"] = llm_path;
    this->params["chat_template_path"] = chat_template_path;
    this->params["prefix"] = prefix;
    this->params["suffix"] = suffix;
       
}



void CPipeline::init(std::string config,int thread_nums){
   
   

     
    this->bind_params(config);
    

   
    bool found;

    std::vector<std::string> separator;
    CRecursiveCharTextSplitter ts(separator,true,true, std::stoi(this->get_param("chunksize",found)), std::stoi(this->get_param("chunkoverlap",found)));
    std::cout<<"1"<<std::endl;
    std::vector<std::pair<std::string, std::vector<std::string>>> splited_texts = ts.files_to_splited_texts(this->get_param("files_path",found));
    std::cout<<"2"<<std::endl;
    
    int count = 0;
    for(auto &file:splited_texts){
          
           for(auto &t:file.second){
               
                count++;
           }
    } 
    std::cout<<"total texts:"<<count<<std::endl;

    std::cout<<splited_texts.size()<<std::endl;
    CTextEmbedding ce("llama-embedding.dll",static_cast<int32_t>(std::stoi(this->get_param("n_batch",found))),std::stoi(this->get_param("embedding_vectors_size",found)),std::stoi(this->get_param("embedding_layers",found)));
    std::cout<<"3"<<std::endl;
    std::vector<std::string> embedding_models = get_all_files_in_directory(this->get_param("embedding_models_directory_path",found));


    std::vector<const char*> thread_embedding_models;

    for(int i=0;i<thread_nums;i++){
          
          thread_embedding_models.push_back(embedding_models.at(i).c_str());
    }
     
    std::vector<std::pair<std::string, std::vector<std::string>>> processed_splited_texts;
    processed_splited_texts.reserve(8000);

    if(this->is_merge==true){
          CTextChunkHelper ctc(ce,std::stoi(this->get_param("min_text_length",found)),std::stof(this->get_param("breakpoint_percentile_threshold",found)));
          processed_splited_texts = ctc.merge_texts_multithread(splited_texts,thread_embedding_models);

           
    }else{
          
          processed_splited_texts.assign(splited_texts.begin(),splited_texts.end());
    } 
    
    std::vector<std::vector<float>> merge_vectors = ce.embedding_multithread(processed_splited_texts,thread_embedding_models);
    
    int totol = merge_vectors.size();
    std::vector<std::vector<std::string>> data;
    data.reserve(totol);

     for(auto &file:processed_splited_texts){
         
           for(const auto &text:file.second){
              std::vector<std::string> temp;
              temp.reserve(2);
              temp.push_back(file.first);
              temp.push_back(text);
              data.push_back(temp);

           }
     }
     Mydb dbop(this->get_param("database_path",found),this->get_param("table_name",found)); 
     bool rr = dbop.insert(data,merge_vectors);
     if(!rr){
           std::cerr<<"插入数据失败！"<<std::endl;
           return ;
     }
     
     this->set_Is_init(true);
     
 }



void print_cin_status() {
    std::cout << "std::cin 状态标志：" << std::endl;

    if (std::wcin.good()) {
        std::cout << "流状态正常。" << std::endl;
    } else if (std::wcin.fail()) {
        std::cout << "流出现了故障。" << std::endl;
    } else if (std::wcin.eof()) {
        std::cout << "流已经到达文件末尾。" << std::endl;
    } else if (std::wcin.bad()) {
        std::cout << "流处于严重错误状态。" << std::endl;
    }

    // 打印更多详细信息
    std::cout << "failbit: " << std::wcin.fail() << std::endl;
    std::cout << "eofbit: " << std::wcin.eof() << std::endl;
    std::cout << "badbit: " << std::wcin.bad() << std::endl;
}
std::string wstring_to_string(const std::wstring& wstr) {
    std::string str;
    for (wchar_t wc : wstr) {
        // 注意：这只是一个示例，可能无法处理所有Unicode字符
        str.push_back(static_cast<char>(wc));
    }
    return str;
}


const char* user_input_preprocesse(const char* c_userinput,const char* c_db_path,const char* c_tablename,const char* embedding_path,const char* embedding_dllname,int k,int batch,int embedding_vectors_size, int layers){
        
       std::string userinput(c_userinput);
       std::cout<<"user input: "<<userinput<<std::endl;
       std::string db_path(c_db_path);
       std::string tablename(c_tablename);

        HMODULE hDLL = LoadLibrary(embedding_dllname);
            if (hDLL == NULL) {
                std::cerr << "无法加载 embeddingDLL. 错误码: " << GetLastError() << std::endl;
                return NULL;
            }

            // 获取函数指针
          EmbeddingFunc embedding = (EmbeddingFunc)GetProcAddress(hDLL, "embedding");
            if (embedding == NULL) {
                std::cerr << "无法找到embedding函数. 错误码: " << GetLastError() << std::endl;
                FreeLibrary(hDLL);
                return NULL;
            }


       // embedding
       std::vector<const char*> total_texts;
       total_texts.push_back(userinput.c_str());
       const char **no_embedding_texts = total_texts.data();
       int res_size;
       int vector_size;
       float **texts_embedding_vectors;

       int res = embedding(no_embedding_texts,1,&texts_embedding_vectors,&res_size,&vector_size,embedding_path,batch,embedding_vectors_size,layers);

       if (res != 1) {
                    for (int i = 0; i < res_size; i++) {
                        delete[] texts_embedding_vectors[i];
                    }
                    delete[] texts_embedding_vectors;

                    std::cerr << "embedding error!" << std::endl;
                    return {};
        }

        std::vector<std::vector<float>> embedidng_vectors;
        for (int i = 0; i < res_size; i++) {
                std::vector<float> embedding_vector;
            for (int j = 0; j < vector_size; j++) {
                    embedding_vector.push_back(texts_embedding_vectors[i][j]);
            }
            embedidng_vectors.push_back(embedding_vector);
        }
        
        //link sqlite
        sqlite3 *db;
        int rc = sqlite3_open(db_path.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::string errmess(sqlite3_errmsg(db));
            throw std::runtime_error("Can't open database: " + errmess);
        }


        char *errMsg = 0;

         // 注册余弦相似度函数
        sqlite3_create_function(db, "cosine_similarity", 2, SQLITE_UTF8, nullptr, sqliteCosineSimilarity, nullptr, nullptr);


        std::vector<char> queryblob;

        vectorToBlob(embedidng_vectors.at(0),queryblob);

        std::string str_querySQL = "SELECT filename, splited_text, cosine_similarity(data, ?) as similarity "
                           "FROM "+tablename+" "
                           "ORDER BY similarity DESC "
                           "LIMIT ?;";

        // const char* querySQL = "SELECT filename, splited_text, cosine_similarity(data, ?) as similarity "
        //                    "FROM "+tablename+" "
        //                    "ORDER BY similarity DESC "
        //                    "LIMIT ?;";
        const char* querySQL = str_querySQL.c_str();
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
            std::cout<<similarity<<std::endl;
          
                std::string filename_str = (filename != nullptr) ? filename : "";
                std::string splited_text_str = (splited_text!=nullptr)?splited_text:"";
                search_res.push_back(std::make_tuple(filename_str,splited_text_str,similarity));

        
            
        }

        std::string prompt = "给定知识：\n";
        int count = 1;
        for(auto &item:search_res){
             
              prompt = prompt +std::to_string(count)+". "+std::get<0>(item)+": "+ std::get<1>(item)+"\n";
              count++;
        }
        
        prompt  = prompt + "结合上述知识回答："+userinput;
         
        char *ans = new char[prompt.length()+1];
        strcpy(ans,prompt.c_str());
      
        return ans;
        
}





void CPipeline::pipeline(std::string config,int thread_nums,int top_k){
     double duration_second;
    
    if(!this->is_init){
         auto chunk_beforeTime  = std::chrono::steady_clock::now();
         this->init(config,thread_nums);
         auto chunk_afterTime  = std::chrono::steady_clock::now();
         duration_second = std::chrono::duration<double>(chunk_afterTime-chunk_beforeTime).count();
          
    }else{
         
         this->bind_params(config);
    }
   //  std::cout<<"退出系统"<<std::endl;
    const std::string n_batch = "n_batch";
    const std::string embedding_vectors_size = "embedding_vectors_size";
    const std::string embedding_layers = "embedding_layers";
    const std::string embedding_models_dirctory_path = "embedding_models_directory_path";
    const std::string database_path = "database_path";
    const std::string table_name = "table_name";
    const std::string llm_layers = "llm_layers";
    const std::string llm_path = "llm_path";
    const std::string chat_template_path = "chat_template_path";
    const std::string prefix = "prefix";
    const std::string suffix = "suffix";
    bool found;

     CTextEmbedding ce("llama-embedding.dll",static_cast<int32_t>(std::stoi(this->get_param(n_batch,found))),std::stoi(this->get_param(embedding_vectors_size,found)),std::stoi(this->get_param(embedding_layers,found)));
     std::vector<std::string> embedding_models = get_all_files_in_directory(this->get_param(embedding_models_dirctory_path,found));
     std::vector<const char*> thread_embedding_models;
     thread_embedding_models.push_back(embedding_models.at(0).c_str());

     Mydb dbop(this->get_param(database_path,found),this->get_param(table_name,found));


      auto llm = CMyLLMFactory::createLLM();
     // auto llm_unique = CMyLLMFactory::createLLM();  // 使用工厂方法创建LLM对象，返回unique_ptr
      //auto llm = std::shared_ptr<CMyLLM>(std::move(llm_unique));  // 将unique_ptr转换为shared_ptr

      llm->setLayers(std::stoi(this->get_param(llm_layers,found)));
      llm->loadModel(this->get_param(llm_path,found));
      llm->setchattemplate(this->get_param(chat_template_path,found));
      llm->setprefixandsuffix(this->get_param(prefix,found),this->get_param(suffix,found));
   
      std::string answers;
   
  
      RagFunc rf = user_input_preprocesse;
      if(!this->is_init){
        std::cout<<"嵌入+存入数据库总耗时："<<duration_second<<std::endl;
      }

      llm->process(answers,"llama-cli.dll",this->get_param(database_path,found),this->get_param(table_name,found),thread_embedding_models.at(0),"llama-embedding.dll",top_k,8192,256,10,rf);
   
    
      std::cout<<"退出系统"<<std::endl;
   
      
}