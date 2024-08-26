set(TARGET llama-embedding)

# 将 embedding.cpp 编译为共享库（DLL）
add_library(${TARGET} SHARED embedding.cpp)

# 设置输出名称，确保生成 .dll 文件
set_target_properties(${TARGET} PROPERTIES 
    PREFIX ""
    OUTPUT_NAME "llama-embedding"
)

# 链接必要的库
target_link_libraries(${TARGET} PRIVATE common llama ${CMAKE_THREAD_LIBS_INIT})

# 设置 C++11 标准
target_compile_features(${TARGET} PRIVATE cxx_std_11)

# 安装目标
install(TARGETS ${TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# 如果需要导出符号，可以添加以下行
set_target_properties(${TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(MSVC)
    set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/embedding.def")
endif()