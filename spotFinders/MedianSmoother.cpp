#include <iostream>
#include <algorithm>
#include <cassert>

#include <simparm/BoostUnits.h>
#include <simparm/Object.h>
#include <simparm/Entry.h>

#include "engine/SpotFinder.h"
#include "engine/SpotFinderBuilder.h"
#include "engine/Image.h"
#include <simparm/GUILabelTable.h>

namespace dStorm {
namespace median_smoother {

struct Config {
    simparm::Entry< quantity<camera::length,int> > mask_size;
    void attach_ui( simparm::NodeHandle at ) { mask_size.attach_ui( at ); }
    Config() 
      : mask_size("SmoothingMaskSize", 5 * camera::pixel) {}
    static std::string get_name() { return "Median"; }
    static std::string get_description() { return simparm::GUILabelTable::get_singleton().get_description( get_name() ); }
};

class MedianSmoother : public engine::spot_finder::Base {
private:
    typedef engine::Image2D Image;
    typedef engine::SmoothedImage SmoothedImage;

    void naiveMedian(const Image &in, SmoothedImage& out, 
                        int mw, int mh);
    
    void (*ahmad)(const Image &in, SmoothedImage& out, int mw, int mh);
    void chooseAhmad(int msx, int msy);

    const Config config;
public:
    MedianSmoother (const Config& c, 
        const engine::spot_finder::Job &job)
        : Base(job), config(c)
        { chooseAhmad(config.mask_size()/2/camera::pixel, config.mask_size()/2/camera::pixel); }
    MedianSmoother* clone() const { return new MedianSmoother(*this); }

    void smooth( const Image &in ) {
        int w = config.mask_size()/camera::pixel;
        ahmad( in, smoothed, w, w );
    }
};

using namespace dStorm::engine;
using std::ostream;

template <typename T>
static int partition(T* list, int left, int right, int pivotIndex) {
     T pivotValue = list[pivotIndex];
     std::swap(list[pivotIndex], list[right]);
     int storeIndex = left;
     for (int i = left; i < right; i++) {
         if (list[i] < pivotValue) {
             std::swap(list[storeIndex], list[i]);
             storeIndex++;
         }
     }
     std::swap(list[right], list[storeIndex]);
     return storeIndex;
}

template <typename T>
static T selectKth(T* list, int left, int right, int k) {
    while (true) {
         int pivotIndex = left + ((right - left) >> 1);
         int pivotNewIndex = partition(list, left, right, pivotIndex);
         if (k == pivotNewIndex)
             return list[k];
         else if (k < pivotNewIndex)
             right = pivotNewIndex-1;
         else
             left = pivotNewIndex+1;
    }
}

template <typename T>
static void insertLine(const T* from, int step, T* into, int w) {
    for (int i = 0; i < w; i++) {
        *into = *from;
        from += step;
        into += 1;
    }
}
 
void MedianSmoother::naiveMedian(const Image &in, SmoothedImage& out, 
                                 int mw, int mh)
 
{
    const int width = in.width_in_pixels(),
              height = in.height_in_pixels();
    const StormPixel *line, *limit = in.ptr(0, height-mh);
    int ln;
    int ms = mw * mh;
    StormPixel buffer[ms], copy[ms];
    int offset = mw/2, median = ms/2;

    for (ln = mh/2, line = in.ptr(); 
         line <= limit; ln++, line += width) 
    {
        for (int block = 0; block < width; block += mw)
            for (int column = 0, coloff = 0; 
                     column < mw && block+column < width; 
                     column++,   coloff += mh)
            {
                insertLine( line + block + column, width,
                            buffer + coloff, mh );
                if (block == 0 && column < mw-1)
                    continue;

                memcpy( copy, buffer, sizeof(StormPixel) * mw * mh );
                SmoothedPixel result = selectKth( copy, 0, ms-1, median );
                out( column + block - offset, ln ) = result;
            }
    }
}

template <typename T, int dim> 
class SortedList {
  private:
    T values[dim];
    int nexts[dim+1];
    int prevs[dim+1];
    bool valid[dim], lowPart[dim];

    int _high, inLow;

  public:
    SortedList() { 
        nexts[dim] = _high = prevs[dim] = dim; inLow = 0; 
        for (int i = 0; i < dim; i++) valid[i] = false;
    }
    inline void init(const T* content, int step, const T& median, int size)
    {
        //cerr << "Initializing column vector with dim " << dim << " ";
        for (int x = 0; x < size; x++) {
            //cerr << x << " ";
            insert(x, *content, median);
            content += step;
        }
        //cerr << "Done" << endl;
    }

