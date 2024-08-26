#include "../include/CRecursiveCharTextSplitter.h"
#include<regex>
#include<iterator>
#include<iostream>
#include<string>
#include<sstream>
#include<filesystem>
#include<utility>
#include<fstream>

namespace fs = std::filesystem;
// log 打印错误
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};


// 去除字符串两边空白字符
std::string strip(const std::string& str) {
    // 找到第一个和最后一个不是空白字符的位置
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");

    if (first == std::string::npos) {
        // 如果字符串全是空白字符，返回空字符串
        return "";
    } else {
        // 提取并返回去除空白字符后的子字符串
        return str.substr(first, last - first + 1);
    }
}

// log 信息
void logMessage(LogLevel level, const std::string& message) {
    switch (level) {
        case LogLevel::Debug:
            std::cout << "[DEBUG] " << message << std::endl;
            break;
        case LogLevel::Info:
            std::cout << "[INFO] " << message << std::endl;
            break;
        case LogLevel::Warning:
            std::cout << "[WARNING] " << message << std::endl;
            break;
        case LogLevel::Error:
            std::cerr << "[ERROR] " << message << std::endl;
            break;
        default:
            break;
    }
}

//拼接
 std::string join_docs(const std::vector<std::string>& docs, const std::string& separator, bool _strip_whitespace){
       
        if(docs.size()<=0){  //空列表
            return "";
        }
        std::vector<std::string> temp;

        for(auto &doc:docs){
            temp.push_back(doc);
            temp.push_back(separator);
        }
        temp.pop_back();
        std::string res;
        for(auto &t:temp){
            res += t;
        }
        if(_strip_whitespace){
           
            res = strip(res);
        }
        return res;

 }


//对分割过于小的块进行merge
std::vector<std::string> _merge_splits(std::vector<std::string> splits,std::string separator,int chunksize,int chunkoverlap){
     
    // std::cout<<"merge被调用"<<std::endl;

     int separator_len = separator.length();
     std::vector<std::string> docs;
     std::vector<std::string> current_docs; 
     int total = 0;

     for(auto &d:splits){

        int _len = d.length();
        //# TODO 达到这个条件，已经达到当前可拼接的极限，再拼要超chunk_size了，之前的句子会合并拼接

        /*
          current_docs应该记录的是当前拼接的splits，
          对于当前这个拼接d：长度计算为：之前拼接的长度+d的长度+分隔符长度
          如果current_docs为0 那么也就是说分隔符之前没有东西，不能加分隔符长度
        */
        int temp_len = current_docs.size()>0?separator_len:0;

        if(total+_len+temp_len>chunksize){  //超出了chunksize长度
             
             if(total>chunksize){
                logMessage(LogLevel::Warning, 
               "Created a chunk of size " + std::to_string(total) +
               ", which is longer than the specified " + std::to_string(chunksize));
             }

             if(current_docs.size()>0){  // 两种情况：1 本身+之前的>chunksize 2 之前的为0，本身d>chunksize
               // # TODO 将之前加起来长度不超过chunk_size的块拼接合并
                //std::cout<<"连接字符串第一次操作："<<separator<<std::endl;
                std::string doc = join_docs(current_docs,separator,true);  // 链接操作
               // std::cout<<"链接结果: "<<doc<<std::endl;
            
                if(!doc.empty()){
                    docs.push_back(doc);
                }
                int temp = current_docs.size()>0?separator_len:0;
                while(total>chunkoverlap || (total+_len+temp)>chunksize && total>0){
                    // # TODO 从左侧开始一个一个弹出子块，直到满足小于重叠大小，小于重叠大小的部分将保留在中间集合current_docs里面和后面的块继续组合拼接，这个重叠部分已经在上文的doc = self._join_docs(current_doc, separator)拼接合并过一次了，由于其保留在了中间集合中，因此未来还会拼接合并一次
                        int tl = current_docs.size()>1?separator_len:0;
                        total -= current_docs[0].length()+tl;
                        current_docs.erase(current_docs.begin());
            }


        }
        }
        current_docs.push_back(d);
        int t2 = current_docs.size()>1?separator_len:0;
        total += _len + t2;
        
     }
     //std::cout<<"链接字符串第二次操作："<<separator<<std::endl;
     std::string doc = join_docs(current_docs,separator,true);
    // std::cout<<"链接结果："<<doc<<std::endl;
     if(!doc.empty()){
          
          docs.push_back(doc);
     }
     
     return docs;       

}




// re.escape 对特殊字符加上转义字符  没问题
std::string escapeRegex(const std::string& str) {
    std::string escaped_str;
    for (char c : str) {
        // 转义正则表达式中的特殊字符
        if (c == '.' || c == '*' || c == '+' || c == '?' || c == '(' ||
            c == ')' || c == '[' || c == ']' || c == '{' || c == '}' ||
            c == '^' || c == '$' || c == '\\' || c == '|') {
            escaped_str.push_back('\\');  // 在特殊字符前加上反斜杠
        }
        escaped_str.push_back(c);
    }
    return escaped_str;
}



