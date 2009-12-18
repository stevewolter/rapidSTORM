#include <data-c++/List.h>
#include <iostream>
#include <algorithm>

using namespace data_cpp;
using namespace std;

void aoutput(const int& a) { cout << a << " "; }

template <typename T>
ostream& operator<<(ostream&o, const List<T>& l) {
    for_each(l.begin(), l.end(), &aoutput);
    return o;
}

int main() {
    List<int>::Allocator allocator;
    List<int> list_a(allocator), list_b(allocator);
    
    for (int i= 0; i < 10; i++)
        list_a.push_back(i); 
    for (int i= 21; i < 30; i++)
        list_b.push_back(i); 

    List<int>::splice(list_a.end(), list_b.begin(), list_b.end());
    cout << (list_a) << endl;
    cout << (list_b) << endl;
    return 0;
}
