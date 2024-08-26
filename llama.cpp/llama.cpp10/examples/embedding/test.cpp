#include<iostream>


int main(int argc, char **arg){

      std::cout<<"argc: "<<argc<<std::endl;

      for(int i=0;i<argc;i++){
         std::cout<<"arg["<<i<<"] = "<<arg[i]<<std::endl;
      }

      bool f = arg[argc]==nullptr;
      std::cout<<f<<std::endl;

      return 0;
}

//./llama-embedding-vul -m C:\wmq\projects\helloworld\projects\text_similarity_search4.0\embedding_models\bge-base-zh-v1.5-q8_0.gguf -e -p "hello world" --verbose-prompt -ngl 99  