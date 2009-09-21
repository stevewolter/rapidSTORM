#include <iostream>
using namespace std;
#include <data-c++/VectorList.h>

using namespace data_cpp;
using namespace std;

int main() throw() {
    VectorList<int> lv;

    for (int i = 0; i < 200000; i++) {
        lv.push_back(i);
    }

    int should = 0;
    for (VectorList<int>::const_iterator i = lv.begin(); i != lv.end(); i++)
        if ( *i == should )
            should++;
        else {
            cerr << "Mismatch: Expected " << should << ", got " << *i << endl;
            return 1;
        }
    return 0;
}
