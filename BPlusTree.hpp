#pragma once
#include<iostream>
#include<string>
#include<list>
#include<vector>
#include<queue>
#include <utility>
#include<fstream>
#include<sstream>
#include<functional> 
using namespace std;

// key, value类型
typedef int KEY;
typedef int VALUE;

// B+树的阶数
static int DEFAULT_DEGREE = 5;

// 搜索结果类
class SearchResult {
public:
    SearchResult(){}

    SearchResult(bool ex, int se, int in) { exist = ex; searchIndex = se; insertIndex = in; }

    // 判断目标key值是否存在
    bool isExist() { return exist; }

    // 若目标值存在，即为要找的关键字的下标
    int getSearchIndex() { return searchIndex; }

    // 若目标值不存在，即为要插入的位置的下标
    int getInsertIndex() { return insertIndex; }

private:
    bool exist;
    int searchIndex;
    int insertIndex;
};

// 存储元素类（key, value）
class Entry {
public:
    Entry() {}

    Entry(KEY k, VALUE v) { key = k; value = v; }

    void setKey(KEY k) { key = k; }

    KEY getKey() { return key; }

    void setValue(VALUE v) { value = v; }

    VALUE getValue() { return value; }

public:
    KEY key;
    VALUE value;
};

// B+树节点类
class Node {
public:
    Node() {}

    Node(int d) { degree = d; }

    bool getisLeaf() { return isLeaf; }

    void setLeaf(bool leaf);

    vector<Entry*> getEntries() { return entries; }

    void setEntries(vector<Entry*> e) { entries = e; }

    vector<Node*> getNodevector() { return nodevector; }

    void setNodevector(vector<Node*> n) { nodevector = n; }

    Node* getParent() { return parent; }

    void setParent(Node* p) { parent = p; }

    int getNodeIndex() { return nodeIndex; }

    void setNodeIndex(int n) { nodeIndex = n; }

    int getDegree() { return degree; }

    Node* getLeftNode() { 
        return getisLeaf() == true ? 
        nodevector[0] == NULL ? NULL : nodevector[0] :
        NULL; 
    }

    Node* getRightNode() { 
        return getisLeaf() == true ? 
        nodevector[1] == NULL ? NULL : nodevector[1] :
        NULL; 
    }

    void setLeftNode(Node* leftNode) { 
        if (getisLeaf()) { 
            nodevector[0] = leftNode; 
        } 
    }

    void setRightNode(Node* rightNode) { 
        if (getisLeaf()) { 
            nodevector[1] = rightNode; 
        } 
    }

    // key值大小比较函数
    int compare(KEY key1, KEY key2) {
        if (key1 < key2) return -1;
        if (key1 > key2) return 1;
        return 0;
    }

    // 在node中二分查找key值，无论存不存在，都要返回一个index，key存在，返回对应的index
    // key不存在，返回大于key的最小值的索引，若没有大于key的值，则返回最大索引值
    SearchResult* search(KEY key) {
        int left = 0; //左指针
        int right = this->getEntries().size() - 1;//右指针
        int mid = (left + right) / 2;//中间的位置
        bool isExist = false; //判断是否找到了
        int searchIndex = 0; //要找的关键字的下标
        int maxIndex = right;//最大索引值
        while (left <= right) {
            mid = (left + right) / 2;//中间的位置
            Entry* midEntry = this->entries[mid];//拿到中间的关键字
            int comparator = this->compare(key, midEntry->getKey());
            if (comparator == 0) {//找到了
                isExist = true;
                searchIndex = mid;
                break;
            }
            else if (comparator == 1) {//在右侧
                left = mid + 1;
            }
            else {//midKey大于key 在左侧
                right = mid - 1;
            }
        }
        //二分查找结束了
        //B树首次插入节点，会出现left = 0且right = -1的情况
        if (isExist == false) {
            if (right == -1) {
                searchIndex = left;
            }
            else {
                //能走到这里说明left > right
                //选择较大的哪个值，但是left可能会出现大于最大索引值的情况，这时取最大值
                if (left <= maxIndex) {
                    searchIndex = left;
                }
                else {
                    searchIndex = maxIndex;
                }
            }
        }
        vector<Entry*> entries = this->getEntries();

        int insertIndex = -1;
        if (maxIndex == -1) {
            insertIndex = 0;
        }
        else {
            if (this->compare(key, entries[maxIndex]->getKey()) == 1) {
                insertIndex = searchIndex + 1;
            }
            else {
                insertIndex = searchIndex;
            }
        }

        return new SearchResult(isExist, searchIndex, insertIndex);
    }

private:
    bool isLeaf = true;
    Node* parent;
    int nodeIndex;              
    int degree;
    vector<Entry*> entries;
    vector<Node*> nodevector;
};

void Node::setLeaf(bool leaf) {
    isLeaf = leaf;
}

