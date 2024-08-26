#include "../include/CMyLLMImpl.h"
#include <iostream>
#include <Windows.h>

CMyLLMImpl::CMyLLMImpl() : layers(0), modelPath(""),template_path(""),prefix(""),suffix("") {}

CMyLLMImpl::~CMyLLMImpl() {}


void CMyLLMImpl::setLayers(int numLayers) {
    layers = numLayers;
    std::cout << "Number of layers set to: " << layers << std::endl;
}

void CMyLLMImpl::loadModel(std::string modelPath) {
    this->modelPath = modelPath;

    std::cout << "Model loaded from: " << modelPath << std::endl;
    // 实际加载模型的逻辑
}
void CMyLLMImpl::setchattemplate(std::string template_path){
     
      this->template_path = template_path;
      
}
void CMyLLMImpl::setprefixandsuffix(std::string prefix,std::string suffix){
       
        this->prefix = prefix;
        this->suffix = suffix;
}

// virtual void process(std::string prompt,std::string &output,std::string ddl_name,int topk,std::string embed_path,std::string tablename, SearchTopKFuncPtr search_topk,EmbeddingFuncPtr embedding)
void CMyLLMImpl::process(std::string &output,std::string dll_name,std::string db_path,std::string tablename,std::string embedding_path,std::string embedding_dllname,int topk,int batch,int embedding_vectors_size,int embed_layers,RagFunc rag) {
      
    const char *result;
    HMODULE hDll2 = LoadLibrary(dll_name.c_str());
    if(hDll2 == NULL){
          
          std::cerr <<"无法加载llm-DLL. 错误码： "<<GetLastError()<<std::endl;
          return ;
    }

    Llm_Inference inference = (Llm_Inference)GetProcAddress(hDll2,"llm_inference");
    if(inference == NULL){
         std::cerr<<"无法找到llm_inference. 错误码： "<<GetLastError()<<std::endl;
         FreeLibrary(hDll2);
         return ;
    }
     
    
    int final_res = inference(this->layers,this->modelPath.c_str(),this->template_path.c_str(),this->prefix.c_str(),this->suffix.c_str(),db_path.c_str(),tablename.c_str(),embedding_path.c_str(),embedding_dllname.c_str(),topk,batch,embedding_vectors_size,embed_layers,rag);
   
        
}

