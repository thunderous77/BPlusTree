//
// Created by 27595 on 2022/5/26.
//

#ifndef BPLUS_TREE_MEMORYRIVER_H
#define BPLUS_TREE_MEMORYRIVER_H

#include <cstring>
#include <cmath>
#include <cstdio>
#include <fstream>
using namespace std;

//T为存储类型，info_len为开头int类型个数(info一开始都设为0),info中的第一位存的是数据占据的最大位置(即要求info_len>=1)
template<class T, int info_len = 1>
class MemoryRiver {
private:
    /* your code here */
    fstream file, file2;
    string file_name;
    string file_name2;
    int sizeofT = sizeof(T);
    int num, num2;

    void read2_las(int &t) {
        file2.open(file_name2);
        int index = num2 * sizeof(int);
        file2.seekp(index);
        file2.read(reinterpret_cast<char *>(&t), sizeof(int));
        file2.close();
    }

    int write2(int &t) {
        file2.open(file_name2);
        int index = (num2 + 1) * sizeof(int);
        file2.seekp(index);
        file2.write(reinterpret_cast<char *>(&t), sizeof(int));
        num2++;
        file2.seekp(0);
        file2.write(reinterpret_cast<char *>(&num2), sizeof(int));
        file2.close();
        return index;
    }

    void initialise2(const string &FN) {
        file_name2 = FN;
        file2.open(file_name2);
        if (!file2) {
            file2.open(file_name2, std::ios::out);//若文件不存在自动创建
            num2 = 0;
            file2.write(reinterpret_cast<char *>(&num2), sizeof(int));
            file2.close();
        } else {
            file2.read(reinterpret_cast<char *>(&num2), sizeof(int));
            file2.close();
        }
    }

public:
    void initialise(const string &FN) {
        file_name = FN;
        file.open(file_name);
        if (!file) {
            file.open(file_name, std::ios::out);//若文件不存在自动创建
            int tmp = 0;
            for (int i = 0; i < info_len; ++i)file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
            num = 0;
            file.close();
        } else {
            file.seekp(0);
            file.read(reinterpret_cast<char *>(&num), sizeof(int));
            file.close();
        }
        initialise2(FN + "_memory_recycling");
    }

    MemoryRiver(const string &FN = "") {
        if (FN != "")initialise(FN);
    }

    //读出第n个int的值赋给tmp，1_base
    void read_info(int &tmp, int n) {
        if (n > info_len) return;
        file.open(file_name);
        file.seekp((n - 1) * sizeof(int));
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
    }

    //将tmp写入第n个int的位置，1_base
    void write_info(int tmp, int n) {
        if (n > info_len) return;
        file.open(file_name);
        file.seekp((n - 1) * sizeof(int));
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
    }


    //在文件合适位置写入类对象t，并返回写入的位置索引index
    //位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
    //位置索引index可以取为对象写入的起始位置
    int write(T &t) {
        file.open(file_name);
        int index;
        if (num2) {
            read2_las(index);
            num2--;
            file2.seekp(0);
            file2.write(reinterpret_cast<char *>(&num2), sizeof(int));
        } else {
            index = info_len * sizeof(int) + num * sizeofT;
            num++;
            file.seekp(0);
            file.write(reinterpret_cast<char *>(&num), sizeof(int));
        }
        file.seekp(index);
        file.write(reinterpret_cast<char *>(&t), sizeofT);
        file.close();
        return index;
    }

    //用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
    void update(T &t, const int index) {
        file.open(file_name);
        file.seekp(index);
        file.write(reinterpret_cast<char *>(&t), sizeofT);
        file.close();
        /* your code here */
    }

    //读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
    void read(T &t, const int index) {
        file.open(file_name);
        file.seekp(index);
        file.read(reinterpret_cast<char *>(&t), sizeofT);
        file.close();
        /* your code here */
    }

    //删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
    void Delete(int index) {
        write2(index);
    }
};

#endif //BPLUS_TREE_MEMORYRIVER_H