// B+树类
class BPlusTree {
public:
    BPlusTree(int degree = DEFAULT_DEGREE) {
        head = newLeafNode();
        //this->degree = degree % 2 == 0 ? degree + 1 : degree;
        this->degree = degree;
        this->half = (this->degree + 1) / 2;
    }

    // 创建叶节点函数
    Node* newLeafNode() {
        Node* node = new Node(degree);
        node->setLeaf(true);

        vector<Entry*> entries;
        entries.reserve(this->degree + 1);
        node->setEntries(entries);

        vector<Node*> nodeList{ NULL, NULL };
        node->setNodevector(nodeList);

        node->setParent(NULL);
        nodeList.reserve(2);
        node->setNodeIndex(-1);

        return node;
    }

    // 创建非叶节点函数
    Node* newBranchNode() {
        Node* node = new Node(degree);
        node->setLeaf(false);

        vector<Entry*> entries;
        entries.reserve(this->degree + 1);
        node->setEntries(entries);

        vector<Node*> nodeList;
        nodeList.reserve(this->degree + 1);
        node->setNodevector(nodeList);

        node->setParent(NULL);
        node->setNodeIndex(-1);

        return node;
    }

    // 插入
    void insert(KEY key, VALUE val) {
        Node* leafNode = searchNode(key);
        SearchResult* search = leafNode->search(key);
        if (search->isExist()) {
            return;
        }
        else {
            //需要插入的key在树中不存在
            int insertIndex = search->getInsertIndex();
            vector<Entry*> entries = leafNode->getEntries();
            Entry* newEntry = new Entry(key, val);
            entries.insert(entries.begin() + insertIndex, newEntry);
            leafNode->setEntries(entries);
            loopChangeParent(leafNode);
            if (entries.size() <= this->degree) {
                return;
            }
            else {
                splitNode(leafNode);
            }
        }
    }

    // 查找
    VALUE findValue(KEY key) {
        Node* leafNode = searchNode(key);
        SearchResult* search = leafNode->search(key);

        if (search->isExist() == false) {
            cout << "目标值未存储！" << endl;
        }
        else {
            int searchIndex = search->getSearchIndex();
            vector<Entry*> entries = leafNode->getEntries();
            return entries[searchIndex]->getValue();
        }
    }

    // 修改
    void updateValue(KEY key, VALUE value) {
        Node* leafNode = searchNode(key);
        SearchResult* search = leafNode->search(key);

        if (search->isExist() == false) {
            cout << "目标值未存储！" << endl;
        }
        else {
            int searchIndex = search->getSearchIndex();
            vector<Entry*> entries = leafNode->getEntries();
            entries[searchIndex]->setValue(value);
            cout << "目标值已更新完毕！" << endl;
        }
    }

    // 删除
    void deleteKey(KEY key) {
        Node* leafNode = searchNode(key);
        SearchResult* search = leafNode->search(key);
        if (!search->isExist()) {
            return;
        }
        else {
            //需要删除的key在树中存在
            int searchIndex = search->getSearchIndex();
            vector<Entry*> entries = leafNode->getEntries();
            entries.erase(entries.begin() + searchIndex);
            leafNode->setEntries(entries);
            loopChangeParent(leafNode);
            if (entries.size() >= (this->degree + 1) / 2 || leafNode == head) {
                return;
            }
            else {
                borrowAndcombineNode(leafNode);
            }
        }
    }

    /**
     * 层序遍历输出
     *                                 9-18-27
     *                     /               |               \
     *         3-6-9                   12-15-18                     21-24-27
     *     /     |     \           /       |       \           /       |       \
     * 1-2-3  4-5-6  7-8-9  10-11-12  13-14-15  16-17-18  19-20-21  22-23-24  25-26-27
     *
     *    print输出
     *    9-18-27
     *    3-6-9  12-15-18  21-24-27
     *    1-2-3  4-5-6  7-8-9  10-11-12  13-14-15  16-17-18  19-20-21  22-23-24  25-26-27
     */
    void print() {
        queue<pair<Node*, int>> queue;
        vector<pair<Node*, int>> vec;
        stringstream ss; ss << "\n";
        queue.push({ this->head, 0 });
        while (!queue.empty()) {
            auto front = queue.front();
            queue.pop();

            Node* node = front.first;
            int level = front.second;

            vec.push_back(front);

            if (!node->getisLeaf()) {
                for (int i = 0; i < node->getNodevector().size(); i++) {
                    queue.push({ node->getNodevector()[i], level + 1 });
                }
            }
        }
        for (int i = 0; i < vec.size(); i++) {
            pair<Node*, int> pair = vec[i];
            vector<Entry*> entries = pair.first->getEntries();
            if (i > 0) {
                int afterVal = vec[i - 1].second;
                int val = vec[i].second;
                if (afterVal != val) {
                    ss << "\r\n";
                }
                else {
                    ss << "  ";
                }
            }
            for (int j = 0; j < entries.size(); j++) {
                ss << entries[j]->getKey();
                if (j < entries.size() - 1) {
                    ss << "-";
                }
            }
        }
        cout << ss.str() << endl;
    }

