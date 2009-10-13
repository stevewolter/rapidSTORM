#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_H

#include <data-c++/Vector.h>

namespace dStorm {
namespace DiscretizedImage {

    template <typename ColorPixel>
    struct Listener {
        /** The setSize method is called before any announcements are made,
         *  and is called afterwards when the image size changes. */
        void setSize(int width, int height);
        /** Called when a pixel changes in the image. The parameters
         *  give the position of the changed pixel. */
        void pixelChanged(int x, int y);
        /** The listener should apply pending changes. */
        void clean();
        /** The state should be reset to an empty image. */
        void clear();
        /** This method is called when the discretization parameters change.
         *  @param index The index of this colour in the key.
         *  @param pixel The color for which the discretization changed.
         *               This parameter's type in an implementation should
         *               not be 
         *  @param value The new upper bound on the number of A/D counts for
         *               this pixel. */
        void notice_key_change( int index, ColorPixel pixel, float value );
    };

#if 0
    struct DummyListener {
        void setSize(int, int) {}
        void pixelChanged(int, int) {}
        void clean() {}
        void clear() {}
    };
#endif

    /** The Publisher class stores a pointer to the currently
     *  set listener, if any is provided. */
    template <typename Listener>
    struct Publisher
    {
        Listener *fwd;
      public:
        inline void setListener(Listener* target)
            { fwd = target; }
        inline Listener& publish() { return *fwd; }
    };

struct HistogramPixel { 
    /** Pixel position in image */
    unsigned short x, y;
    /** Linked list with pixels of same value. */
    HistogramPixel *prev, *next; 

    HistogramPixel() { clear(); }
    HistogramPixel(const HistogramPixel&) { clear(); }
    void push_back(HistogramPixel& node) {
        node.prev->next = node.next;
        node.next->prev = node.prev;
        node.prev = prev;
        node.next = this;
        prev->next = &node;
        prev = &node;
    }
    void unlink() {
        prev->next = next;
        next->prev = prev;
        clear();
    }
    void clear() { prev = next = this; }
    bool empty() { return ( next == this ); }
};

template <typename Colorizer,
          typename ImageListener>
class ImageDiscretizer 
: public BinningListener,
  public Publisher<ImageListener>
{
    typedef unsigned short HighDepth;
    typedef typename Colorizer::BrightnessType LowDepth;

    typedef data_cpp::Vector<LowDepth> TransitionTable;

    unsigned int total_pixel_count;
    Colorizer& colorizer;

    float max_value, max_value_used_for_disc_factor,
          disc_factor;

    data_cpp::Vector<unsigned int> histogram;
    TransitionTable transition;
    data_cpp::Vector<HistogramPixel> pixels_by_value;
    cimg_library::CImg<HistogramPixel> pixels_by_position;

    unsigned int in_depth, out_depth,
                 background_threshold,
                 non_background_pixel_count,
                 pixels_above_used_max_value;
    float histogram_power;

    const cimg_library::CImg<float>& binned_image;

    HighDepth discretize( float value, float disc_factor ) 
        { return std::min<HighDepth>( in_depth-1,
            HighDepth(round( value * disc_factor )) ); }
    HighDepth discretize( float value ) 
        { return discretize(value, disc_factor); }

    inline void change( int x, int y, HighDepth to );
    void normalize_histogram();
    void publish_differences_in_transitions( 
        TransitionTable& old_table, TransitionTable& new_table );

  public:
    inline ImageDiscretizer(
        int intermediate_depth, int background_threshold,
        float histogram_power, 
        const cimg_library::CImg<float>& binned_image,
        Colorizer& colorizer);
    inline ~ImageDiscretizer();

    inline void setSize(int, int);
    inline void updatePixel(int, int, float, float);
    void clean();
    void clear();

    void announce(const Output::Announcement& a) 
        { colorizer.announce(a); }
    void announce(const Output::EngineResult& er)
        { colorizer.announce(er); }
    void announce(const Localization& loc)
        { colorizer.announce( loc ); }

    typename Colorizer::Pixel get_pixel( int x, int y ) {
        return colorizer.getPixel(x, y, 
            transition[ discretize( binned_image(x,y) ) ]);
    }
    typename Colorizer::Pixel get_background() 
        { return colorizer.get_background(); }

    std::auto_ptr< cimg_library::CImg<uint8_t> > full_image();

    void setHistogramPower(float power);
};

}
}

#endif
