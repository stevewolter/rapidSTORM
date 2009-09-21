#include <exception>
#include <iostream>

void testException();

int main()
{
    try {
        testException();
    } catch (const std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
