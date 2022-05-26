//reference to https://blog.csdn.net/liu1064782986/article/details/7982290

#include <cstring>
#include <cmath>
#include <cstdio>
#include <iostream>
#include "MemoryRiver.h"
#include "BPlusTree.h"

using namespace std;
CBPlusTree Database;
int main() {
    int n;
    string tp, Key;
    int val;
    scanf("%d", &n);
    for (int i = 1; i <= n; i++) {
        cin >> tp;
        if (tp == "insert") {
            cin >> Key >> val;
            Database.insert(Key, val);
        }
        if (tp == "delete") {
            cin >> Key >> val;
            Database.remove(Key, val);
        }
        if (tp == "find") {
            cin >> Key;
            Database.Findall(Key);
        }
    }
    return 0;
}