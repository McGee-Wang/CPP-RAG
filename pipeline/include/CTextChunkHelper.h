#include <mutex>
#include<vector>
#include<string>
#include <utility>
#include "CTextEmbedding.h"


//std::mutex embedding_mutex;
class CTextChunkHelper{
       
        private:
                
                CTextEmbedding ctextembedding;
                int min_text_length = 50;
                float breakpoint_percentile_threshold = 25;
                


        public:

                CTextChunkHelper(){
                         
                         
                         min_text_length = 0;
                         breakpoint_percentile_threshold = 0;
                }

                CTextChunkHelper(CTextEmbedding cte,int mtl,float bpt){
                      
                       this->ctextembedding = cte; //
                       this->min_text_length = mtl;
                       this->breakpoint_percentile_threshold = bpt;
     
                }

                CTextChunkHelper(CTextEmbedding ctextembedding){
                      
                       this->ctextembedding = ctextembedding;
                }
                
                

                void setMinTextLength(int length){
                     
                      this->min_text_length = length;
                }

                void setBreakpointPercentileThreshold(float bpt){
                     this->breakpoint_percentile_threshold = bpt;
                }

               int getMinTextLength(){return this->min_text_length;}
               float getBpt(){return this->breakpoint_percentile_threshold;}
                
               CTextEmbedding getCtextEmbedding(){return this->ctextembedding;}

               std::vector<std::pair<std::string, std::vector<std::string>>> mergeShortTexts(const std::vector<std::pair<std::string, std::vector<std::string>>>& processed_splited_text);
               std::vector<std::pair<std::string, std::vector<std::string>>> _merge_texts(std::vector<std::pair<std::string, std::vector<std::string>>> splited_text,const char* model_path); 
               std::vector<std::pair<std::string, std::vector<std::string>>> merge_texts_multithread(std::vector<std::pair<std::string, std::vector<std::string>>> splited_text,const std::vector<const char*> &model_paths);

               
               



};