std::vector<std::string> extractMatches(const std::string& text, const std::string& pattern) {
    std::regex regex_pattern(pattern);
    std::vector<std::string> matches;

    // 使用 std::sregex_iterator 遍历匹配结果
    auto words_begin = std::sregex_iterator(text.begin(), text.end(), regex_pattern);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        matches.push_back(match.str());
    }

    return matches;
}


/*
说明：将文本按照分隔符分割
@parma： 
    text：被分割的文档
    separator：分隔符
    keep_separator: true 则结果保留分隔符，false 结果不保留分隔符

@return：
    分割后的文本集合

 */

std::vector<std::string> _split_text_with_regex_from_end(std::string text,std::string separator,bool keep_separator){
           
            std::vector<std::string> res;
            std::vector<std::string> splits;
            std::vector<std::string> matchs;

           if(separator.compare("")!=0){
               
                 if(keep_separator){ //保持分隔符
                  // bool first = false;
                   bool tail = false;
                   // 分隔符 可能是 。|？|! 这种 但是可能就匹配到了 。 
                   // 先分割 不带 分隔符的std::regex regex_pattern(separator);
                   std::regex regex_separator(separator);
                   std::vector<std::string> _splits(std::sregex_token_iterator(text.begin(), text.end(), regex_separator, -1),
                                    std::sregex_token_iterator()); 
                   matchs=extractMatches(text,separator); // matchs[0] 表示实际匹配到的分隔符  得到匹配的具体分隔符 比如我们的separator 是 .|?|! 但是匹配结果可能是 . ? !
                   // matchs.size 表示了text中有几个分隔符 
                   //假设分隔符不会出现在句子首
                   //A,B,C
                   //A,B,C,D. -> A,  B,  C, D.
                   // matchs[0]第一个分隔符 matchs[len-1]最后一个分隔符

                   std::regex regex_pattern(separator);
                   std::sregex_iterator iter(text.begin(), text.end(), regex_pattern);
                   std::sregex_iterator end;
                   
                    while (iter != end) {
                        std::smatch match = *iter;  // 获取匹配结果
                        size_t match_start = match.position();  // 匹配子串在原始字符串中的起始位置
                        size_t match_end = match_start + match.length();  // 匹配子串在原始字符串中的结束位置
                        
                        if(match_end==text.length()-1){
                             tail = true;
                             break;
                        }
       

                           ++iter;
                   }

                   for(int i=0,j=0;i<_splits.size()||j<matchs.size();i++,j++){
                            std::string temp = "";
                            if(i<_splits.size()){
                                
                                temp = temp + _splits[i];
                            }
                            if(j<matchs.size()){
                                temp = temp+matchs[j];
                            }
                            splits.push_back(temp);
                          
                   }



                 }else{
                  std::regex regex_separator(separator);
                  splits.assign(std::sregex_token_iterator(text.begin(), text.end(), regex_separator, -1),
                  std::sregex_token_iterator());
                 }
           }else{
               for(char c:text){
                splits.push_back(std::string(1,c));
               }
           }
          
           splits.erase(std::remove(splits.begin(),splits.end(),""),splits.end());
            
           return splits;

}






CRecursiveCharTextSplitter::CRecursiveCharTextSplitter(){
      
      this->separators = {};
      this->keep_separator = true;
      this->is_separator_regex = true;
      this->chunksize = 4000;
      this->chunkoverlap = 0;
}

CRecursiveCharTextSplitter::CRecursiveCharTextSplitter(std::vector<std::string> separators,bool keep_separator,bool is_separator_regex,int chunksize,int chunkoverlap){
     
       if(!separators.empty()){
           this->separators = separators;
       }else{
           
           /*
           不添加合并（merge）：为了保证句子通顺，连贯，分隔符细粒度大

           */
        //    this->separators.emplace_back("\\n\\n");
        //    this->separators.emplace_back("。|！|？");
        //    this->separators.emplace_back("\\.\\s|\\!\\s|\\?\\s");
        //    this->separators.emplace_back("\\t");



           /*
            添加合并（merge）：分隔符细粒度小

           */

           this->separators.emplace_back("\\n\\n");
           //this->separators.emplace_back("\\n");
           this->separators.emplace_back("。|！|？");
           this->separators.emplace_back("\\.\\s|\\!\\s|\\?\\s");
          // this->separators.emplace_back("，|,\\s");
          // this->separators.emplace_back("；|;\\s");
          //this->separators.emplace_back("，|,\\s");
          // this->separators.emplace_back("\\n");
           this->separators.emplace_back("\\t");
           //this->separators.emplace_back("\\n");
        


           
       }

       this->keep_separator = keep_separator;
       this->is_separator_regex = is_separator_regex;
       this->chunksize=chunksize;
       this->chunkoverlap = chunkoverlap;

}



