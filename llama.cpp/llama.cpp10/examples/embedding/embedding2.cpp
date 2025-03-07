#include<new>
#include<stdlib.h>
#include <ctime>
#include "common.h"
#include "llama.h"


#if defined(_MSC_VER)
#pragma warning(disable: 4244 4267) // possible loss of data
#endif


static std::vector<std::string> split_lines(const std::string & s, const std::string & separator = "\n") {
    std::vector<std::string> lines;
    size_t start = 0;
    size_t end = s.find(separator);

    while (end != std::string::npos) {
        lines.push_back(s.substr(start, end - start));
        start = end + separator.length();
        end = s.find(separator, start);
    }

    lines.push_back(s.substr(start)); // Add the last part

    return lines;
}

static void batch_add_seq(llama_batch & batch, const std::vector<int32_t> & tokens, llama_seq_id seq_id) {
    size_t n_tokens = tokens.size();
    for (size_t i = 0; i < n_tokens; i++) {
        llama_batch_add(batch, tokens[i], i, { seq_id }, true);
    }
}

static void batch_decode(llama_context * ctx, llama_batch & batch, float * output, int n_seq, int n_embd, int embd_norm) {
    // clear previous kv_cache values (irrelevant for embeddings)
    llama_kv_cache_clear(ctx);

    // run model
    fprintf(stderr, "%s: n_tokens = %d, n_seq = %d\n", __func__, batch.n_tokens, n_seq);
    if (llama_decode(ctx, batch) < 0) {
        fprintf(stderr, "%s : failed to decode\n", __func__);
    }

    for (int i = 0; i < batch.n_tokens; i++) {
        if (!batch.logits[i]) {
            continue;
        }

        // try to get sequence embeddings - supported only when pooling_type is not NONE
        const float * embd = llama_get_embeddings_seq(ctx, batch.seq_id[i][0]);
        GGML_ASSERT(embd != NULL && "failed to get sequence embeddings");

        float * out = output + batch.seq_id[i][0] * n_embd;
        llama_embd_normalize(embd, out, n_embd, embd_norm);
    }
}

#ifdef _WIN32
    #define LLAMA_API __declspec(dllexport)
#else
    #define LLAMA_API
#endif

