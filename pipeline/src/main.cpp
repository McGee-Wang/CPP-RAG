
#include "../include/CPipeline.h"
#include<iostream>
#include<vector>
#include<string>


int main(){

    //
    CPipeline cp(false,false);
   
    // your config file path
    std::string config = "C:\\wmq\\debug_projects\\debug_text_simi_cuda_with_merge\\src\\pipeline\\src\\config.txt";
    cp.pipeline(config,2,6);
    return 1;

  
   
}

