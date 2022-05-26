cmake_minimum_required(VERSION 3.16)
project(bplus_tree)

set(CMAKE_CXX_STANDARD 20)

add_executable(bplus_tree main.cpp BPlusTree.cpp BPlusTree.h MemoryRiver.h ../CPlustree_test/BPlusTree.cpp ../CPlustree_test/BPlusTree.h)
