//
// Created by 27595 on 2022/5/26.
//
#include "BPlusTree.h"
#include <string.h>
#include <iostream>
#include <algorithm>
#include <vector>

// CNode
CNode::CNode() {
    setType(LEAF);
    setKeyNum(0);
}

CNode::~CNode() {
    setKeyNum(0);
}

int CNode::getKeyIndex(KeyType key) const {
    int left = 0;
    int right = getKeyNum() - 1;
    int mid;
    while (left != right) {
        mid = (left + right) >> 1;
        KeyType currentKey = getKeyValue(mid);
        if (strcmp(key.c_str(), currentKey.c_str()) > 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return left;
    /*//left==right
    if (strcmp(key.c_str(), getKeyValue(left).c_str()) < 0)
        return mid;
    else return mid + 1;*/
}

// CInternalNode
CInternalNode::CInternalNode() : CNode() {
    setType(INTERNAL);
}

CInternalNode::~CInternalNode() {
    clear();
}

CInternalNode *CInternalNode::getChild_Internal(int i) {
    CInternalNode *tmp;
    tree->InternalRead(tmp, i);
    return tmp;
}

CLeafNode *CInternalNode::getChild_Leaf(int i) {
    CLeafNode *tmp;
    tree->LeafRead(tmp, i);
    return tmp;
}

void CInternalNode::clear() {
    for (int i = 0; i <= m_KeyNum; ++i) {
        if (childs_type[i] == INTERNAL) {
            getChild_Internal(m_Childs[i])->clear();
            tree->InternalDelete(m_Childs[i]);
        } else {
            getChild_Leaf(m_Childs[i])->clear();
        }
        m_Childs[i] = 0;
    }
}

void CInternalNode::split(CNode *parentNode, int childIndex) {//childIndex=keyIndex+1
    CInternalNode *newNode = new CInternalNode();//分裂后的右节点
    newNode->setKeyNum(MINNUM_KEY);
    int i;
    for (i = 0; i < MINNUM_KEY; ++i)// 拷贝关键字的值
    {
        newNode->setKeyValue(i, m_KeyValues[i + MINNUM_CHILD]);
    }
    for (i = 0; i < MINNUM_CHILD; ++i) // 拷贝孩子节点指针
    {
        newNode->setChild(i, m_Childs[i + MINNUM_CHILD], childs_type[i + MINNUM_CHILD]);
    }
    setKeyNum(MINNUM_KEY);  //更新左子树的关键字个数
    newNode->store_possession = tree->InternalWrite(newNode);
    tree->InteranlUpdate(newNode, newNode->store_possession);
    ((CInternalNode *) parentNode)->insert(childIndex, childIndex + 1, m_KeyValues[MINNUM_KEY],
                                           newNode->store_possession, INTERNAL);
    CInternalNode *tmp = (CInternalNode *) parentNode;
    tree->InteranlUpdate(tmp, parentNode->getPossession());
    tmp = this;
    tree->InteranlUpdate(tmp, store_possession);
    delete newNode;
}

void CInternalNode::insert(int keyIndex, int childIndex, KeyType key, int childNode, NODE_TYPE type) {
    int i;
    for (i = getKeyNum(); i > keyIndex; --i)//将父节点中的childIndex后的所有关键字的值和子树指针向后移一位
    {
        setChild(i + 1, m_Childs[i], childs_type[i]);
        setKeyValue(i, m_KeyValues[i - 1]);
    }
    /*if (i == childIndex) {
        setChild(i + 1, m_Childs[i]);
    }*/
    setChild(childIndex, childNode, type);
    setKeyValue(keyIndex, key);
    setKeyNum(m_KeyNum + 1);
}

void CInternalNode::mergeChild(CNode *parentNode, CNode *childNode, int keyIndex) {
    // 合并数据
    insert(MINNUM_KEY, MINNUM_KEY + 1, parentNode->getKeyValue(keyIndex), ((CInternalNode *) childNode)->getChild(0),
           INTERNAL);
    int i;
    for (i = 1; i <= childNode->getKeyNum(); ++i) {
        insert(MINNUM_KEY + i, MINNUM_KEY + i + 1, childNode->getKeyValue(i - 1),
               ((CInternalNode *) childNode)->getChild(i), INTERNAL);
    }
    //父节点删除index的key
    tree->InternalDelete(childNode->getPossession());
    parentNode->removeKey(keyIndex + 1, keyIndex + 2);
    CInternalNode *tmp = (CInternalNode *) parentNode;
    tree->InteranlUpdate(tmp, parentNode->getPossession());
    tmp = this;
    tree->InteranlUpdate(tmp, store_possession);
}

void CInternalNode::removeKey(int keyIndex, int childIndex) {
    for (int i = 0; i < getKeyNum() - keyIndex - 1; ++i) {
        setKeyValue(keyIndex + i, getKeyValue(keyIndex + i + 1));
        setChild(childIndex + i, m_Childs[childIndex + i + 1], childs_type[childIndex + i + 1]);
    }
    setKeyNum(getKeyNum() - 1);
}

void CInternalNode::borrowFrom(CNode *siblingNode, CNode *parentNode, int keyIndex, SIBLING_DIRECTION d) {
    switch (d) {
        case LEFT:  // 从左兄弟结点借
        {
            insert(0, 0, parentNode->getKeyValue(keyIndex),
                   ((CInternalNode *) siblingNode)->getChild(siblingNode->getKeyNum()), INTERNAL);
            parentNode->setKeyValue(keyIndex, siblingNode->getKeyValue(siblingNode->getKeyNum() - 1));
            siblingNode->removeKey(siblingNode->getKeyNum() - 1, siblingNode->getKeyNum());
        }
            break;
        case RIGHT:  // 从右兄弟结点借
        {
            insert(getKeyNum(), getKeyNum() + 1, parentNode->getKeyValue(keyIndex),
                   ((CInternalNode *) siblingNode)->getChild(0), INTERNAL);
            parentNode->setKeyValue(keyIndex, siblingNode->getKeyValue(0));
            siblingNode->removeKey(0, 0);
        }
            break;
    }
    CInternalNode *tmp = (CInternalNode *) parentNode;
    tree->InteranlUpdate(tmp, parentNode->getPossession());
    tmp = this;
    tree->InteranlUpdate(tmp, store_possession);
}

int CInternalNode::getChildIndex(KeyType key, int keyIndex) const {
    if (strcmp(key.c_str(), getKeyValue(keyIndex).c_str()) == 0) {
        return keyIndex + 1;
    } else {
        return keyIndex;
    }
}

// CLeafNode
CLeafNode::CLeafNode() : CNode() {
    setType(LEAF);
    m_LeftSibling = m_RightSibling = 0;
}

CLeafNode::~CLeafNode() {
    clear();
}

void CLeafNode::clear() { tree->LeafDelete(store_possession); }

CLeafNode *CLeafNode::getLeftSibling() const {
    CLeafNode *tmp;
    tree->LeafRead(tmp, m_LeftSibling);
    return tmp;
}

CLeafNode *CLeafNode::getRightSibling() {
    CLeafNode *tmp;
    tree->LeafRead(tmp, m_RightSibling);
    return tmp;
}

void CLeafNode::insert(KeyType key, const DataType &data) {
    int i;
    for (i = m_KeyNum; i >= 1 && (strcmp(m_KeyValues[i - 1].c_str(), key.c_str()) > 0 ||(
                       strcmp(m_KeyValues[i - 1].c_str(), key.c_str()) == 0 && m_Datas[i - 1] > data)); --i) {
        setKeyValue(i, m_KeyValues[i - 1]);
        setData(i, m_Datas[i - 1]);
    }
    setKeyValue(i, key);
    setData(i, data);
    setKeyNum(m_KeyNum + 1);
}

void CLeafNode::split(CNode *parentNode, int childIndex) {
    CLeafNode *newNode = new CLeafNode();//分裂后的右节点
    setKeyNum(MINNUM_LEAF);
    newNode->setKeyNum(MINNUM_LEAF + 1);
    newNode->setRightSibling(m_RightSibling);
    setRightSibling(newNode->getPossession());
    newNode->setLeftSibling(this->getPossession());
    int i;
    for (i = 0; i < MINNUM_LEAF + 1; ++i)// 拷贝关键字的值
    {
        newNode->setKeyValue(i, m_KeyValues[i + MINNUM_LEAF]);
    }
    for (i = 0; i < MINNUM_LEAF + 1; ++i)// 拷贝数据
    {
        newNode->setData(i, m_Datas[i + MINNUM_LEAF]);
    }
    newNode->store_possession = tree->LeafWrite(newNode);
    tree->LeafUpdate(newNode, newNode->getPossession());
    CLeafNode *tmp1 = this;
    tree->LeafUpdate(tmp1, getPossession());
    ((CInternalNode *) parentNode)->insert(childIndex, childIndex + 1, m_KeyValues[MINNUM_LEAF],
                                           newNode->store_possession, LEAF);
    CInternalNode *tmp2 = (CInternalNode *) parentNode;
    tree->InteranlUpdate(tmp2, parentNode->getPossession());
    delete newNode;
}

void CLeafNode::mergeChild(CNode *parentNode, CNode *childNode, int keyIndex) {
    // 合并数据
    for (int i = 0; i < childNode->getKeyNum(); ++i) {
        insert(childNode->getKeyValue(i), ((CLeafNode *) childNode)->getData(i));
    }
    setRightSibling(((CLeafNode *) childNode)->m_RightSibling);
    //父节点删除index的key，
    delete childNode;
    parentNode->removeKey(keyIndex + 1, keyIndex + 2);
    CLeafNode *tmp1 = this;
    tree->LeafUpdate(tmp1, getPossession());
    CInternalNode *tmp2 = (CInternalNode *) parentNode;
    tree->InteranlUpdate(tmp2, parentNode->getPossession());
}

void CLeafNode::removeKey(int keyIndex, int childIndex) {
    for (int i = keyIndex; i < getKeyNum() - 1; ++i) {
        setKeyValue(i, getKeyValue(i + 1));
        setData(i, getData(i + 1));
    }
    setKeyNum(getKeyNum() - 1);
    CLeafNode *tmp = this;
    tree->LeafUpdate(tmp, getPossession());
}

void CLeafNode::remove(KeyType key, const DataType &dataValue) {
    int left = 0, mid;
    int right = getKeyNum() - 1;
    while (left <= right) {
        mid = (left + right) >> 1;
        KeyType currentKey = getKeyValue(mid);
        if (strcmp(key.c_str(), currentKey.c_str()) > 0 || (strcmp(key.c_str(), currentKey.c_str()) == 0) && dataValue >
                                                                                                             getData(mid)) {
            left = mid + 1;
        } else {
            if (strcmp(key.c_str(), currentKey.c_str()) < 0 ||
                (strcmp(key.c_str(), currentKey.c_str()) == 0) && dataValue <
                                                                  getData(mid))
                right = mid - 1;
            else break;
        }
    }
    for (int i = mid; i < getKeyNum() - 1; ++i) {
        setKeyValue(i, getKeyValue(i + 1));
        setData(i, getData(i + 1));
    }
    setKeyNum(getKeyNum() - 1);
    CLeafNode *tmp = this;
    tree->LeafUpdate(tmp, getPossession());
}

void CLeafNode::borrowFrom(CNode *siblingNode, CNode *parentNode, int keyIndex, SIBLING_DIRECTION d) {
    switch (d) {
        case LEFT:  // 从左兄弟结点借
        {
            insert(siblingNode->getKeyValue(siblingNode->getKeyNum() - 1),
                   ((CLeafNode *) siblingNode)->getData(siblingNode->getKeyNum() - 1));
            siblingNode->removeKey(siblingNode->getKeyNum() - 1, siblingNode->getKeyNum() - 1);
            parentNode->setKeyValue(keyIndex, getKeyValue(0));
        }
            break;
        case RIGHT:  // 从右兄弟结点借
        {
            insert(siblingNode->getKeyValue(0), ((CLeafNode *) siblingNode)->getData(0));
            siblingNode->removeKey(0, 0);
            parentNode->setKeyValue(keyIndex, siblingNode->getKeyValue(0));
        }
            break;
    }
    CLeafNode *tmp1 = this;
    tree->LeafUpdate(tmp1, getPossession());
    CInternalNode *tmp2 = (CInternalNode *) parentNode;
    tree->InteranlUpdate(tmp2, parentNode->getPossession());
}

int CLeafNode::getChildIndex(KeyType key, int keyIndex) const {
    return keyIndex;
}

CBPlusTree::CBPlusTree() {
    m_Root = NULL;
    m_DataHead = NULL;
}

CBPlusTree::~CBPlusTree() {
    clear();
}

void CBPlusTree::insert(KeyType key, const DataType &data) {
    if (m_Root == NULL) {
        m_Root = new CLeafNode();
        m_DataHead = (CLeafNode *) m_Root;
        CLeafNode *tmp = (CLeafNode *) m_Root;
        CLeafNode_store.write(tmp);
    }
    if (m_Root->getKeyNum() >= MAXNUM_KEY) // 根结点已满，分裂
    {
        CInternalNode *newNode = new CInternalNode();  //创建新的根节点
        newNode->setChild(0, m_Root->getPossession(), m_Root->getType());
        m_Root->split(newNode, 0);    // 叶子结点分裂
        m_Root = newNode;  //更新根节点指针
        CInternalNode_store.write(newNode);
        delete newNode;
    }
    recursive_insert(m_Root, key, data);
}

void CBPlusTree::recursive_insert(CNode *parentNode, KeyType key, const DataType &data) {
    if (parentNode->getType() == LEAF) { // 叶子结点，直接插入
        ((CLeafNode *) parentNode)->insert(key, data);
        CLeafNode *tmp = (CLeafNode *) parentNode;
        CLeafNode_store.update(tmp, parentNode->getPossession());
    } else {
        // 找到子结点
        int keyIndex = parentNode->getKeyIndex(key);
        int childIndex = parentNode->getChildIndex(key, keyIndex); // 孩子结点指针索引
        CNode *childNode;
        if (((CInternalNode *) parentNode)->getChildtype(childIndex) == INTERNAL)
            childNode = ((CInternalNode *) parentNode)->getChild_Internal(childIndex);
        else childNode = ((CInternalNode *) parentNode)->getChild_Leaf(childIndex);
        if (childNode->getKeyNum() >= MAXNUM_LEAF)  // 子结点已满，需进行分裂
        {
            childNode->split(parentNode, childIndex);
            if (strcmp(parentNode->getKeyValue(childIndex).c_str(), key.c_str()) <= 0)   // 确定目标子结点
            {
                if (((CInternalNode *) parentNode)->getChildtype(childIndex + 1) == INTERNAL)
                    childNode = ((CInternalNode *) parentNode)->getChild_Internal(childIndex + 1);
                else childNode = ((CInternalNode *) parentNode)->getChild_Leaf(childIndex + 1);
            }
        }
        recursive_insert(childNode, key, data);
    }
}

void CBPlusTree::clear() {
    if (m_Root != NULL) {
        m_Root->clear();
        m_Root = NULL;
        m_DataHead = NULL;
    }
}

bool CBPlusTree::search(KeyType key) {
    return recursive_search(m_Root, key);
}

bool CBPlusTree::recursive_search(CNode *pNode, KeyType key) const {
    if (pNode == NULL)  //检测节点指针是否为空，或该节点是否为叶子节点
    {
        return false;
    } else {
        int keyIndex = pNode->getKeyIndex(key);
        int childIndex = pNode->getChildIndex(key, keyIndex); // 孩子结点指针索引
        if (keyIndex < pNode->getKeyNum() && strcmp(key.c_str(), pNode->getKeyValue(keyIndex).c_str()) == 0) {
            return true;
        } else {
            if (pNode->getType() == LEAF)   //检查该节点是否为叶子节点
            {
                return false;
            } else {
                return recursive_search(((CInternalNode *) pNode)->getChild_Internal(childIndex), key);
            }
        }
    }
}


void CBPlusTree::remove(KeyType key, DataType &dataValue) {
    if (!search(key))  //不存在
        return;
    if (m_Root->getKeyNum() == 1)//特殊情况处理
    {
        if (m_Root->getType() == LEAF) {
            clear();
        } else {
            CNode *pChild1, *pChild2;
            if (((CInternalNode *) m_Root)->getChildtype(0) == INTERNAL)
                pChild1 = ((CInternalNode *) m_Root)->getChild_Internal(0);
            else pChild1 = ((CInternalNode *) m_Root)->getChild_Leaf(0);
            if (((CInternalNode *) m_Root)->getChildtype(1) == INTERNAL)
                pChild2 = ((CInternalNode *) m_Root)->getChild_Internal(1);
            else pChild2 = ((CInternalNode *) m_Root)->getChild_Leaf(1);
            if (pChild1->getKeyNum() == MINNUM_KEY && pChild2->getKeyNum() == MINNUM_KEY) {
                pChild1->mergeChild(m_Root, pChild2, 0);
                delete m_Root;
                m_Root = pChild1;
            }
        }
    }
    recursive_remove(m_Root, key, dataValue);
}

// parentNode中包含的键值数>MINNUM_KEY
void CBPlusTree::recursive_remove(CNode *parentNode, KeyType key, DataType &dataValue) {
    int keyIndex = parentNode->getKeyIndex(key);
    int childIndex = parentNode->getChildIndex(key, keyIndex); // 孩子结点指针索引
    if (parentNode->getType() == LEAF)// 找到目标叶子节点
    {
        ((CLeafNode *) parentNode)->remove(key, dataValue);  // 直接删除
        // 如果键值在内部结点中存在，也要相应的替换内部结点
        if (childIndex == 0 && m_Root->getType() != LEAF && parentNode != m_DataHead) {
            changeKey(m_Root, key, parentNode->getKeyValue(0));
        }
    } else { // 内结点
        CNode *pChildNode;
        if (((CInternalNode *) parentNode)->getChildtype(childIndex) == LEAF)
            pChildNode = ((CInternalNode *) parentNode)->getChild_Leaf(childIndex);
        else pChildNode = ((CInternalNode *) parentNode)->getChild_Internal(childIndex);
        if (pChildNode->getKeyNum() == MINNUM_KEY) {                       // 包含关键字达到下限值，进行相关操作
            CNode *pLeft;//左兄弟节点
            if (childIndex > 0) {
                if (((CInternalNode *) parentNode)->getChildtype(childIndex) == LEAF)
                    pLeft = ((CInternalNode *) parentNode)->getChild_Leaf(childIndex - 1);
                else pLeft = ((CInternalNode *) parentNode)->getChild_Internal(childIndex - 1);
            } else pLeft = NULL;
            CNode *pRight;//右兄弟节点
            if (childIndex < parentNode->getKeyNum() - 1) {
                if (((CInternalNode *) parentNode)->getChildtype(childIndex) == LEAF)
                    pRight = ((CInternalNode *) parentNode)->getChild_Leaf(childIndex + 1);
                else pRight = ((CInternalNode *) parentNode)->getChild_Internal(childIndex + 1);
            } else pRight = NULL;
            // 先考虑从兄弟结点中借
            if (pLeft && pLeft->getKeyNum() > MINNUM_KEY)// 左兄弟结点可借
            {
                pChildNode->borrowFrom(pLeft, parentNode, childIndex - 1, LEFT);
            } else if (pRight && pRight->getKeyNum() > MINNUM_KEY)//右兄弟结点可借
            {
                pChildNode->borrowFrom(pRight, parentNode, childIndex, RIGHT);
            }
                //左右兄弟节点都不可借，考虑合并
            else if (pLeft)                    //与左兄弟合并
            {
                pLeft->mergeChild(parentNode, pChildNode, childIndex - 1);
                pChildNode = pLeft;
            } else if (pRight)                   //与右兄弟合并
            {
                pChildNode->mergeChild(parentNode, pRight, childIndex);
            }
        }
        recursive_remove(pChildNode, key, dataValue);
    }
}

void CBPlusTree::changeKey(CNode *pNode, KeyType oldKey, KeyType newKey) {
    if (pNode != NULL && pNode->getType() != LEAF) {
        int keyIndex = pNode->getKeyIndex(oldKey);
        if (keyIndex < pNode->getKeyNum() && oldKey == pNode->getKeyValue(keyIndex))  // 找到
        {
            pNode->setKeyValue(keyIndex, newKey);
            if (pNode->getType() == INTERNAL) {
                CInternalNode *tmp = (CInternalNode *) pNode;
                CInternalNode_store.update(tmp, pNode->getPossession());
            } else {
                CLeafNode *tmp = (CLeafNode *) pNode;
                CLeafNode_store.update(tmp, pNode->getPossession());
            }
        } else   // 继续找
        {
            CNode *pChildnode;
            if (((CInternalNode *) m_Root)->getChildtype(0) == INTERNAL)
                pChildnode = ((CInternalNode *) m_Root)->getChild_Internal(0);
            else pChildnode = ((CInternalNode *) m_Root)->getChild_Leaf(0);
            changeKey(pChildnode, oldKey, newKey);
        }
    }
}

vector<DataType> CBPlusTree::Findall(KeyType compareKey) {
    vector<DataType> ans;
    CLeafNode *it = m_DataHead;
    for (; it != NULL; it = it->getRightSibling()) {
        int keyIndex = it->getKeyIndex(compareKey);
        int i;
        for (i = keyIndex; i < it->getKeyNum(); ++i) {
            if (strcmp(compareKey.c_str(), it->getKeyValue(i).c_str()) == 0)
                ans.push_back(it->getData(i));
            else break;
        }
        if (i != it->getKeyNum()) break;
    }
    if (ans.empty()) cout << "null\n";
    else {
        for (auto iter: ans) {
            cout << iter << " ";
        }
        cout << "\n";
    }
    return ans;
}