    inline void remove(int place) {
        assert( pixInSS1consistent() );

        if (!valid[place]) return;
        nexts[prevs[place]] = nexts[place];
        prevs[nexts[place]] = prevs[place];
        if (_high == place) _high = nexts[place];

        if (lowPart[place]) inLow--;
        valid[place] = false;
        assert( pixInSS1consistent() );
    }
    inline void insertPreferingHigher(int place, const T& value, const T& median)
    {
        assert( state_consistent(median) );
        values[place] = value;
        bool gr = ! (value < median);
        int index = gr ? _high : nexts[dim];
        while (index != dim && values[index] < value) 
            index = nexts[index];

        nexts[place] = index;
        prevs[place] = prevs[index];
        prevs[nexts[place]] = place;
        nexts[prevs[place]] = place;

        if (gr && nexts[place] == _high) _high = place;
        if ( ! gr ) inLow++;
        valid[place] = true;
        lowPart[place] = !gr;
        assert( state_consistent(median) );
    }
    inline void insert(int place, const T& value, const T& median)
    {
        assert( state_consistent(median) );
        values[place] = value;
        bool gr = (value > median);
        int index = gr ? _high : nexts[dim];
        while (index != dim && values[index] < value) 
            index = nexts[index];

        nexts[place] = index;
        prevs[place] = prevs[index];
        prevs[nexts[place]] = place;
        nexts[prevs[place]] = place;

        if (gr && nexts[place] == _high) _high = place;
        if ( ! gr ) inLow++;
        valid[place] = true;
        lowPart[place] = !gr;
        assert( state_consistent(median) );
    }
    inline void replace(int place, T value, const T& median) {
        remove(place);
        insert(place, value, median);
        assert( pixInSS1consistent() );
    }
    inline void replacePreferingHigher(int place, T value, const T& median) {
        remove(place);
        insertPreferingHigher(place, value, median);
        assert( pixInSS1consistent() );
    }

    void adjustBorder(const T& median) {
        assert( pixInSS1consistent() );
        while ( prevs[_high] != dim && values[prevs[_high]] > median ) {
            _high = prevs[_high];
            inLow--;
            lowPart[_high] = false;
        }
        assert( pixInSS1consistent() );
        while ( _high != dim && values[_high] <= median ) {
            lowPart[_high] = true;
            _high = nexts[_high];
            inLow++;
        }
        assert( state_consistent(median) );
    }

    inline int numberInLowerSet() const { return inLow; }

    inline const T* getData() const { return values; }
    inline const T& operator[](const int i) const { return values[i]; }
    inline bool isValid(int i) const { return valid[i]; }
    inline const T& lowFront() const 
        { return values[lowBegin()]; }
    inline const T& highFront() const { return values[_high]; }
    inline int lowBegin() const { return prevs[_high]; }
    inline int highBegin() const { return _high; }

    inline bool lowEmpty() const { return nexts[dim] == _high; }
    inline bool highEmpty() const { return dim == _high; }

    inline void oneToLower() { 
        assert( ! highEmpty() );
        assert( pixInSS1consistent() );
        lowPart[_high] = true;
        _high = nexts[_high];
        inLow++; 
        assert( pixInSS1consistent() );
    }
    inline void oneToHigher() { 
        assert( ! lowEmpty() );
        assert( pixInSS1consistent() );
        _high = prevs[_high]; 
        lowPart[_high] = false;
        inLow--; 
        assert( pixInSS1consistent() );
    }

    void print(ostream &o) const {
        int index = nexts[dim];
        if (_high == nexts[dim]) o << "|"; else o << " ";
        while (index != dim) {
            o << values[index] << ((nexts[index] == _high)?"|":" ");
            index = nexts[index];
        }
    }
    void printState(ostream &o) const {
        o << "NL " << nexts[dim] << " " << prevs[dim] << " ";
        o << " H " << _high << " SS1 " << inLow << "\n";
        for (int i= 0; i < dim; i++)
            o << i << " " << values[i] << " "<< nexts[i] << " "<< prevs[i] << " " << lowPart[i] << " " << valid[i] << "\n";
    }

    bool pixInSS1consistent() const {
        int realSS1 = 0;
        for (int cat = nexts[dim]; cat != _high; cat = nexts[cat]) {
            realSS1++;
        }
        if (realSS1 != inLow) {
            printState(std::cerr);
        }
        return (realSS1 == inLow);
    }

