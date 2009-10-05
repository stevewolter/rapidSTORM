#ifndef CImgBuffer_ANDORDIRECT_H
#define CImgBuffer_ANDORDIRECT_H

#include "Config.h"
#include "Source.h"
#include "ImageTraits.h"
#include "InputMethod.h"
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
#include <stdint.h>

namespace cimg_library {
    template <typename PixelType> class CImg;
    class CImgDisplay;
};

namespace CImgBuffer {
    namespace AndorDirect {
        typedef uint16_t CameraPixel;
        typedef cimg_library::CImg<CameraPixel> CamImage;
        typedef Source< CamImage > CamSource;
        typedef InputConfig<CamImage> CamConfig;

        /** The Config class provides the configuration items for Andor camera
         *  acquisition that are acquisition-specific - control elements and
         *  acquisition area borders. All camera specific parameters are in
         *  AndorCamera::Config. */
        class Config : public InputConfig<CamImage>
        {
          private:
            class CameraSwitcher;
            std::auto_ptr<CameraSwitcher> switcher;
            simparm::FileEntry basename;

            void registerNamedEntries();

          protected:
            CamSource* impl_makeSource();

          public:
            Config(CImgBuffer::Config& src) 
;
            Config(const Config &c, 
                    CImgBuffer::Config &src) 
;
            virtual ~Config();

            std::auto_ptr< CamSource > makeSource()
                { return std::auto_ptr< CamSource >(impl_makeSource()); }

            InputConfig<CamImage>* clone
                (CImgBuffer::Config &newMaster) const
                { return new Config(*this, newMaster); }

            std::string get_basename_of_output_file();
            bool may_be_autoloaded() const { return false; }
        };

        /** This Source class provides a source that captures directly
         *  from Andor cameras present on the system. It needs the
         *  AndorCamera library to compile. */
        class Source 
        : public CamSource, public simparm::Set, private ost::Thread
        {
          private:
            AndorCamera::CameraReference control;
            ost::Mutex initMutex;
            ost::Condition is_initialized;
            bool initialized;
            int width, height, numImages;
            AndorCamera::Acquisition acquisition;

            void run() throw();
            void acquire();
            bool cancelAcquisition;

            void waitForInitialization() const;

            simparm::StringEntry& status;
            simparm::BoolEntry show_live;
            simparm::StringEntry dynamic_range;

            std::auto_ptr< cimg_library::CImgDisplay > display;
            void display_concurrently( const CamImage& image, int num );

          public:
            Source(AndorCamera::CameraReference& camera) 
;
            Source(const Source &);
            virtual ~Source() 
;
            Source *clone() const { 
                throw std::logic_error("Unclonable."); }

            virtual int dimx() const; 
            virtual int dimy() const; 
            virtual int quantity() const; 
            virtual bool pull_length() const { return initialized; }

            virtual void startPushing(Drain<CamImage> *target)
;
            virtual void stopPushing(Drain<CamImage> *target)
;

        };
    }
}

#endif
