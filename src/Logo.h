#include <CImg.h>
#include <cc++/thread.h>

extern cimg_library::CImg<uint8_t> getLogo() throw();

namespace cimg_library {
    class Logo : private ost::Thread {
    private:
        bool stop;
    public:
        Logo() throw() : ost::Thread("Logo"), stop(false) { start(); }
        ~Logo() throw() { stop = true; }

        void run() throw() {
            CImg<uint8_t> realLogo = getLogo();
            int resPerc = (int)(
                ((double(CImgDisplay::screen_dimx())/2) / realLogo.width)
                    * 100);
            CImg<uint8_t> logo = realLogo.get_resize(-resPerc,-resPerc);
            try {
                CImgDisplay display(logo, "rapidSTORM splash screen");
                for (int i = 0; !stop && i < 10; i++)
                    cimg::sleep(500);
            } catch (const CImgException& e) {}
        }
    };
}