extern "C" {
//std::vector<std::string> res_prompts,std::vector<std::vector<float>> &res,std::string model_path
int embedding(const char** c_res_prompts, int prompts_count, float*** res_out, int* res_sizes, int *vector_size,const char* c_model_path){
     
     
   //  ./llama-embedding-vul -m C:\wmq\projects\helloworld\projects\text_similarity_search4.0\embedding_models\bge-base-zh-v1.5-q8_0.gguf -e -p "hello world" --verbose-prompt -ngl 99  
   std::vector<std::string> res_prompts;
   for(int i=0;i<prompts_count;i++){
      
       res_prompts.push_back(std::string(c_res_prompts[i]));
   }
   
  std::string model_path = std::string(c_model_path);

  std::vector<std::vector<float>> res;
  res.reserve(8000);

   int argc = 9;
   char *argcv[10];
   argcv[0] = "";
   argcv[1] = "-m";
   argcv[2] = const_cast<char*>(model_path.c_str());
   argcv[3] = "-e";
   argcv[4] = "-p";
   argcv[5] = "";
   argcv[6] = "--verbose-prompt";
   argcv[7] = "-ngl";
   argcv[8] = "99";
   argcv[9] = nullptr;

    gpt_params params;

    if (!gpt_params_parse(argc, argcv, params)) {
        gpt_params_print_usage(argc, argcv, params);
        return -1;
    }


    params.embedding = true;
    // For non-causal models, batch size must be equal to ubatch size
    params.n_ubatch = params.n_batch;  // 2048

    print_build_info();

    if (params.seed == LLAMA_DEFAULT_SEED) {
        params.seed = time(NULL);
    }

    // fprintf(stderr, "%s: seed  = %u\n", __func__, params.seed);

    std::mt19937 rng(params.seed);

    llama_backend_init();
    llama_numa_init(params.numa);

    llama_model * model;
    llama_context * ctx;
    
    // load the model
    std::tie(model, ctx) = llama_init_from_gpt_params(params);
    if (model == NULL) {
        fprintf(stderr, "%s: error: unable to load model\n", __func__);
        return -1;
    }

    const int n_ctx_train = llama_n_ctx_train(model);
    const int n_ctx = llama_n_ctx(ctx);

    const enum llama_pooling_type pooling_type = llama_pooling_type(ctx);
    if (pooling_type == LLAMA_POOLING_TYPE_NONE) {
        fprintf(stderr, "%s: error: pooling type NONE not supported\n", __func__);
        return -1;
    }

    if (n_ctx > n_ctx_train) {
        fprintf(stderr, "%s: warning: model was trained on only %d context tokens (%d specified)\n",
                __func__, n_ctx_train, n_ctx);
    }

    // print system information
    // {
    //     fprintf(stderr, "\n");
    //     fprintf(stderr, "%s\n", gpt_params_get_system_info(params).c_str());
    // }

    std::cout<<"1"<<std::endl;

    const uint64_t n_batch = params.n_batch;
    GGML_ASSERT(params.n_batch >= params.n_ctx);
     
     std::cout<<"1.5"<<std::endl;
    
    for(auto &i:res_prompts){
         
         std::cout<<"prompts: "<<i<<std::endl;

    }

    for(auto &res_prompt:res_prompts){   //对于每一个prompt 进行处理

        std::cout<<"1.8"<<std::endl;
        std::vector<std::string> prompts = split_lines(res_prompt);

       /// std::vector<std::string> prompts;
        //prompts.push_back(res_prompt);


        std::cout<<"2"<<std::endl;

        std::vector<std::vector<int32_t>> inputs;
        for (const auto & prompt : prompts) {
            auto inp = ::llama_tokenize(ctx, prompt, true, false);
            std::cout<<"3"<<std::endl;
            if (inp.size() > n_batch) {
                fprintf(stderr, "%s: error: number of tokens in input line (%lld) exceeds batch size (%lld), increase batch size and re-run\n",
                        __func__, (long long int) inp.size(), (long long int) n_batch);
                return -1;
            }
            std::cout<<"4"<<std::endl;
            inputs.push_back(inp);
            std::cout<<"5"<<std::endl;
         }

        // check if the last token is SEP
        // it should be automatically added by the tokenizer when 'tokenizer.ggml.add_eos_token' is set to 'true'
        for (auto & inp : inputs) {
            if (inp.empty() || inp.back() != llama_token_sep(model)) {
                fprintf(stderr, "%s: warning: last token in the prompt is not SEP\n", __func__);
                fprintf(stderr, "%s:          'tokenizer.ggml.add_eos_token' should be set to 'true' in the GGUF header\n", __func__);
            }
        }
        std::cout<<"6"<<std::endl;

        // tokenization stats
        if (params.verbose_prompt) {
            for (int i = 0; i < (int) inputs.size(); i++) {
                fprintf(stderr, "%s: prompt %d: '%s'\n", __func__, i, prompts[i].c_str());
                fprintf(stderr, "%s: number of tokens in prompt = %zu\n", __func__, inputs[i].size());
                for (int j = 0; j < (int) inputs[i].size(); j++) {
                    fprintf(stderr, "%6d -> '%s'\n", inputs[i][j], llama_token_to_piece(ctx, inputs[i][j]).c_str());
             }
                fprintf(stderr, "\n\n");
            }
        }
        std::cout<<"7"<<std::endl;
        // initialize batch
        const int n_prompts = prompts.size();  //这个是batch size
        struct llama_batch batch = llama_batch_init(n_batch, 0, 1);
        std::cout<<"8"<<std::endl;
        // allocate output
        const int n_embd = llama_n_embd(model);
        std::vector<float> embeddings(n_prompts * n_embd, 0);
        float * emb = embeddings.data();
        std::cout<<"9"<<std::endl;

        // break into batches
        int p = 0; // number of prompts processed already
        int s = 0; // number of prompts in current batch
        for (int k = 0; k < n_prompts; k++) {
            // clamp to n_batch tokens
            auto & inp = inputs[k];

            const uint64_t n_toks = inp.size();

            // encode if at capacity
            if (batch.n_tokens + n_toks > n_batch) {
                float * out = emb + p * n_embd;
                batch_decode(ctx, batch, out, s, n_embd, params.embd_normalize);
                llama_batch_clear(batch);
                p += s;
                s = 0;
            }

            // add to batch
            batch_add_seq(batch, inp, s);
            s += 1;
        }
        std::cout<<"10"<<std::endl;
        // final batch
        float * out = emb + p * n_embd;
        batch_decode(ctx, batch, out, s, n_embd, params.embd_normalize);
        std::vector<float> temp;
        if (params.embd_out.empty()) {
            for (int j = 0; j < n_prompts; j++) {
                for (int i = 0; i < (n_prompts > 1 ? std::min(16, n_embd) : n_embd); i++) {
              // for (int i = 0; i < 256; i++) {
                    temp.push_back(emb[j*n_embd+i]);
            }
        }
        std::cout<<"11"<<std::endl;
        //print temp
        for(auto &i:temp){
            std::cout<<i<<" ";
        }
        std::cout<<std::endl;

        res.push_back(temp);

        }

    }

     
    //结果存入 float **res_out
    *res_sizes = res.size();
    *vector_size = res[0].size();

    *res_out = new float*[*res_sizes];

    for(int i=0;i<*res_sizes;i++){

        (*res_out)[i] = new float[*vector_size];
        for(int j=0;j<*vector_size;j++){
             (*res_out)[i][j] = res[i][j];
        }
    }


    return 1;

   

}

}










