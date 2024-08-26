#ifndef CMyLLMImpl_H
#define CMyLLMImpl_H

#include "CMyLLM.h"
#include <string>
#include <vector>
#include <tuple>
class CMyLLMImpl : public CMyLLM {
public:
    CMyLLMImpl();
    ~CMyLLMImpl();
    
    void setLayers(int numLayers) override;
    void loadModel(std::string modelPath) override;
    void setchattemplate(std::string template_path) override;
    void setprefixandsuffix(std::string prefix,std::string suffix) override;
    void process(std::string &output,std::string dll_name,std::string db_path,std::string tablename,std::string embedding_path,std::string embedding_dllname,int topk,int batch,int embedding_vectors_size,int embed_layers,RagFunc rag) override;

private:
    int layers;
    std::string modelPath;
    std::string template_path;
    std::string prefix;
    std::string suffix;
};

#endif // CMyLLMImpl_H