    // 判断key索引值是否存在
    bool isExist(KEY key) {
        Node* leafNode = searchNode(key);
        SearchResult* search = leafNode->search(key);
        return search->isExist();
    }

    // 从head开始查找key，找到key对应的叶子节点结束。若为空树，自己造一个叶节点。
    Node* searchNode(KEY key) {
        if (head->getisLeaf()) {
            return head;
        }
        Node* node = head;
        //非叶子节点时循环，跳出循环时，说明找到了叶子节点
        while (!node->getisLeaf()) {
            SearchResult* search = node->search(key);
            int index = search->getSearchIndex();
            node = node->getNodevector()[index];
        }
        return node;
    }

    /**
     * 节点分裂
     * @param node 需要分裂的节点
     *             当node为head节点时 生成newHead和newNode
     *                  head为叶子节点
     *                      1）newNode.setParent(newHead)
     *                      2）half个entry需要移动到newNode
     *                      3）newNode.setLeftNode(node)
     *                      4）分别将node和newNode的最后一个Entry加入到newHead的entries，并且将node，newNode加入到newHead的nodevector中
     *                      5）node.setRightNode(newNode)
     *                  head为非叶子节点
     *                      1）newNode.setParent(newHead)
     *                      2）half个entry需要移动到newNode
     *                      3）half个node需要移动，且它们的父节点要指向newNode，它们的NodeIndex也需要变更
     *             当node不为head节点时 生成newNode
     *                  叶子节点分裂
     *                      1）newNode.setParent(node.getParent())
     *                      2）half个entry需要移动到newNode
     *                      3）如果原本存在右节点，则需完成newNode.setRightNode和rightNode.setLeftNode
     *                      4）newNode.setNodeIndex(node.getNodeIndex() + 1)
     *                  非叶子节点分裂
     *                      1）newNode.setParent(node.getParent())
     *                      2）half个entry需要移动到newNode
     *                      3）half个node需要移动，且它们的父节点要指向newNode，它们的NodeIndex也需要变更
     *
     */
    void splitNode(Node* node) {
        if (node == head) {
            if (node->getisLeaf()) {
                //分裂节点为head，会生成两个新节点
                Node* newNode = newLeafNode();
                Node* newHead = newBranchNode();
                // ===================newNode的赋值过程===================
                newNode->setParent(newHead);
                newNode->setNodeIndex(1);

                vector<Entry*> old_vec;
                vector<Entry*> new_vec;
                for (int i = 0; i < half; i++) {
                    old_vec = node->getEntries();
                    new_vec.insert(new_vec.begin(), old_vec[this->degree - i]);
                    old_vec.pop_back(); old_vec.shrink_to_fit();

                    newNode->setEntries(new_vec);
                    node->setEntries(old_vec);
                }

                newNode->setLeftNode(node);
                //===================newHead节点的赋值过程===================
                //newHead.setParent(null);
                //newHead.setNodeIndex(-1);

                vector<Entry*> head_vec1;
                head_vec1.push_back(node->getEntries()[node->getEntries().size() - 1]);
                head_vec1.push_back(newNode->getEntries()[node->getEntries().size() - 1]);
                newHead->setEntries(head_vec1);

                vector<Node*> head_vec2;
                head_vec2.push_back(node);
                head_vec2.push_back(newNode);
                newHead->setNodevector(head_vec2);
                // ===================head节点的赋值过程===================
                node->setParent(newHead);
                node->setNodeIndex(0);
                node->setRightNode(newNode);
                //===================head重新指向===================
                head = newHead;
            }
            else {
                //分裂节点为head，会生成两个新节点
                Node* newNode = newBranchNode();
                Node* newHead = newBranchNode();
                // ===================newNode的赋值过程===================
                newNode->setParent(newHead);
                newNode->setNodeIndex(1);

                vector<Entry*> old_vec1; vector<Node*> old_vec2;
                vector<Entry*> new_vec1; vector<Node*> new_vec2;
                for (int i = 0; i < half; i++) {
                    old_vec1 = node->getEntries();
                    new_vec1.insert(new_vec1.begin(), old_vec1[this->degree - i]);
                    old_vec1.pop_back(); old_vec1.shrink_to_fit();

                    newNode->setEntries(new_vec1);
                    node->setEntries(old_vec1);


                    old_vec2 = node->getNodevector();
                    new_vec2.insert(new_vec2.begin(), old_vec2[this->degree - i]);
                    old_vec2[this->degree - i]->setParent(newNode);
                    old_vec2.pop_back(); old_vec2.shrink_to_fit();

                    newNode->setNodevector(new_vec2);
                    node->setNodevector(old_vec2);

                }
                changeRightNodeIndex(newNode->getNodevector()[0], 0);
                newNode->setLeftNode(node);

                //===================newHead节点的赋值过程===================
                //newHead.setParent(null);
                //newHead.setNodeIndex(-1);
                vector<Entry*> head_vec1;
                head_vec1.push_back(node->getEntries()[node->getEntries().size() - 1]);
                head_vec1.push_back(newNode->getEntries()[node->getEntries().size() - 1]);
                newHead->setEntries(head_vec1);

                vector<Node*> head_vec2;
                head_vec2.push_back(node);
                head_vec2.push_back(newNode);
                newHead->setNodevector(head_vec2);
                // ===================head节点的赋值过程===================
                node->setParent(newHead);
                node->setNodeIndex(0);
                //===================head重新指向===================
                head = newHead;
            }
        }
        else {
            //分裂节点不为head，只会生成一个节点，这个节点可能时叶子节点，也可能时非叶子节点
            if (node->getisLeaf()) {
                //node为叶子节点，则新生成一个叶子节点
                Node* newNode = newLeafNode();
                // ===================newNode的赋值过程===================
                newNode->setParent(node->getParent());
                newNode->setNodeIndex(node->getNodeIndex() + 1);

                vector<Entry*> old_vec;
                vector<Entry*> new_vec;
                for (int i = 0; i < half; i++) {
                    old_vec = node->getEntries();
                    new_vec.insert(new_vec.begin(), old_vec[this->degree - i]);
                    old_vec.pop_back(); old_vec.shrink_to_fit();

                    newNode->setEntries(new_vec);
                    node->setEntries(old_vec);
                }

                newNode->setLeftNode(node);//完成newNode的左指向
                Node* rightNode = node->getRightNode();

                if (rightNode != NULL) {//如果小于，说明node存在右节点
                    newNode->setRightNode(rightNode);//完成newNode的右指向
                    rightNode->setLeftNode(newNode);
                    //此外从node第一个右节点开始(包括第一个)，所有的右节点的nodeIndex都需要修改
                    if (isRightNodeInCommonParent(node, rightNode)) {
                        //如果rightNode和node在同一个父节点下
                        changeRightNodeIndex(rightNode, node->getNodeIndex() + 2);
                    }
                }
                // ===================node.parent节点的赋值过程===================
                Node* parent = node->getParent();

                vector<Entry*> par_vec1 = parent->getEntries();
                par_vec1[node->getNodeIndex()] = node->getEntries()[node->getEntries().size() - 1];
                par_vec1.insert(par_vec1.begin() + node->getNodeIndex() + 1, newNode->getEntries()[newNode->getEntries().size() - 1]);
                parent->setEntries(par_vec1);

                vector<Node*> par_vec2 = parent->getNodevector();
                par_vec2.insert(par_vec2.begin() + node->getNodeIndex() + 1, newNode);
                parent->setNodevector(par_vec2);
                // ===================node节点的赋值过程===================
                node->setRightNode(newNode);
            }
            else {
                //node为非叶子节点，则新生成一个非叶子节点
                Node* newNode = newBranchNode();
                // ===================叶子节点的赋值过程===================
                newNode->setParent(node->getParent());
                newNode->setNodeIndex(node->getNodeIndex() + 1);

                vector<Entry*> old_vec1; vector<Node*> old_vec2;
                vector<Entry*> new_vec1; vector<Node*> new_vec2;
                for (int i = 0; i < half; i++) {
                    old_vec1 = node->getEntries();
                    new_vec1.insert(new_vec1.begin(), old_vec1[this->degree - i]);
                    old_vec1.pop_back(); old_vec1.shrink_to_fit();

                    newNode->setEntries(new_vec1);
                    node->setEntries(old_vec1);


                    old_vec2 = node->getNodevector();
                    new_vec2.insert(new_vec2.begin(), old_vec2[this->degree - i]);
                    old_vec2[this->degree - i]->setParent(newNode);
                    old_vec2.pop_back(); old_vec2.shrink_to_fit();

                    newNode->setNodevector(new_vec2);
                    node->setNodevector(old_vec2);
                }

                changeRightNodeIndex(newNode->getNodevector()[0], 0);
                // ===================node.parent节点的赋值过程===================
                Node* parent = node->getParent();

                vector<Entry*> par_vec1 = parent->getEntries();
                par_vec1.insert(par_vec1.begin() + node->getNodeIndex(), node->getEntries()[half - 1]);
                parent->setEntries(par_vec1);

                vector<Node*> par_vec2 = parent->getNodevector();
                par_vec2.push_back(newNode);
                parent->setNodevector(par_vec2);

                changeRightNodeIndex(newNode, newNode->getNodeIndex());
            }
            if (node->getParent()->getEntries().size() > this->degree) {
                splitNode(node->getParent());
            }
        }
    }

