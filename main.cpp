#include "lru_cache.h"

int main(){
    cout<<"Startig MiniRedis Server..."<<"\n";
    MYSQL *conn=setupDatabase();

    int capacity;
    cout<<"Enter cache capacity: ";
    cin>>capacity;

    LRUcache cache(capacity,conn);
    int choice,key,val;

    // Interactive server loop
    while(true){
        cout<<"\n   --MiniRedis Menu--\n";
        cout<<"1. PUT data\n2. GET data\n3. EXIT\nChoice: ";
        cin>>choice;

        // Input validation->prevents infinite loops
        if(cin.fail()){
            cin.clear();
            cin.ignore(1000,'\n');
            cout<<RED<<"Invalid input"<<RESET<<"\n";
            continue;
        }
        if(choice==1){
            cout<<"Enter key & value: ";
            cin>>key>>val;
            if(cin.fail()){
            cin.clear();
            cin.ignore(1000,'\n');
            cout<<RED<<"Invalid input"<<RESET<<"\n";
            continue;
            }
            cache.put(key,val);
            cout<<"Saved ("<<key<<","<<val<<")\n";
        }
        else if(choice==2){
            cout<<"Enter Key to fetch: ";
            cin>>key;

            auto start=chrono::high_resolution_clock::now();    //Timer starts
            int r=cache.get(key);
            auto end=chrono::high_resolution_clock::now();      //Timer stops
            auto duration=chrono::duration_cast<chrono::microseconds>(end-start);

            if(r==-1)cout<<RED<<"!Key not found"<<RESET<<"\n";
            else{cout<<"Result: "<<r<<"\n";}
            cout<<HIGHLIGHT<<"Time taken: "<<duration.count()<<" micro-secs"<<RESET<<"\n";
        }
        else if(choice==3){
            cout<<"Shutting down server...\n";
            break;
        }
        else{cout<<"Invalid choice\n";}
    }

    mysql_close(conn);
    return 0;
}