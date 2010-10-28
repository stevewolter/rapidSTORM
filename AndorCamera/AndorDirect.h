#ifndef CImgBuffer_ANDORDIRECT_H
#define CImgBuffer_ANDORDIRECT_H

#include "AndorDirect_decl.h"

#include <dStorm/input/Source.h>
#include <dStorm/ImageTraits.h>
#include <AndorCamera/CameraReference.h>
#include <AndorCamera/Acquisition.h>
#include <memory>
#include <string>
#include <dStorm/helpers/thread.h>
#include <simparm/Set.hh>

#include <boost/shared_ptr.hpp>

#include "LiveView_decl.h"

namespace AndorCamera {

    /** This Source class provides a source that captures directly
        *  from Andor cameras present on the system. It needs the
        *  AndorCamera library to compile. */
    class Source 
    : public simparm::Set, public CamSource
    {
      private:
        AndorCamera::CameraReference control;
        mutable ost::Mutex initMutex;
        mutable ost::Condition is_initialized;
        bool initialized, error_in_initialization;
        AndorCamera::Acquisition acquisition;

        class iterator;
        friend class iterator;

        void acquire();
        bool cancelAcquisition;

        void waitForInitialization() const;

        simparm::StringEntry& status;

        boost::shared_ptr<LiveView> live_view;
        void dispatch(Messages m) { assert( ! m.any() ); }

      public:
        Source( boost::shared_ptr<LiveView> live_view,
                AndorCamera::CameraReference& camera);
        Source(const Source &);
        virtual ~Source();
        Source *clone() const { 
            throw std::logic_error("Unclonable."); }

        virtual CamSource::iterator begin();
        virtual CamSource::iterator end();
        virtual TraitsPtr get_traits();
    };
}

#endif
