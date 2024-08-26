#include "../include/CMyLLM.h"
#include "../include/CMyLLMFactory.h"
#include "../include/CMyLLMImpl.h"


std::unique_ptr<CMyLLM> CMyLLMFactory::createLLM() {
     
     return std::make_unique<CMyLLMImpl>();
}