// int main(int argc, char ** argv) {
//     gpt_params params;

//     if (!gpt_params_parse(argc, argv, params)) {
//         gpt_params_print_usage(argc, argv, params);
//         return 1;
//     }

//     params.embedding = true;
//     // For non-causal models, batch size must be equal to ubatch size
//     params.n_ubatch = params.n_batch;

//     print_build_info();

//     if (params.seed == LLAMA_DEFAULT_SEED) {
//         params.seed = time(NULL);
//     }

//     fprintf(stderr, "%s: seed  = %u\n", __func__, params.seed);

//     std::mt19937 rng(params.seed);

//     llama_backend_init();
//     llama_numa_init(params.numa);

//     llama_model * model;
//     llama_context * ctx;

//     // load the model
//     std::tie(model, ctx) = llama_init_from_gpt_params(params);
//     if (model == NULL) {
//         fprintf(stderr, "%s: error: unable to load model\n", __func__);
//         return 1;
//     }

//     const int n_ctx_train = llama_n_ctx_train(model);
//     const int n_ctx = llama_n_ctx(ctx);

//     const enum llama_pooling_type pooling_type = llama_pooling_type(ctx);
//     if (pooling_type == LLAMA_POOLING_TYPE_NONE) {
//         fprintf(stderr, "%s: error: pooling type NONE not supported\n", __func__);
//         return 1;
//     }

//     if (n_ctx > n_ctx_train) {
//         fprintf(stderr, "%s: warning: model was trained on only %d context tokens (%d specified)\n",
//                 __func__, n_ctx_train, n_ctx);
//     }

//     // print system information
//     {
//         fprintf(stderr, "\n");
//         fprintf(stderr, "%s\n", gpt_params_get_system_info(params).c_str());
//     }

//     // split the prompt into lines
//     std::vector<std::string> prompts = split_lines(params.prompt, params.embd_sep);

//     // max batch size
//     const uint64_t n_batch = params.n_batch;
//     GGML_ASSERT(params.n_batch >= params.n_ctx);

//     // tokenize the prompts and trim
//     std::vector<std::vector<int32_t>> inputs;
//     for (const auto & prompt : prompts) {
//         auto inp = ::llama_tokenize(ctx, prompt, true, false);
//         if (inp.size() > n_batch) {
//             fprintf(stderr, "%s: error: number of tokens in input line (%lld) exceeds batch size (%lld), increase batch size and re-run\n",
//                     __func__, (long long int) inp.size(), (long long int) n_batch);
//             return 1;
//         }
//         inputs.push_back(inp);
//     }

//     // check if the last token is SEP
//     // it should be automatically added by the tokenizer when 'tokenizer.ggml.add_eos_token' is set to 'true'
//     for (auto & inp : inputs) {
//         if (inp.empty() || inp.back() != llama_token_sep(model)) {
//             fprintf(stderr, "%s: warning: last token in the prompt is not SEP\n", __func__);
//             fprintf(stderr, "%s:          'tokenizer.ggml.add_eos_token' should be set to 'true' in the GGUF header\n", __func__);
//         }
//     }

