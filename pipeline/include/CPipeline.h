#include "CMyLLMFactory.h"
#include "CRecursiveCharTextSplitter.h"
#include "CTextChunkHelper.h"
#include "Mydb.h"
#include "Config.h"
#include <string>
#include <vector>
#include <unordered_map>


class CPipeline{

    
    private:
      
      bool is_merge;
      bool is_init;
      std::unordered_map<std::string,std::string> params;
      
    
    public:

    CPipeline(){
        this->is_init = false;
        this->is_merge = false;
    }
    CPipeline(bool is_merge,bool is_init){
          
           this->is_init = is_init;
           this->is_merge = is_merge;
    }

    //传递文件夹
    
    void init(std::string config,int thread_nums);
    void pipeline(std::string config,int thread_nums,int top_k);
    void bind_params(std::string config_path);
    void set_Is_merge(bool is_merge){this->is_merge = is_merge;}
    void set_Is_init(bool is_init){this->is_init;}



     // Get方法，返回参数值
    std::string get_param(const std::string& key, bool &found) const {
        auto it = params.find(key);
        if (it != params.end()) {
            found = true;
            return it->second;
        } else {
            found = false;
            return ""; // 或者返回一个默认值
        }
    }
     

};
