#include <CImg.h>
#include <iostream>
#include <pthread.h>
#include <stdexcept>

using namespace cimg_library;

pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;

void make_frame(int i) {
    if ( i % 2 == 0 ) {
        cimg_library::CImg<uint8_t> i(20,20);
        i.save( "Z:\\Steve\\foo.jpg");
    } else
        make_frame(i/2);
}

void *subthread(void *a) throw() {
    std::cerr << "BOF\n";
    int i = (int)a;
    while (i < 20000) {
        try {
            make_frame( (i) );
        } catch (CImgIOException& e) {
            std::cerr << "Caught\n";
        }
        i++;
    }
    std::cerr << "EOF\n";
    return NULL;
}

int main() throw() {
#ifdef _MT
    std::cerr << "Have MT\n";
#else
    std::cerr << "NO MT\n";
#endif
    pthread_t a, b;
    pthread_create(&a, NULL, &subthread, (void*)5);
    pthread_create(&b, NULL, &subthread, (void*)8);

    try { 
        cimg::exception_mode() = 0U;
        std::cerr << "About to throw exception\n";
        throw cimg_library::CImgIOException("Catched exception");
    } catch (const CImgException& e) {
        std::cerr << "Caught exception\n";
    }
    std::cerr << "Caught no exception\n";

    std::cerr << "Joining subthreads\n";
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    return 0;
}
