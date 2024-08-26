#ifndef CMyLLM_H
#define CMyLLM_H
#include<string>
#include<vector>
#include<tuple>
typedef const char* (*RagFunc)(const char* ,const char* ,const char* ,const char* ,const char* ,int ,int ,int , int);
typedef int (*Llm_Inference)(int,const char* ,const char* ,const char*,const char* ,const char*,const char*,const char*,const char*,int ,int,int,int,RagFunc);
 
class CMyLLM {
public:
    virtual ~CMyLLM() = default;
    virtual void setLayers(int numLayers) = 0;
    virtual void loadModel(std::string modelPath) = 0;
    virtual void setchattemplate(std::string template_path) =0;
    virtual void setprefixandsuffix(std::string prefix,std::string suffix) = 0;
    //int final_res = inference(13,llm_model_path,chat_temp_path,prefix,suffix,prompt.c_str(),&result);
    virtual void process(std::string &output,std::string dll_name,std::string db_path,std::string tablename,std::string embedding_path,std::string embedding_dllname,int topk,int batch,int embedding_vectors_size,int embed_layers,RagFunc rag)= 0;
};

#endif // CMyLLM_H