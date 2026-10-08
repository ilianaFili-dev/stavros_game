cmake_minimum_required(VERSION 3.20)
project(AshinaArena CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(minhook STATIC
    third_party/minhook/src/buffer.c
    third_party/minhook/src/hook.c
    third_party/minhook/src/trampoline.c
    third_party/minhook/src/hde/hde64.c)
target_include_directories(minhook PUBLIC third_party/minhook/include)

add_library(dinput8 SHARED
    src/proxy.cpp
    src/log.cpp
    src/clonedrone_locator.cpp
    src/dinput8.def)
target_include_directories(dinput8 PRIVATE src)
target_link_libraries(dinput8 PRIVATE minhook advapi32)
set_target_properties(dinput8 PROPERTIES PREFIX "" OUTPUT_NAME "dinput8")
if(MINGW)
    target_link_options(dinput8 PRIVATE -static -static-libgcc -static-libstdc++ -s)
endif()