    // 如果node.entries的最后一个entry和node.parent.entries的最后一个entry不同，则需要变更
    void loopChangeParent(Node* node) {
        if (node->getParent() == NULL) {
            return;
        }
        vector<Entry*> parentEntries = node->getParent()->getEntries();
        vector<Entry*> entries = node->getEntries();
        Entry* parentLastEntry = parentEntries[parentEntries.size() - 1];
        Entry* lastEntry = entries[entries.size() - 1];
        if (parentLastEntry == lastEntry) {
            return;
        }
        parentEntries[node->getNodeIndex()] = lastEntry;
        node->setEntries(entries);
        node->getParent()->setEntries(parentEntries);
        loopChangeParent(node->getParent());
    }

    // 判别rightNode是否是node同一个父节点下的右节点
    bool isRightNodeInCommonParent(Node* node, Node* rightnode) {
        if (!node->getisLeaf()) {
            cout << "it is not leafNode, can\'t identify" << endl;
        }
        if (node->getNodeIndex() == node->getParent()->getNodevector().size() - 1) {
            return false;
        }
        return node->getParent()->getNodevector()[node->getNodeIndex() + 1] == rightnode;
    }

    // 判别leftNode是否是node同一个父节点下的左节点
    bool isLeafNodeInCommonParent(Node* node, Node* leftnode) {
        if (!node->getisLeaf()) {
            cout << "it is not leafNode, can\'t identify" << endl;
        }
        if (node->getNodeIndex() == 0) {
            return false;
        }
        return node->getParent()->getNodevector()[node->getNodeIndex() - 1] == leftnode;
    }

