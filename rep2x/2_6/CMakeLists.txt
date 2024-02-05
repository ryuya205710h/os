cmake_minimum_required(VERSION 3.10)
project(ls_la C)

set(CMAKE_C_STANDARD 11)

# ls_la.c のビルド設定
add_executable(ls_la src/ls_la.c)

# recursive_ls.c のビルド設定
add_executable(ls_laR src/ls_laR.c)