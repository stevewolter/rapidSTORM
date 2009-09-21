#include "../src/ImageLoader.h"
#include <iostream>

using namespace std;
using namespace dStorm;

int main(int argc, char *argv[]) {
   for (int i = 1; i < argc; i++) {
      SIFLoader sv(argv[i]);
      cout << sv.image_size().w * sv.image_size().h * sv.image_quantity() << " " << sv.image_quantity() << endl;
   }
   return 0;
}
