
#include<iostream>
#include<stdio.h>
#include"BPlusTree.hpp"
using namespace std;


int main() {
    BPlusTree* tree = new BPlusTree(3);

    cout << "开始建立一个B+树" << endl;
    for (int i = 0; i <= 100; i+=10) {
        cout << "插入 " << i << " 之后: ";
        tree->insert(i, i);
        tree->print();
    }
    cout << "B+树建立完毕" << endl;
    cout << endl << endl;
    
    bool flag = true;
    while (flag) {
        // 交互框
        cout << "请输入您的操作" << endl;
        cout << "1. insert" << endl;
        cout << "2. delete" << endl;
        cout << "3. find" << endl;
        cout << "4. update" << endl;
        cout << "5. over" << endl;
        cout << endl;

        string command;
        cin >> command;

        int key, value;
        switch (command[0]) {
        case 'i':
            cout << "请输入您要插入的key、value值" << endl;
            cin >> key >> value;
            tree->insert(key, value);
            tree->print();
            break;

        case 'd':
            cout << "请输入您要删除的key键" << endl;
            cin >> key;
            tree->deleteKey(key);
            tree->print();
            break;

        case 'f':
            cout << "请输入您要查找的key键" << endl;
            cin >> key;
            cout << tree->findValue(key) << endl;
            break;

        case 'u':
            cout << "请输入您要更新的key、value值" << endl;
            cin >> key >> value;
            tree->updateValue(key, value);
            cout << "更新成功" << endl;
            break;
        case 'o':
        default:
            flag = false;
            break;
        }

        cout << endl << endl;
    }


    //1. 插入操作 
    //tree->insert(5, 5);   
    //tree->print();

    //tree->insert(6, 6);
    //tree->print();


    //tree->insert(110, 110);
    //tree->print();

    //tree->insert(31, 31);
    //tree->print();

    //tree->insert(35, 35);
    //tree->print();


    //2. 查找操作（key）
    //cout << tree->findValue(6) << endl;
    //cout << tree->findValue(16) << endl;
    //cout << tree->findValue(26) << endl;
    //cout << tree->findValue(36) << endl;

    //vector<VALUE> Gvec = tree->selectGreaterAndEqual(10);
    //for (int i = 0; i < Gvec.size(); i++) {
    //    cout << Gvec[i] << " ";
    //}
    //cout << endl;

    //vector<VALUE> Lvec = tree->selectLessAndEqual(25);
    //for (int i = Lvec.size() - 1; i >= 0; i--) {
    //    cout << Lvec[i] << " ";
    //}
    //cout << endl;


    //3.修改操作
    //tree->updateValue(6, 666);
    //tree->updateValue(16, 666);
    //tree->updateValue(26, 666);
    //tree->updateValue(36, 666);

    //cout << tree->findValue(6) << endl;
    //cout << tree->findValue(16) << endl;
    //cout << tree->findValue(26) << endl;
    //cout << tree->findValue(36) << endl;


    //4. 删除操作（key）
    //tree->deleteKey(60);
    //tree->print();

    //tree->deleteKey(6);
    //tree->print();

    //tree->deleteKey(10);
    //tree->print();

    //tree->deleteKey(5);
    //tree->print();

    //tree->deleteKey(90);
    //tree->print();

    //tree->deleteKey(31);
    //tree->print();

    //tree->deleteKey(50);
    //tree->print();

    //tree->deleteKey(70);
    //tree->print();

    //tree->deleteKey(20);
    //tree->print();

    //tree->deleteKey(2);
    //tree->print();

    //for (int i = 0; i <= 50; i++) {
    //    cout << "删除 " << 50 - i << " 之后: ";
    //    tree->deleteKey(50 - i);
    //    tree->print();
    //}

	return 0;
}

