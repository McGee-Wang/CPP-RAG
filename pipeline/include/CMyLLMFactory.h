#ifndef CMyLLMFactory_H
#define CMyLLMFactory_H

#include "CMyLLM.h"
#include <string>
#include <memory>

class CMyLLMFactory {
public:
    static std::unique_ptr<CMyLLM> createLLM();
};

#endif // CMyLLMFactory_H