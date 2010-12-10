#include <data-c++/Vector.h>
#include <cassert>
#include <iostream>

using namespace data_cpp;
using namespace std;

int check_erase() throw() {
    Vector<int> vector;

    for (int i = 0; i < 10; i++) {
        vector.push_back(i);
    }

    assert( vector.size() == 10 );
    for (int i = 0; i < 10; i++)
        assert( vector[i] == i );

    for (Vector<int>::iterator i = vector.begin(); i!= vector.end(); )
        if (*i % 2 == 1)
            i = vector.erase(i);
        else
            i++;

    assert( vector.size() == 5 );
    for (int i = 0; i < 5; i++)
        assert( vector[i] == 2 * i );
    return 0;
}

int main() throw() {
    return check_erase();
}