/*
分割主体函数

*/
std::vector<std::string> CRecursiveCharTextSplitter::_split_text(std::string text,std::vector<std::string> separators,bool _keep_separator,int chunksize, int chunkoverlap){
          
          std::vector<std::string> splited_texts = {};
          std::string separator = separators.back(); //取最后一个元素
          std::vector<std::string> new_separators = {};

          std::string _s;
          std::string _separator;
          //先以分隔符优先级高的切分

          for(int i=0;i<separators.size();i++){  //遍历分隔符 从头遍历 ？？ 默认排在前面的优先级高是吗
              _s = separators[i];
              if(is_separator_regex){
                 _separator = _s;
              }else{
                 _separator = escapeRegex(_s);
              }

              if(_s.compare("")==0){
                separator = _s;
                break;
              }

              std::regex parttern(_separator);
              std::smatch match;
              if(std::regex_search(text,match,parttern)){
                  
                  separator = _s;
                  new_separators.clear();
                  std::copy(separators.begin() + i + 1, separators.end(), std::back_inserter(new_separators));
                  break;
              }

          }
           
        
          if(is_separator_regex){
            _separator = separator;
          }else{
            _separator = escapeRegex(separator);
          }
        
          std::vector<std::string> splits = _split_text_with_regex_from_end(text,_separator,keep_separator);
          
          
          // Now go merging things, recursively splitting longer texts.
          std::vector<std::string> _good_splits;
          if(_keep_separator){
            _separator = "";
          }else{
            _separator = separator;
          }

    

          std::vector<std::string> final_chunk;
          for(auto &s:splits){
            /*
            # TODO 如果不超长，直接添加到中间集合good_splits,否则对之前所有的good_splits进行合并，并且对当前超长的句子也当作一个大段落，使用同样的分隔逻辑递归处理,
            # TODO 直到所有的子块都[不超长]或者[没有可分的分隔符]为止，递归停止
            # TODO 对于good_splits，虽然每个子块没有超过chunk_size，但是将他们合并之后长度可能超出了chunk_size
            */

               if(s.length() < chunksize){
                   
                   _good_splits.push_back(s);
               }else{
                  
                   if (_good_splits.size()>0){
                     //std::cout<<"11123"<<std::endl;
                     std::vector<std::string> merged_text = _merge_splits(_good_splits,_separator,chunksize,chunkoverlap);
                     final_chunk.insert(final_chunk.end(),merged_text.begin(),merged_text.end());
                     _good_splits.clear();
                   }
                   if(new_separators.empty()){
                      final_chunk.push_back(s);
                   }else{
                     std::vector<std::string> other_info = _split_text(s,new_separators,keep_separator,chunksize,chunkoverlap);
                     final_chunk.insert(final_chunk.end(),other_info.begin(),other_info.end());

                   }

               }

          }
          
           if(!_good_splits.empty()){
           //  std::cout<<"11123"<<std::endl;
             std::vector<std::string> merged_text = _merge_splits(_good_splits,_separator,chunksize,chunkoverlap);
             final_chunk.insert(final_chunk.end(),final_chunk.begin(),final_chunk.end());
           }
          
       // 正则表达式模式：匹配连续出现至少两次的换行符 \n
    std::regex pattern("\n{2,}");
    std::vector<std::string> res;
   
    for(auto &chunk:final_chunk){

         if(strip(chunk)!=""){
            res.push_back(std::regex_replace(strip(chunk), pattern, "\n"));
         }
    }
    return res;
}



void removeNewlines(std::string& str) {
    // 去掉尾部的 '\n'
    while (!str.empty() && (str.back() == '\n' || str.back() == '\r')) {
        str.pop_back();
    }
}

std::pair<std::string,std::string> processFile(const fs::directory_entry &entry){
           
           std::string filename = entry.path().filename().string();
           std::ifstream file(entry.path().string());

           if(!file.is_open()){
             std::cerr<< "Failed to open file: "<<filename<<std::endl;
             exit(0);
           }

           std::string content;
           std::string line;
           while(std::getline(file,line)){
               
               content += line+"\n";
               //  content += line;
           }

           file.close();

           
           return std::make_pair(filename,content);
}

    /*
      
      读一个文件夹下的所有文件，然后将其以 <文件名，文件内容存储>

    */
std::vector<std::pair<std::string,std::string>> ReadFiles(std::string path){

            std::vector<std::pair<std::string,std::string>> res;

             try{
                 for(const auto& entry: fs::directory_iterator(path)){
                      
                      if(entry.is_regular_file()){
                          
                          std::pair<std::string,std::string> re = processFile(entry);
                          res.push_back(re);
                          
                      }
                 }

             }catch(const std::exception &ex){

                std::cerr<<"Exception caught: "<<ex.what()<<std::endl;
                exit(0);

             }

             return res;
}


//最终切割结果
std::vector<std::pair<std::string,std::vector<std::string>>> CRecursiveCharTextSplitter::files_to_splited_texts(std::string file_paths){
     

        
            std::vector<std::pair<std::string,std::string>> contents = ReadFiles(file_paths);

            std::vector<std::pair<std::string,std::vector<std::string>>> res;

            for(auto &file:contents){
                      

                        std::vector<std::string> temp = CRecursiveCharTextSplitter::_split_text(file.second,this->separators,this->keep_separator,this->chunksize,this->chunkoverlap);
                        res.push_back(std::make_pair(file.first,temp));
                         
            }
                   
            return res;
       
}


