#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include<bits/stdc++.h>
#include<mysql.h>
#include<windows.h>
using namespace std;

// ANSI Escape codes for terminal UI
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define HIGHLIGHT "\033[47;30m"

// single entry in cache
class Node {
public:
    int key,val;
    chrono::steady_clock::time_point ex;  //Expiration Timestamp
    Node* prev;
    Node* next;
    Node(int k,int v,int ttl=60);
};

// LRU eviction and MYSQL fallback
class LRUcache {
private:
    // Lock mechanism
    recursive_mutex mtx;  //protects RAM
    mutex dbmx;           //protects MySQL connection
    int cap;
    unordered_map<int,Node*> cachemap;  //Maps key->loc
    Node* head;     //Dummy head->points most recent
    Node* tail;     //Dummy tail->points least recent
    MYSQL *conn; 
    void addNode(Node* newNode);
    void delNode(Node* delNode);

public:
    bool ram_flag;  //cache hit/miss->(T/F)
    LRUcache(int c,MYSQL *dbconn);
    ~LRUcache();    //Destructor
    int get(int key);
    void put(int key,int val,int ttl=60);
    void syncDB(int key,int val);
    int fetchDB(int key);
};

MYSQL* setupDatabase();
#endif

