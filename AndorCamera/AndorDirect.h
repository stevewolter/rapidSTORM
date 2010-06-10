#ifndef CImgBuffer_ANDORDIRECT_H
#define CImgBuffer_ANDORDIRECT_H

#include "AndorDirect_decl.h"

#include <dStorm/input/Config.h>
#include <dStorm/input/Source.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/input/Method.h>
#include <AndorCamera/CameraReference.h>
#include <AndorCamera/Camera.h>
#include <AndorCamera/StateMachine.h>
#include <AndorCamera/Acquisition.h>
#include <memory>
#include <string>
#include <set>
#include <dStorm/helpers/thread.h>
#include <simparm/TriggerEntry.hh>
#include <simparm/Set.hh>

#include "LiveView_decl.h"

namespace AndorCamera {

    /** The Config class provides the configuration items for Andor camera
        *  acquisition that are acquisition-specific - control elements and
        *  acquisition area borders. All camera specific parameters are in
        *  AndorCamera::Config. */
    class Method : public CamConfig, public simparm::Node::Callback
    {
        private:
        class CameraSwitcher;
        std::auto_ptr<CameraSwitcher> switcher;
        simparm::FileEntry basename;
        public:
        simparm::BoolEntry show_live_by_default;
        simparm::DoubleEntry live_show_frequency;
        dStorm::input::Config& resolution_element;

        private:
        void registerNamedEntries();

        protected:
        CamSource* impl_makeSource();
        void operator()(const simparm::Event&);

        public:
        Method(dStorm::input::Config& src);
        Method(const Method &c, dStorm::input::Config &src);
        virtual ~Method();

        std::auto_ptr< CamSource > makeSource()
            { return std::auto_ptr< CamSource >(impl_makeSource()); }

        Method* clone(dStorm::input::Config &newMaster) const
            { return new Method(*this, newMaster); }
        bool uses_input_file() const { return false; }
    };

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

        std::auto_ptr<LiveView> live_view;

      public:
        Source(const Method& config,
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
