#ifndef CImgBuffer_ANDORDIRECT_H
#define CImgBuffer_ANDORDIRECT_H

#ifdef HAVE_LIBATMCD32D

#include <dStorm/input/Config.h>
#include <dStorm/input/Source.h>
#include <dStorm/input/ImageTraits.h>
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

namespace dStorm {
    namespace AndorDirect {

        /** The Config class provides the configuration items for Andor camera
         *  acquisition that are acquisition-specific - control elements and
         *  acquisition area borders. All camera specific parameters are in
         *  AndorCamera::Config. */
        class Config : public CamConfig, public simparm::Node::Callback
        {
          private:
            class CameraSwitcher;
            std::auto_ptr<CameraSwitcher> switcher;
            simparm::FileEntry basename;
          public:
            simparm::BoolEntry show_live_by_default;
            simparm::DoubleEntry live_show_frequency;
            const simparm::DoubleEntry& resolution_element;

          private:
            void registerNamedEntries();

          protected:
            CamSource* impl_makeSource();
            void operator()(const simparm::Event&);

          public:
            Config(input::Config& src);
            Config(const Config &c, input::Config &src);
            virtual ~Config();

            std::auto_ptr< CamSource > makeSource()
                { return std::auto_ptr< CamSource >(impl_makeSource()); }

            Config* clone(input::Config &newMaster) const
                { return new Config(*this, newMaster); }
            bool uses_input_file() const { return false; }
        };

        /** This Source class provides a source that captures directly
         *  from Andor cameras present on the system. It needs the
         *  AndorCamera library to compile. */
        class Source 
        : public simparm::Set, public CamSource, private ost::Thread
        {
          private:
            AndorCamera::CameraReference control;
            ost::Mutex initMutex;
            ost::Condition is_initialized;
            bool initialized;
            int numImages;
            AndorCamera::Acquisition acquisition;

            void run() throw();
            void acquire();
            bool cancelAcquisition;

            void waitForInitialization() const;

            simparm::StringEntry& status;

            std::auto_ptr<LiveView> live_view;

          public:
            Source(const Config& config,
                   AndorCamera::CameraReference& camera);
            Source(const Source &);
            virtual ~Source();
            Source *clone() const { 
                throw std::logic_error("Unclonable."); }

            virtual int quantity() const; 
            virtual bool pull_length() const { return initialized; }

            virtual void startPushing(input::Drain<CamImage> *target);
            virtual void stopPushing(input::Drain<CamImage> *target);

        };
    }
}

#endif

#endif
