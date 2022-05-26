//
// Created by 27595 on 2022/5/26.
//

#ifndef BPLUS_TREE_BPLUSTREE_H
#define BPLUS_TREE_BPLUSTREE_H


#include <vector>
#include "MemoryRiver.h"

const int INVALID_INDEX = -1;
enum NODE_TYPE {
    INTERNAL, LEAF
};        // 结点类型：内结点、叶子结点
enum SIBLING_DIRECTION {
    LEFT, RIGHT
};   // 兄弟结点方向：左兄弟结点、右兄弟结点
typedef string KeyType;                 // 键类型
typedef int DataType;                  // 值类型
const int ORDER = 7;                   // B+树的阶（非根内结点的最小子树个数）
const int MINNUM_KEY = ORDER - 1;        // 最小键值个数
const int MAXNUM_KEY = 2 * ORDER - 1;      // 最大键值个数
const int MINNUM_CHILD = MINNUM_KEY + 1; // 最小子树个数
const int MAXNUM_CHILD = MAXNUM_KEY + 1; // 最大子树个数
const int MINNUM_LEAF = MINNUM_KEY;    // 最小叶子结点键值个数
const int MAXNUM_LEAF = MAXNUM_KEY;    // 最大叶子结点键值个数

class CBPlusTree;

class CNode {
protected:
    NODE_TYPE m_Type;
    int m_KeyNum;
    KeyType m_KeyValues[MAXNUM_KEY];
    int store_possession;
    CBPlusTree *tree;
public:
    CNode();

    virtual ~CNode();

    int getPossession() { return store_possession; }

    NODE_TYPE getType() const { return m_Type; }

    void setType(NODE_TYPE type) { m_Type = type; }

    int getKeyNum() const { return m_KeyNum; }

    void setKeyNum(int n) { m_KeyNum = n; }

    KeyType getKeyValue(int i) const { return m_KeyValues[i]; }

    void setKeyValue(int i, KeyType key) { m_KeyValues[i] = key; }

    int getKeyIndex(KeyType key) const;  // 找到键值在结点中存储的下标(第一个不小于key的下标)
    // 纯虚函数，定义接口
    virtual void removeKey(int keyIndex, int childIndex) = 0;  // 从结点中移除键值
    virtual void split(CNode *parentNode, int childIndex) = 0; // 分裂结点
    virtual void mergeChild(CNode *parentNode, CNode *childNode, int keyIndex) = 0;  // 合并结点
    virtual void clear() = 0; // 清空结点，同时会清空结点所包含的子树结点
    virtual void
    borrowFrom(CNode *destNode, CNode *parentNode, int keyIndex, SIBLING_DIRECTION d) = 0; // 从兄弟结点中借一个键值
    virtual int getChildIndex(KeyType key, int keyIndex) const = 0;  // 根据键值获取孩子结点指针下标
};

// 叶子结点
class CLeafNode : public CNode {
private:
    int m_LeftSibling;
    int m_RightSibling;
    DataType m_Datas[MAXNUM_LEAF];
public:
    CLeafNode();

    virtual ~CLeafNode();

    CLeafNode *getLeftSibling() const;

    void setLeftSibling(int possession) { m_LeftSibling = possession; }

    CLeafNode *getRightSibling();

    void setRightSibling(int possession) { m_RightSibling = possession; }

    DataType getData(int i) const { return m_Datas[i]; }

    void remove(KeyType key, const DataType &dataValue);

    void setData(int i, const DataType &data) { m_Datas[i] = data; }

    void insert(KeyType key, const DataType &data);

    virtual void split(CNode *parentNode, int childIndex);

    virtual void mergeChild(CNode *parentNode, CNode *childNode, int keyIndex);

    virtual void removeKey(int keyIndex, int childIndex);

    virtual void clear();

    virtual void borrowFrom(CNode *destNode, CNode *parentNode, int keyIndex, SIBLING_DIRECTION d);

    virtual int getChildIndex(KeyType key, int keyIndex) const;
};

// 内结点
class CInternalNode : public CNode {
private:
    int m_Childs[MAXNUM_CHILD];
    NODE_TYPE childs_type[MAXNUM_CHILD];
public:
    CInternalNode();

    virtual ~CInternalNode();

    int getChild(int i) { return m_Childs[i]; }

    CInternalNode *getChild_Internal(int i);

    CLeafNode *getChild_Leaf(int i);

    void setChild(int i, int child, NODE_TYPE type) {
        m_Childs[i] = child;
        childs_type[i] = type;
    }

    void insert(int keyIndex, int childIndex, KeyType key, int childNode, NODE_TYPE type);//在keyIndex/childIndex位置插入
    //不修改文件
    //NODE_TYPE必须是CInternal

    virtual NODE_TYPE getChildtype(int i) { return childs_type[i]; }

    virtual void split(CNode *parentNode, int childIndex);//childIndex是split的节点在parentnode里的序号

    virtual void mergeChild(CNode *parentNode, CNode *childNode, int keyIndex);//将childnode（右）合并到this（左）里

    virtual void removeKey(int keyIndex, int childIndex);//不修改文件

    virtual void clear();

    virtual void borrowFrom(CNode *destNode, CNode *parentNode, int keyIndex, SIBLING_DIRECTION d);//尽量从左边借，复杂度小

    virtual int getChildIndex(KeyType key, int keyIndex) const;

};

class CBPlusTree {
public:
    CBPlusTree();

    ~CBPlusTree();

    void insert(KeyType key, const DataType &data);

    void remove(KeyType key, DataType &dataValue);

    vector<DataType> Findall(KeyType compareKey);

    bool search(KeyType key);

    void clear();

    void InternalRead(CInternalNode *&node, int index) { CInternalNode_store.read(node, index); };

    void InternalDelete(int index) { CInternalNode_store.Delete(index); }

    void InteranlUpdate(CInternalNode *&node, int index) { CInternalNode_store.update(node, index); }

    int InternalWrite(CInternalNode *&node) { return CInternalNode_store.write(node); }

    void LeafRead(CLeafNode *&node, int index) { CLeafNode_store.read(node, index); };

    void LeafDelete(int index) { CLeafNode_store.Delete(index); }

    void LeafUpdate(CLeafNode *&node, int index) { CLeafNode_store.update(node, index); }

    int LeafWrite(CLeafNode *&node) { return CLeafNode_store.write(node); }

private:
    MemoryRiver<CInternalNode *, 1> CInternalNode_store;
    MemoryRiver<CLeafNode *, 1> CLeafNode_store;
    CNode *m_Root;
    CLeafNode *m_DataHead;
private:
    void recursive_insert(CNode *parentNode, KeyType key, const DataType &data);

    void recursive_remove(CNode *parentNode, KeyType key);

    bool recursive_search(CNode *pNode, KeyType key) const;

    void changeKey(CNode *pNode, KeyType oldKey, KeyType newKey);

    void recursive_remove(CNode *parentNode, KeyType key, DataType &dataValue);
};


#endif //BPLUS_TREE_BPLUSTREE_H
