#include "ColorImage.h"

using namespace std;
using namespace dStorm;
using namespace cimg_library;

ColorImage::ColorImage(const dStorm::Image& i,
   int xl, int xh, int yl, int yh) throw()
: CImg<StormPixel>(i.width, i.height, 1, 3) ,
  min_grey_value(i.min()),
  max_grey_value(i.max())
{
   if (xl == -1) xl = 0;
   if (xh == -1) xh = i.width-1;
   if (yl == -1) yl = 0;
   if (yh == -1) yh = i.height-1;
   xo = xl;
   yo = yl;

   for (int d = 0; d < 3; d++) {
      for (int y = yl; y <= yh; y++)
         memcpy(ptr(xl, y, 0, d), i.ptr(xl, y), 
               (xh-xl+1) * sizeof(StormPixel));
   }

   for (int j = 0; j < 8; j++) {
      colours[j][0] = ((j & 0x1) == 0) ? min_grey_value : max_grey_value;
      colours[j][1] = ((j & 0x2) == 0) ? min_grey_value : max_grey_value;
      colours[j][2] = ((j & 0x4) == 0) ? min_grey_value : max_grey_value;
   }
}

void ColorImage::mark(const Spot &s, int colour) throw() {
   int x = xo + s.x();
   int y = yo + s.y();
   draw_rectangle(std::max<int>(0, x-4), 
                  std::max<int>(0, y-4),
                  std::min<int>(width-1, x+4),
                  std::min<int>(height-1, y+4),
                  colours[colour], 1.0, ~0U);
}
