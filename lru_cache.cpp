#include "lru_cache.h"

//  Node Methods
Node::Node(int k,int v){
    key=k;
    val=v;
    prev=nullptr;
    next=nullptr;
}

//  LRUcache Methods
LRUcache::LRUcache(int c,MYSQL *dbconn){
    cap=c;
    conn=dbconn;
    // Initialize dummy nodes
    head=new Node(-1,-1);
    tail=new Node(-1,-1);
    head->next=tail;
    tail->prev=head;
}

LRUcache::~LRUcache(){
    Node* curr=head;
    while(curr!=nullptr){
        Node* nextNode=curr->next;
        delete curr;
        curr=nextNode;
    }
    cout<<YELLOW<<"Memory freed. Server shutdown."<<RESET<<"\n";
}

// Inserts a new node
void LRUcache::addNode(Node* newNode){
    Node* temp=head->next;
    newNode->next=temp;
    newNode->prev=head;
    head->next=newNode;
    temp->prev=newNode;
}
//  Remove an existing node
void LRUcache::delNode(Node* delNode){
    Node* delprev=delNode->prev;
    Node* delnext=delNode->next;
    delprev->next=delnext;
    delnext->prev=delprev;
}

//  Retrieves data
int LRUcache::get(int key){
    if(cachemap.find(key)!=cachemap.end()){
        Node* resNode=cachemap[key];
        int res=resNode->val;
        // Refresh position to MRU
        cachemap.erase(key);
        delNode(resNode);
        addNode(resNode);
        cachemap[key]=head->next;
        cout<<GREEN<<"cache hit(RAM):key-"<<key<<RESET<<"\n";
        return res;
    }
    return fetchDB(key);
}

//  Adds new data
void LRUcache::put(int key,int val){
    //if already exists, delete old version
    if(cachemap.find(key)!=cachemap.end()){
        Node* extNode=cachemap[key];
        cachemap.erase(key);
        delNode(extNode);
    }
    //if cache full, delete LRU
    if(cachemap.size()==cap){
        Node* lru=tail->prev;
        cachemap.erase(lru->key);
        delNode(lru);
        delete lru; //Free RAM
    }
    addNode(new Node(key,val));
    cachemap[key]=head->next;
    syncDB(key,val);
}

void LRUcache::syncDB(int key,int val){
    string query="INSERT INTO cache_data (cache_key,cache_value) VALUES("+to_string(key)+","+to_string(val)+") ON DUPLICATE KEY UPDATE cache_value = "+ to_string(val);
    mysql_query(conn,query.c_str());
}

// Handles cache miss
int LRUcache::fetchDB(int key){
    string query="SELECT cache_value FROM cache_data WHERE cache_key = "+to_string(key);
    mysql_query(conn,query.c_str());
    MYSQL_RES *res=mysql_store_result(conn);
    MYSQL_ROW row=mysql_fetch_row(res);
    if(row){
        int dbval=stoi(row[0]);
        mysql_free_result(res);
        cout<<YELLOW<<"Cache miss! Recovered from DB: key-"<<key<<RESET<<"\n";
        //puts back in RAM
        this->put(key,dbval);
        return dbval;
    }
    mysql_free_result(res);
    return -1;
}

// DB setup
MYSQL* setupDatabase(){
    MYSQL *conn=mysql_init(NULL);
    const char* dbPass=getenv("DB_PASS");
    if(dbPass==nullptr){
        cout<<"Error:DB_PASS environment variable not set!";
        exit(1);
    }
    if(!mysql_real_connect(conn,"localhost","root",dbPass,"cachedb",3306,NULL,0)){
        cout<<"Connection Failed"<<"\n";
        exit(1);
    }
    cout<<"Connection Successful\n";
    return conn;
}

