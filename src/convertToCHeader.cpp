#include <CImg.h>
#include <stdint.h>
#include <iostream>

using namespace std;
using namespace cimg_library;

int main(int argc, char *argv[]) throw() {
    for (int i = 1; i < argc; i++) {
        CImg<uint8_t> input(argv[i]);
        cout << "#include <stdint.h>" << endl;
        cout << "#include <CImg.h>" << endl;
        cout << "using namespace cimg_library;" << endl;
        cout << "static uint8_t data[] = {\n";
        for (unsigned int i = 0; i < input.size(); i++) {
            cout <<(int) input[i];
            if (i != input.size()-1) cout << ", ";
            if (i % 10 == 9) cout << "\n\t";
        }
        cout << "};\n";
        cout << "\n";
        cout << "CImg<uint8_t> getLogo() throw() {\n";
        cout << "  CImg<uint8_t> logo(" << input.width << ", "
             << input.height << ", " << input.depth << ", " << input.dim << ");\n";
        cout << "  for (unsigned int i = 0; i < logo.size(); i++)\n";
        cout << "    logo[i] = data[i];\n";
        cout << "  return logo;\n}" << endl;
    }
}