//     // tokenization stats
//     if (params.verbose_prompt) {
//         for (int i = 0; i < (int) inputs.size(); i++) {
//             fprintf(stderr, "%s: prompt %d: '%s'\n", __func__, i, prompts[i].c_str());
//             fprintf(stderr, "%s: number of tokens in prompt = %zu\n", __func__, inputs[i].size());
//             for (int j = 0; j < (int) inputs[i].size(); j++) {
//                 fprintf(stderr, "%6d -> '%s'\n", inputs[i][j], llama_token_to_piece(ctx, inputs[i][j]).c_str());
//             }
//             fprintf(stderr, "\n\n");
//         }
//     }

//     // initialize batch
//     const int n_prompts = prompts.size();
//     struct llama_batch batch = llama_batch_init(n_batch, 0, 1);

//     // allocate output
//     const int n_embd = llama_n_embd(model);
//     std::vector<float> embeddings(n_prompts * n_embd, 0);
//     float * emb = embeddings.data();

//     // break into batches
//     int p = 0; // number of prompts processed already
//     int s = 0; // number of prompts in current batch
//     for (int k = 0; k < n_prompts; k++) {
//         // clamp to n_batch tokens
//         auto & inp = inputs[k];

//         const uint64_t n_toks = inp.size();

//         // encode if at capacity
//         if (batch.n_tokens + n_toks > n_batch) {
//             float * out = emb + p * n_embd;
//             batch_decode(ctx, batch, out, s, n_embd, params.embd_normalize);
//             llama_batch_clear(batch);
//             p += s;
//             s = 0;
//         }

//         // add to batch
//         batch_add_seq(batch, inp, s);
//         s += 1;
//     }

//     // final batch
//     float * out = emb + p * n_embd;
//     batch_decode(ctx, batch, out, s, n_embd, params.embd_normalize);

//     if (params.embd_out.empty()) {
//         // print the first part of the embeddings or for a single prompt, the full embedding
//         fprintf(stdout, "\n");
//         for (int j = 0; j < n_prompts; j++) {
//             fprintf(stdout, "embedding %d: ", j);
//             for (int i = 0; i < (n_prompts > 1 ? std::min(16, n_embd) : n_embd); i++) {
//                 if (params.embd_normalize == 0) {
//                     fprintf(stdout, "%6.0f ", emb[j * n_embd + i]);
//                 } else {
//                     fprintf(stdout, "%9.6f ", emb[j * n_embd + i]);
//                 }
//             }
//             fprintf(stdout, "\n");
//         }

//         // print cosine similarity matrix
//         if (n_prompts > 1) {
//             fprintf(stdout, "\n");
//             printf("cosine similarity matrix:\n\n");
//             for (int i = 0; i < n_prompts; i++) {
//                 fprintf(stdout, "%6.6s ", prompts[i].c_str());
//             }
//             fprintf(stdout, "\n");
//             for (int i = 0; i < n_prompts; i++) {
//                 for (int j = 0; j < n_prompts; j++) {
//                     float sim = llama_embd_similarity_cos(emb + i * n_embd, emb + j * n_embd, n_embd);
//                     fprintf(stdout, "%6.2f ", sim);
//                 }
//                 fprintf(stdout, "%1.10s", prompts[i].c_str());
//                 fprintf(stdout, "\n");
//             }
//         }
//     }

//     if (params.embd_out == "json" || params.embd_out == "json+" || params.embd_out == "array") {
//         const bool notArray = params.embd_out != "array";

//         fprintf(stdout, notArray ? "{\n  \"object\": \"list\",\n  \"data\": [\n" : "[");
//         for (int j = 0;;) { // at least one iteration (one prompt)
//             if (notArray) fprintf(stdout, "    {\n      \"object\": \"embedding\",\n      \"index\": %d,\n      \"embedding\": ",j);
//             fprintf(stdout, "[");
//             for (int i = 0;;) { // at least one iteration (n_embd > 0)
//                 fprintf(stdout, params.embd_normalize == 0 ? "%1.0f" : "%1.7f", emb[j * n_embd + i]);
//                 i++;
//                 if (i < n_embd) fprintf(stdout, ","); else break;
//             }
//             fprintf(stdout, notArray ? "]\n    }" : "]");
//             j++;
//             if (j < n_prompts) fprintf(stdout, notArray ? ",\n" : ","); else break;
//         }
//         fprintf(stdout, notArray ? "\n  ]" : "]\n");