    /**
     * 设置node在parent中的nodeIndex，此外在同一个parent下的所有右兄弟节点的nodeIndex都要从startNodeIndex开始依次+1
     * 例如
     *        K0-》index：0    K1-》index：1    K2-》index：2    K3-》index：3
     *        K1节点被删除了，那么再执行subRightNodeIndex(K2, 1)
     *        ===》
     *        K0-》index：0    K2-》index：1    K3-》index：2
     *        也就是说将
     *          k2.nodeIndex = nodeIndex;
     *          K3.nodeIndex = nodeIndex + 1;
     *
     * @param node
     */
    void changeRightNodeIndex(Node* node, int startNodeIndex) {
        vector<Node*> parentNodevector = node->getParent()->getNodevector();
        int nodeIndex = -1;
        for (int i = 0; i < parentNodevector.size(); i++) {
            if (node == parentNodevector[i]) {
                nodeIndex = i;
                break;
            }
        }

        if (nodeIndex == -1) {
            cout << "nodeIndex = -1" << endl;
        }

        for (int i = nodeIndex; i < parentNodevector.size(); i++) {
            Node* rightNode = parentNodevector[i];
            rightNode->setNodeIndex(startNodeIndex++);
        }
    }

    /**
     * 从node开始借entry，和合并
     * @param node
     */
    void borrowAndcombineNode(Node* node) {
        Node* leftNode = getLeftNodeInCommonParent(node);
        Node* rightNode = getRightNodeInCommonParent(node);
        if (node->getisLeaf()) {
            if (leftNode != NULL && leftNode->getEntries().size() > half) {//找左借
                vector<Entry*> now_vec = node->getEntries();
                vector<Entry*> left_vec = leftNode->getEntries();

                now_vec.insert(now_vec.begin(), left_vec[left_vec.size() - 1]);
                left_vec.pop_back();

                node->setEntries(now_vec);
                leftNode->setEntries(left_vec);

                loopChangeParent(leftNode);
            }
            else if (rightNode != NULL && rightNode->getEntries().size() > half) {//找右借
                vector<Entry*> now_vec = node->getEntries();
                vector<Entry*> right_vec = rightNode->getEntries();

                now_vec.push_back(right_vec[0]);
                right_vec.erase(right_vec.begin());

                node->setEntries(now_vec);
                rightNode->setEntries(right_vec);

                loopChangeParent(node);//右借之后，node.entries的最后一个元素变了
            }
            else {//左右都不能借，那只能合并了
                combineNode(node);
            }
        }
    }

    /**
     * 得到node同一个parent下的leftNode
     * @param node
     * @return
     */
    Node* getLeftNodeInCommonParent(Node* node) {
        if (node->getParent() == NULL || node == node->getParent()->getNodevector()[0]) {
            return NULL;
        }
        return node->getParent()->getNodevector()[node->getNodeIndex() - 1];
    }

