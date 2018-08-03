#include <string.h>
#include <vector>
#include <iostream>

using namespace std;


template <class T>
void swaps(T& a1,T& a2){
    T tmp=a1;
    a1=a2;
    a2=tmp;
}

template <class T>
    
void add(T* v)
{
    for(int i=0; i<v.size(); i++)
    {
        cout << v[i] << " ";
    }
}


int main() {
    int a=10;
    int b=20;
    cout<<"特製化樣板"<<endl;
    cout<<'('<<a<<','<<b<<')'<<endl;
    swaps<int>(a,b);
    cout<<'('<<a<<','<<b<<')'<<endl;
    float c=10;
    float d=20;
    cout<<"一般樣板"<<endl;
    cout<<'('<<c<<','<<d<<')'<<endl;
    swaps<float>(c,d);
    cout<<'('<<c<<','<<d<<')'<<endl;
    
    vector<int> filter_data = {1, 4, 7, 2, 5, 8, 3, 6, 9};
    
    add<vector>(filter_data.data());
    
    
    return 0;
}