//         if (params.embd_out == "json+" && n_prompts > 1) {
//             fprintf(stdout, ",\n  \"cosineSimilarity\": [\n");
//             for (int i = 0;;) { // at least two iteration (n_prompts > 1)
//                 fprintf(stdout, "    [");
//                 for (int j = 0;;) { // at least two iteration (n_prompts > 1)
//                     float sim = llama_embd_similarity_cos(emb + i * n_embd, emb + j * n_embd, n_embd);
//                     fprintf(stdout, "%6.2f", sim);
//                     j++;
//                     if (j < n_prompts) fprintf(stdout, ", "); else break;
//                 }
//                 fprintf(stdout, " ]");
//                 i++;
//                 if (i < n_prompts) fprintf(stdout, ",\n"); else break;
//             }
//             fprintf(stdout, "\n  ]");
//         }

//         if (notArray) fprintf(stdout, "\n}\n");
//     }

//     // clean up
//     llama_print_timings(ctx);
//     llama_batch_free(batch);
//     llama_free(ctx);
//     llama_free_model(model);
//     llama_backend_free();

//     return 0;
// }


// #include "common.h"
// #include "llama.h"
// #include <vector>
// #include <string>
// #include <stdexcept>

// std::vector<std::vector<float>> get_embeddings_for_sentences(const std::string& model_path, const std::vector<std::string>& sentences) {
//     // 初始化后端
//     llama_backend_init();

//     // 设置模型参数
//     llama_model_params model_params = llama_model_default_params();
//     llama_context_params ctx_params = llama_context_default_params();
//     ctx_params.n_gpu_layers = -1; // 使用所有可用的GPU层
//     ctx_params.embeddings = true;  // 启用embedding模式

//     // 加载模型
//     llama_model* model = llama_load_model_from_file(model_path.c_str(), model_params);
//     if (!model) {
//         throw std::runtime_error("Failed to load model");
//     }

//     // 创建上下文
//     llama_context* ctx = llama_new_context_with_model(model, ctx_params);
//     if (!ctx) {
//         llama_free_model(model);
//         throw std::runtime_error("Failed to create context");
//     }

//     const int n_embd = llama_n_embd(model);
//     const int n_ctx = llama_n_ctx(ctx);
//     std::vector<std::vector<float>> all_embeddings;

//     llama_batch batch = llama_batch_init(n_ctx, 0, 1);

//     for (const auto& sentence : sentences) {
//         // 对句子进行tokenization
//         auto tokens = llama_tokenize(ctx, sentence, true, false);
        
//         // 如果token数量超过上下文大小，进行截断
//         if (tokens.size() > n_ctx) {
//             tokens.resize(n_ctx);
//         }

//         // 清除之前的KV缓存
//         llama_kv_cache_clear(ctx);

//         // 将tokens添加到batch中
//         llama_batch_clear(batch);
//         for (size_t i = 0; i < tokens.size(); i++) {
//             llama_batch_add(batch, tokens[i], i, { 0 }, i == tokens.size() - 1);
//         }

//         // 执行推理
//         if (llama_decode(ctx, batch) < 0) {
//             fprintf(stderr, "Failed to decode\n");
//             continue;
//         }

//         // 获取embedding
//         const float* embd = llama_get_embeddings_seq(ctx, 0);
//         if (embd == NULL) {
//             fprintf(stderr, "Failed to get embeddings\n");
//             continue;
//         }

//         // 归一化并保存embedding
//         std::vector<float> normalized_embd(n_embd);
//         llama_embd_normalize(embd, normalized_embd.data(), n_embd);
//         all_embeddings.push_back(normalized_embd);
//     }

//     // 清理资源
//     llama_batch_free(batch);
//     llama_free(ctx);
//     llama_free_model(model);
//     llama_backend_free();

//     return all_embeddings;
// }