    bool state_consistent(T median) const {
        bool consistent = true;
        bool slowPart = true;
        int lowParts = 0;
        for (int i = nexts[dim]; i != dim; i = nexts[i]) {
            if (i == _high) slowPart = false; 
            if (slowPart) lowParts++;
            if ( slowPart != lowPart[i] )  {
                std::cerr << "slowPart no match for " << i << "\n";
                consistent = false;
            }
            if (slowPart && values[i] > median) {
                std::cerr << "value too large for " << i << "\n";
                consistent = false;
            }
            if (!slowPart && values[i] < median) {
                std::cerr << "value too small for " << i << "\n";
                consistent = false;
            }
        }
        if (lowParts != inLow)
            std::cerr << "lowParts " << lowParts << " wrong" << "\n";
        consistent = consistent && (lowParts == inLow);
        if (!consistent) {
            std::cerr << "Inconsistent state at median " << median << "\n";
            printState(std::cerr);
        }
        return consistent;
    }
};

template <typename T, int mh>
ostream& operator<<(ostream &o, const SortedList<T,mh>& l) 
    { l.print(o); return o; }

static int modMap[500];
static int modMapInit = -1;

template <int strucSize>
void ahmadMedian(const engine::Image2D &in, SmoothedImage& out, int mw, int mh)

{
    const int W = in.width_in_pixels(), H = in.height_in_pixels(), xoff = -mw/2, yoff = mw/2;
    StormPixel median;
    SortedList<StormPixel,strucSize*2> sortedColumns[W];

    int targetPixInSS1 = mw*mh/2+1;

    int borderOthers[2*mw], reverseMod[2*mw];
    if (modMapInit != mw) {
        for (int i = 0; i < W+mw; i++)
            modMap[i] = i % mw;
        modMapInit = mw;
    }
    for (int i = 0; i < mw; i++) {
        borderOthers[i] = i+mw;
        borderOthers[i+mw] = i;
    }

    for (int y = 0, replaceRow = mh-1; y <= H-mh; 
         y++, replaceRow = (replaceRow == mh-1) ? 0 : replaceRow+1) 
    {
        SortedList<StormPixel,strucSize*2> border;
        int pixInSS1 = 0;
        if (y == 0) {
            std::vector<StormPixel> init_window_pixels;
            init_window_pixels.reserve(mw*mh);
            for (int y = 0; y < mh; ++y) {
                const StormPixel *p = in.ptr(0,y);
                for (int x = 0; x < mw; ++x) 
                    init_window_pixels.push_back(p[x]);
            }
            std::partial_sort( 
                init_window_pixels.begin(),
                init_window_pixels.begin()+targetPixInSS1+1,
                init_window_pixels.end() );
            median = init_window_pixels[targetPixInSS1+1];
        } else
            median = out(0+xoff,y+yoff-1);

        for (int x = 0; x < W; x++) {
            reverseMod[ modMap[x] ] = reverseMod[ mw+modMap[x] ] = x;

            /* Update next column vector */
            if (y == 0)
                sortedColumns[x].init( in.ptr(x,0), in.get_offsets().y(), median, mh );
            else {
                sortedColumns[x].adjustBorder( median );
                sortedColumns[x].replace(replaceRow, in(x, y+mh-1),
                                         median);
            }

            int xmod = modMap[x];
            if (! sortedColumns[x].lowEmpty() )
                border.replace(xmod, sortedColumns[x].lowFront(), median);
            else border.remove(xmod);

            if (! sortedColumns[x].highEmpty() )
                border.replace(xmod+mw, sortedColumns[x].highFront(), median);
            else border.remove(xmod+mw);

            pixInSS1 += sortedColumns[x].numberInLowerSet();

            if (x >= mw-1 ) {
                if (x > mw-1)
                    pixInSS1 -= sortedColumns[x-mw].numberInLowerSet();
                //cerr << "PixInSS1 " << pixInSS1 << endl;

                while (pixInSS1 != targetPixInSS1) {
                    bool move_down = pixInSS1 < targetPixInSS1;
                    int movePos = (move_down) ? border.highBegin() : border.lowBegin();
                    int otherPos = borderOthers[movePos];
                    int moveCol = reverseMod[movePos];

                    if ( move_down )
                        sortedColumns[moveCol].oneToLower();
                    else
                        sortedColumns[moveCol].oneToHigher();

                    if ( move_down && ! sortedColumns[moveCol].highEmpty() )
                        border.replacePreferingHigher(otherPos,
                            sortedColumns[moveCol].highFront(), median);
                    else if ( ! move_down && ! sortedColumns[moveCol].lowEmpty() ) {
                        border.replace(otherPos,
                            sortedColumns[moveCol].lowFront(), median);
                    } else {
                        border.remove(otherPos);
                    }

                    if ( move_down ) border.oneToLower(); else border.oneToHigher();
                    pixInSS1 += (move_down) ? 1 : -1;
                    median = border.lowFront();
                    assert( border.state_consistent( median ) );
                }
                median = border.lowFront();
            }
            //cerr << "Result median " << median << endl;

            if (x >= xoff)
                out(x+xoff, y+yoff) = median;
            //cerr << endl;
        }
        //cerr << endl;
    }

    //cerr << "Finished image" << endl;
}

void MedianSmoother::chooseAhmad(int msx, int msy) {
    switch (std::max(msx,msy)) {
        case 0: ahmad = &ahmadMedian<1>; break;
        case 1: ahmad = &ahmadMedian<3>; break;
        case 2: ahmad = &ahmadMedian<5>; break;
        case 3: ahmad = &ahmadMedian<7>; break;
        case 4: ahmad = &ahmadMedian<9>; break;
        case 5: ahmad = &ahmadMedian<11>; break;
        case 6: ahmad = &ahmadMedian<13>; break;
        case 7: ahmad = &ahmadMedian<15>; break;
        case 8: ahmad = &ahmadMedian<17>; break;
    }
}

std::auto_ptr<engine::spot_finder::Factory> make_spot_finder_factory() { 
    return std::auto_ptr<engine::spot_finder::Factory>(new engine::spot_finder::Builder<Config,MedianSmoother>()); 
}

}
}
