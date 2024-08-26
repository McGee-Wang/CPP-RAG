//#ifndef TEXTSPLITER_H
//#define TEXTSPLITER_H
#include<string>
#include<vector>

class CRecursiveCharTextSplitter{

    private:
    std::vector<std::string> separators;
    bool keep_separator;
    bool is_separator_regex;
    int chunksize;
    int chunkoverlap;

    public:

    CRecursiveCharTextSplitter();
    CRecursiveCharTextSplitter(std::vector<std::string> separators,bool keep_separator,bool is_separator_regex,int chunksize,int chunkoverlap);
    std::vector<std::string> _split_text(std::string text,std::vector<std::string> separators,bool _keep_separater,int chunksize,int chunkoverlap);
    

    //重载 = 
    CRecursiveCharTextSplitter &operator=(const CRecursiveCharTextSplitter& spliter){
               
                
                this->separators = spliter.separators;
                this->keep_separator = spliter.keep_separator;
                this->chunksize = spliter.chunksize;
                this->chunkoverlap = spliter.chunkoverlap;
               

                return *this;
    }
    
    std::vector<std::pair<std::string,std::vector<std::string>>> files_to_splited_texts(std::string file_paths);
    //get set 
    std::vector<std::string> GetSeparators(){return this->separators;}
    void SetSeparators(std::vector<std::string> separators){this->separators=separators;}

    bool GetKeep_separator(){return this->keep_separator;}
    void SetKeep_separator(bool keep_separator){this->keep_separator=keep_separator;}

    bool GetIsSeparatorRegex(){return this->is_separator_regex;}
    void SetIsSetparatorRegex(bool is_separator_regex){this->is_separator_regex=is_separator_regex;}

    int GetChunkSize(){return this->chunksize;}
    void SetChunkSize(int chunksize){this->chunksize = chunksize;};

    int GetChunkOverlap(){return this->chunkoverlap;}
    void SetChunkOverlap(int chunkoverlap){this->chunkoverlap = chunkoverlap;}


};





//#endif