    /**
     * 得到node同一个parent下的rightNode
     * @param node
     * @return
     */
    Node* getRightNodeInCommonParent(Node* node) {
        if (node->getParent() == NULL || node == node->getParent()->getNodevector()[node->getParent()->getNodevector().size() - 1]) {
            return NULL;
        }
        return node->getParent()->getNodevector()[node->getNodeIndex() + 1];
    }

    /**
     * 当这个方法执行的时候，一定存在叶子节点的合并，可能存在非叶子节点的合并
     * 节点合并
     *      叶子节点的合并
     *           合并到左节点中(同一个父节点)，父节点的entries.size和nodevector.size()都要-1，层数是否降低
     *           合并到右节点中(同一个父节点)，父节点的entries.size和nodevector.size()都要-1，层数是否降低
     *      非叶子节点
     *          非叶子节点的不合并（当node.entries.size() >= half）
     *          非叶子节点的合并（当node.entries.size() < half）
     *              合并到左节点中(同一个父节点)，父节点的entries.size和nodevector.size()都要-1，层数是否降低
     *              合并到右节点中(同一个父节点)，父节点的entries.size和nodevector.size()都要-1，层数是否降低
     * @param node
     */
    void combineNode(Node* node) {
        if (node->getEntries().size() >= half || node == head) {
            return;
        }
        Node* parent = node->getParent();
        Node* leftNode = getLeftNodeInCommonParent(node);//同一个父节点中的左节点
        Node* rightNode = getRightNodeInCommonParent(node);//同一个父节点中的右节点
        if (node->getisLeaf()) {
            Node* leftRealNode = node->getLeftNode();//node的左节点
            Node* rightRealNode = node->getRightNode();//node的右节点
            if (leftNode != NULL) {//左节点都不为空，元素移动到左节点
                //===================node节点的赋值过程===================
                node->setParent(NULL);

                vector<Entry*> now_vec;
                vector<Entry*> left_vec;
                for (int i = 0; i < half - 1; i++) {
                    left_vec = leftNode->getEntries();
                    now_vec = node->getEntries();

                    left_vec.push_back(now_vec[0]);
                    now_vec.erase(now_vec.begin());

                    leftNode->setEntries(left_vec);
                    node->setEntries(now_vec);
                }
                node->setLeftNode(NULL);
                node->setRightNode(NULL);
                //===================parent节点的赋值过程===================
                vector<Entry*> par_vec1 = parent->getEntries();
                vector<Node*> par_vec2 = parent->getNodevector();

                par_vec1.erase(par_vec1.begin() + node->getNodeIndex() - 1);
                par_vec2.erase(par_vec2.begin() + node->getNodeIndex());

                parent->setEntries(par_vec1);
                parent->setNodevector(par_vec2);
                //===================左右节点的赋值过程===================
                if (rightRealNode != NULL) {
                    leftNode->setRightNode(rightRealNode);
                    rightRealNode->setLeftNode(leftNode);
                }
                else {
                    leftRealNode->setRightNode(NULL);
                }
                if (rightNode != NULL) {
                    changeRightNodeIndex(rightNode, rightNode->getNodeIndex() - 1);
                }
                if (canOrNotReduceLayers(leftNode)) { return; }
            }
            else {//左节点为空，右节点不为空，元素移动到右节点
             //===================node节点的赋值过程===================
                node->setParent(NULL);

                vector<Entry*> now_vec;
                vector<Entry*> right_vec;
                for (int i = 0; i < half - 1; i++) {
                    right_vec = rightNode->getEntries();
                    now_vec = node->getEntries();

                    right_vec.insert(right_vec.begin(), now_vec[now_vec.size() - 1]);
                    now_vec.erase(now_vec.begin() + now_vec.size() - 1);

                    rightNode->setEntries(right_vec);
                    node->setEntries(now_vec);
                }
                node->setLeftNode(NULL);
                node->setRightNode(NULL);
                //===================parent节点的赋值过程===================
                vector<Entry*> par_vec1 = parent->getEntries();
                vector<Node*> par_vec2 = parent->getNodevector();

                par_vec1.erase(par_vec1.begin() + node->getNodeIndex());
                par_vec2.erase(par_vec2.begin() + node->getNodeIndex());

                parent->setEntries(par_vec1);
                parent->setNodevector(par_vec2);
                //===================左右节点的赋值过程===================
                if (leftRealNode != NULL) {
                    rightNode->setLeftNode(leftRealNode);
                    leftRealNode->setRightNode(rightNode);
                }
                else {
                    rightNode->setLeftNode(NULL);
                }
                changeRightNodeIndex(rightNode, rightNode->getNodeIndex() - 1);
                if (canOrNotReduceLayers(rightNode)) {
                    return;
                }
            }
        }
        else {//合并的节点为非叶子节点
            if (leftNode != NULL && leftNode->getEntries().size() > half) {//从左边借
                leftBorrow(node);
            }
            else if (rightNode != NULL && rightNode->getEntries().size() > half) {//从右边借
                rightBorrow(node);
            }
            else {//两边都不能借，只能合并
             //优先找左边合并
                if (node->getNodeIndex() > 0) {//左旋，和左节点合并
                    leftRotate(node);
                    if (canOrNotReduceLayers(leftNode)) {
                        return;
                    }
                }
                else {//右旋，和右节点合并
                    rightRotate(node);
                    if (canOrNotReduceLayers(rightNode)) {
                        return;
                    }
                }
            }
        }
        combineNode(parent);
    }

