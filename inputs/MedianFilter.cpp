#include "MedianFilter.h"

#include <queue>

#include <simparm/BoostUnits.h>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/io.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>
#include <dStorm/image/extend.h>
#include <dStorm/Image.h>
#include <dStorm/image/slice.h>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/localization/Traits.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <dStorm/UnitEntries/FrameEntry.h>
#include <dStorm/units/frame_count.h>

namespace dStorm {
namespace ROIFilter {

struct Config {
    simparm::Object name_object;
    simparm::BoolEntry apply_filter;
    IntFrameEntry width, stride;

    Config();
    void attach_ui( simparm::NodeHandle at ) {
        simparm::NodeHandle r = name_object.attach_ui(at);
        apply_filter.attach_ui( r );
        width.attach_ui( r ); 
        stride.attach_ui( r ); 
    }
};

class Source
: public input::AdapterSource<engine::ImageStack>
{
    const int width, stride;
    std::queue<engine::ImageStack> outgoing;
    std::vector<dStorm::Image<engine::StormPixel, 3>> median_values;

    typedef engine::StormPixel StormPixel;
    typedef input::Source<engine::ImageStack>::iterator base_iterator;

    struct _iterator;

    void attach_local_ui_( simparm::NodeHandle ) {}

    void InsertValue(const engine::ImageStack& images, int nth_value) {
        assert(images.plane_count() == int(median_values.size()));

        outgoing.push(images);
        for (int i = 0; i < images.plane_count(); ++i) {
            std::copy(images.plane(i).begin(), images.plane(i).end(),
                      median_values[i].slice(0, nth_value * camera::pixel).begin());
        }
    }

    template <class UpdateFunction>
    void ComputeMedian(const UpdateFunction& update) {
        for (size_t plane_index = 0; plane_index < median_values.size(); ++plane_index) {
            engine::Image2D starts = median_values[plane_index].slice(0, 0 * camera::pixel);
            for (auto i = starts.begin(); i != starts.end(); ++i) {
                UpdateFunction(&*i, (&*i) + width, plane_index, i.position());
            }
        }
    }

    void ComputeInitialMedian() {
        ComputeMedian([](StormPixel* begin, StormPixel* end, int plane, const engine::Image2D::Position& position) {
                std::sort(begin, end);
        });
    }

    void UpdateMedian(const engine::ImageStack& incoming, const engine::ImageStack& outgoing) {
        ComputeMedian([&](StormPixel* begin, StormPixel* end, int plane, const engine::Image2D::Position& position) {
            StormPixel new_value = incoming.plane(plane)(position);
            StormPixel old_value = outgoing.plane(plane)(position);
            StormPixel* insertion_point = std::lower_bound(begin, end, new_value);
            StormPixel* deletion_point = std::find(begin, end, old_value);

            assert(deletion_point < end);
            if (insertion_point < deletion_point) {
                std::move_backward(insertion_point, deletion_point, deletion_point + 1);
            } else if (insertion_point == end) {
                std::move(deletion_point + 1, end, deletion_point);
                insertion_point = end - 1;
            } else {
                std::move(deletion_point + 1, insertion_point + 1, deletion_point);
            }
            *insertion_point = new_value;
        });
    }

    void StoreMedian(engine::ImageStack& target) {
        for (const auto& median : median_values) {
            target.push_back_background(median.slice(0, width/2 * camera::pixel));
        }
    }

  public:
    Source( std::auto_ptr< input::Source<engine::ImageStack> > upstream,
            frame_index width, frame_index stride)
        : input::AdapterSource<engine::ImageStack>(upstream), width(width.value()), stride(stride.value()) {}
    Source* clone() const { throw std::logic_error("clone() for MedianFilter::Source not implemented"); }

    base_iterator begin();
    base_iterator end();
    void modify_traits( input::Traits<engine::ImageStack>& p)
    {
        for (int i = 0; i < p.plane_count(); ++i) {
            dStorm::image::MetaInfo<3> size;
            size.size[0] = width * camera::pixel;
            size.size[1] = p.plane(i).image.size[0];
            size.size[2] = p.plane(i).image.size[1];
            median_values.emplace_back(size);
        }
    }
};

class Source::_iterator 
  : public boost::iterator_adaptor< 
        Source::_iterator,
        typename input::Source<engine::ImageStack>::iterator >
{
    Source& s;
    typedef typename input::Source<engine::ImageStack>::iterator Base;
    const Base end;
    Base lookahead;
    bool has_median;
    mutable engine::ImageStack i;

    friend class boost::iterator_core_access;
    void increment() { 
        ++this->base_reference();
        lookahead += s.stride;

        UpdateMedian();
    }

        if ( this->base() != end ) {
            i = *this->base();
            if (lookahead != end) {
                s.UpdateMedian(*lookahead, *this->base());
            }
            s.StoreMedian(i);
        }

    engine::ImageStack& dereference() const { return i; }
    
  public:
    explicit _iterator(Source& s, const Base& from, const Base& end)
      : _iterator::iterator_adaptor_(from), s(s), end(end), lookahead(from), has_median(true)
    {
        for (frame_index i = 0; i < s.width; ++i) {
            lookahead += s.stride;
            if ( lookahead != end ) {
                s.InsertValue(*this->base(), i);
            } else {
                has_median = false;
            }
        }
        if (has_median) {
            s.ComputeInitialMedian();
        if (this->base() != end) {
            i = *this->base();
        } else if (!delay.empty()) {
            i = delay.front();
        }
        s.StoreMedian(i);
    }
};

Source::base_iterator Source::begin() { 
    return Source::base_iterator( 
        _iterator( *this, this->base().begin(), this->base().end() ) ); 
}

Source<Ty>::base_iterator Source<Ty>::end() {
    return Source::base_iterator( 
        _iterator( *this, this->base().end(), this->base().end() ) );
}

class ChainLink 
: public input::Method<ChainLink>
{
    friend class input::Method<ChainLink>;

    Config config;

    void update_traits( input::MetaInfo&, input::Traits<engine::ImageStack>& traits ) {}

    void notice_traits( const input::MetaInfo&, const input::Traits<engine::ImageStack>& t ) {}

    template <typename Type>
    input::Source<Type>* make_source( std::auto_ptr< input::Source<Type> > p ) {
        if ( config.apply_filter() ) {
            return new Source<Type>( p, config.width(), config.stride() );
        } else
            return p.release();
        }
    }

  public:
    static std::string getName() { return "TemporalMedianFilter"; }
    void attach_ui( simparm::NodeHandle at ) { 
        config.attach_ui( at ); 
    }
};

Config::Config() 
: name_object(ChainLink::getName(), "Temporal median filter"),
  apply_filter("ApplyFilter", false),
  width("Number of images in median", 20 * camera::frame),
  stride( "Stride between key images", 10 * camera::frame )
{
    apply_filter.set_user_level( simparm::Intermediate ),
    width.set_user_level( simparm::Expert );
    stride.set_user_level( simparm::Expert );
}

std::auto_ptr<input::Link> make_link() {
    return std::auto_ptr<input::Link>( new ChainLink() );
}

}
}
