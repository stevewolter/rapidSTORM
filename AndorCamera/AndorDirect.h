#ifndef CImgBuffer_ANDORDIRECT_H
#define CImgBuffer_ANDORDIRECT_H

#include "AndorDirect_decl.h"

#include <dStorm/input/Source.h>
#include <dStorm/ImageTraits.h>
#include <memory>
#include <string>
#include <simparm/Set.hh>

#include <boost/shared_ptr.hpp>

#include "LiveView.h"

namespace dStorm {
namespace AndorCamera {

struct CameraConnection;

    /** This Source class provides a source that captures directly
        *  from Andor cameras present on the system. It needs the
        *  AndorCamera library to compile. */
    class Source 
    : public simparm::Set, public CamSource
    {
      private:
        std::auto_ptr<CameraConnection> connection;
        TraitsPtr traits;
        bool has_ended, show_live;
        std::auto_ptr<LiveView> live_view;
        LiveView::Resolution resolution;
        class iterator;

        simparm::StringEntry status;

        void dispatch(Messages m) { assert( ! m.any() ); }
        simparm::Node& node() { return *this; }

      public:
        Source( std::auto_ptr<CameraConnection> connection, bool live_view, LiveView::Resolution );
        Source(const Source &);
        virtual ~Source();
        Source *clone() const { 
            throw std::logic_error("Unclonable."); }

        virtual CamSource::iterator begin();
        virtual CamSource::iterator end();
        virtual TraitsPtr get_traits( Wishes );
        Capabilities capabilities() const { return Capabilities(); }
    };
}
}

#endif