    /**
     * @param node 节点合并完成判断是否需要降层数，每当层数降低之后。说明树结构已经符合B+树定义(已经稳定)
     */
    bool canOrNotReduceLayers(Node* node) {
        if (node->getParent()->getEntries().size() == 1 && node->getParent() == head) {
            //head的设置
            vector<Entry*> head_vec1 = head->getEntries();
            vector<Node*> head_vec2 = head->getNodevector();

            head_vec1.erase(head_vec1.begin());
            head_vec2.erase(head_vec2.begin());

            head->setEntries(head_vec1);
            head->setNodevector(head_vec2);

            //node的设置
            node->setParent(NULL);
            node->setNodeIndex(-1);
            if (node->getNodevector()[0] == NULL) {
                node->setLeaf(true);
            }
            head = node;
            return true;
        }
        return false;
    }

    /**
     * 左借
     * @param node node.getNodevector().size() < half的节点
     */
    void leftBorrow(Node* node) {
        Node* parent = node->getParent();
        Node* leftNode = parent->getNodevector()[node->getNodeIndex() - 1];
        vector<Entry*> entries = node->getEntries();
        vector<Entry*> left_entries = leftNode->getEntries();

        entries.insert(entries.begin(), leftNode->getEntries()[leftNode->getEntries().size() - 1]);
        left_entries.pop_back();

        node->setEntries(entries);
        leftNode->setEntries(left_entries);

        vector<Node*> nodevector = node->getNodevector();
        vector<Node*> left_node = leftNode->getNodevector();
        Node* leftNodeLastNode = left_node[leftNode->getNodevector().size() - 1];

        nodevector.insert(nodevector.begin(), leftNodeLastNode);
        left_node.pop_back();
        
        node->setNodevector(nodevector);
        leftNode->setNodevector(left_node);

        leftNodeLastNode->setParent(node);
        leftNodeLastNode->setNodeIndex(0);
        changeRightNodeIndex(node->getNodevector()[1], 1);
        loopChangeParent(leftNode);
    }

    /**
     * 右借
     * @param node node.getNodevector().size() < half的节点
     */
    void rightBorrow(Node* node) {
        Node* parent = node->getParent();
        Node* rightNode = parent->getNodevector()[node->getNodeIndex() + 1];
        vector<Entry*> entries = node->getEntries();
        vector<Entry*> right_entries = rightNode->getEntries();

        entries.push_back(right_entries[0]);
        right_entries.erase(right_entries.begin());

        node->setEntries(entries);
        rightNode->setEntries(right_entries);

        vector<Node*> nodevector = node->getNodevector();
        vector<Node*> right_node = rightNode->getNodevector();
        Node* rightNodeFirstNode = right_node[0];

        nodevector.push_back(rightNodeFirstNode);
        right_node.erase(right_node.begin());

        node->setNodevector(nodevector);
        rightNode->setNodevector(right_node);

        rightNodeFirstNode->setParent(node);
        rightNodeFirstNode->setNodeIndex(nodevector.size() - 1);
        changeRightNodeIndex(rightNode->getNodevector()[0], 0);
        loopChangeParent(node);
    }

    // 判断同一父节点下是否存在右兄弟
    bool isExistRightNodeInCommonParent(Node* node) {
        return node->getNodeIndex() < node->getParent()->getNodevector().size() - 1;
    }

    // 判断同一父节点下是否存在左兄弟
    bool isExistLeftNodeInCommonParent(Node* node) {
        return node->getNodeIndex() > 0;
    }

    /**
     * 左旋，和左节点合并
     * @param node node.getNodevector().size() < half的节点
     */
    void leftRotate(Node* node) {
        Node* parent = node->getParent();
        Node* leftNode = parent->getNodevector()[node->getNodeIndex() - 1];
        Node* rightNode = NULL;
        if (isExistRightNodeInCommonParent(node)) {
            rightNode = parent->getNodevector()[node->getNodeIndex() + 1];
        }

        vector<Entry*> now_vec1; vector<Node*> now_vec2;
        vector<Entry*> left_vec1; vector<Node*> left_vec2;
        for (int i = 0; i < half - 1; i++) {
            now_vec1 = node->getEntries();
            left_vec1 = leftNode->getEntries();

            left_vec1.push_back(now_vec1[0]);
            now_vec1.erase(now_vec1.begin());

            node->setEntries(now_vec1);
            leftNode->setEntries(left_vec1);

            now_vec2 = node->getNodevector();
            left_vec2 = leftNode->getNodevector();
            now_vec2[0]->setParent(leftNode);
            now_vec2[0]->setNodeIndex(left_vec2[left_vec2.size() - 1]->getNodeIndex() + 1);
            left_vec2.push_back(now_vec2[0]);
            now_vec2.erase(now_vec2.begin());

            node->setNodevector(now_vec2);
            leftNode->setNodevector(left_vec2);
        }
        if (isExistRightNodeInCommonParent(node)) {
            changeRightNodeIndex(rightNode, rightNode->getNodeIndex() - 1);
        }
        node->setParent(NULL);

        vector<Entry*> par_vec1 = parent->getEntries(); 
        vector<Node*> par_vec2 = parent->getNodevector();
        
        par_vec1.erase(par_vec1.begin() + node->getNodeIndex() - 1);
        par_vec2.erase(par_vec2.begin() + node->getNodeIndex());

        parent->setEntries(par_vec1);
        parent->setNodevector(par_vec2);

        loopChangeParent(leftNode);
    }

    /**
     * 右旋，和右节点合并
     * @param node node.getNodevector().size() < half的节点
     */
    void rightRotate(Node* node) {
        Node* parent = node->getParent();
        vector<Node*> parentNodevector = parent->getNodevector();
        Node* rightNode = parentNodevector[node->getNodeIndex() + 1];

        vector<Entry*> now_vec1; vector<Node*> now_vec2;
        vector<Entry*> right_vec1; vector<Node*> right_vec2;
        for (int i = 0; i < half - 1; i++) {
            now_vec1 = node->getEntries();
            right_vec1 = rightNode->getEntries();

            right_vec1.insert(right_vec1.begin(),now_vec1[node->getEntries().size() - 1]);
            now_vec1.erase(now_vec1.begin() + now_vec1.size() - 1);

            node->setEntries(now_vec1);
            rightNode->setEntries(right_vec1);

            now_vec2 = node->getNodevector();
            right_vec2 = rightNode->getNodevector();

            now_vec2[now_vec2.size() - 1]->setParent(rightNode);
            right_vec2.insert(right_vec2.begin(), now_vec2[now_vec2.size() - 1]);
            now_vec2.erase(now_vec2.begin() + now_vec2.size() - 1);
            
            node->setNodevector(now_vec2);
            rightNode->setNodevector(right_vec2);
        }
        changeRightNodeIndex(rightNode->getNodevector()[0], 0);
        changeRightNodeIndex(rightNode, rightNode->getNodeIndex() - 1);
        node->setParent(NULL);

        vector<Entry*> par_vec1 = parent->getEntries();
        vector<Node*> par_vec2 = parent->getNodevector();

        par_vec1.erase(par_vec1.begin() + node->getNodeIndex());
        par_vec2.erase(par_vec2.begin() + node->getNodeIndex());

        parent->setEntries(par_vec1);
        parent->setNodevector(par_vec2);
    }

    vector<VALUE> selectGreaterAndEqual(KEY key) {
        Node* leafNode = searchNode(key);
        SearchResult* search = leafNode->search(key);
        vector<VALUE> resvector;
        if (search->isExist()) {
            int insertIndex = search->getInsertIndex();
            vector<Entry*> entries = leafNode->getEntries();
            for (int i = insertIndex; i < entries.size(); i++) {
                resvector.push_back(entries[i]->getValue());
            }
            while (leafNode->getRightNode() != NULL) {
                Node* rightNode = leafNode->getRightNode();
                vector<Entry*> rightNodeEntries = rightNode->getEntries();
                for (int i = 0; i < rightNodeEntries.size(); i++) {
                    resvector.push_back(rightNodeEntries[i]->getValue());
                }
                leafNode = rightNode;
            }
        }
        return resvector;
    }

    vector<VALUE> selectLessAndEqual(KEY key) {
        Node* leafNode = searchNode(key);
        SearchResult* search = leafNode->search(key);
        vector<VALUE> resvector;
        if (search->isExist()) {
            int insertIndex = search->getInsertIndex();
            vector<Entry*> entries = leafNode->getEntries();
            for (int i = insertIndex; i >= 0; i--) {
                resvector.push_back(entries[i]->getValue());
            }
            while (leafNode->getLeftNode() != NULL) {
                Node* leftNode = leafNode->getLeftNode();
                vector<Entry*> leftNodeEntries = leftNode->getEntries();
                for (int i = leftNodeEntries.size() - 1; i >= 0; i--) {
                    resvector.push_back(leftNodeEntries[i]->getValue());
                }
                leafNode = leftNode;
            }
        }
        return resvector;
    }

private:
    Node* head;
    int degree;
    int half;
};

