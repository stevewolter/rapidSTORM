#ifndef CImgBuffer_ANDORDIRECT_H
#define CImgBuffer_ANDORDIRECT_H

#include "Config.h"
#include "Source.h"
#include "InputMethod.h"
#include <AndorCamera/ControlReference.h>
#include <AndorCamera/ControlStateListener.h>
#include <AndorCamera/Acquisition.h>
#include <memory>
#include <string>
#include <set>
#include <ConfigEventHandler.hh>
#include <cc++/thread.h>

namespace CImgBuffer {
    template <typename PixelType> class Image;

    namespace AndorDirect {
        typedef unsigned short CameraPixel;
        typedef Image<CameraPixel> CamImage;

        /** The Config class provides the configuration items for Andor camera
         *  acquisition that are acquisition-specific - control elements and
         *  acquisition area borders. All camera specific parameters are in
         *  AndorCamera::Config. */
        class Config : public InputConfig<CamImage>,
                       public ConfigEventHandler,
                       public AndorCamera::ControlStateListener
        {
            private:
                AndorCamera::ControlReference control;

                void init() throw();
                void registerEntries(ConfigSet &c) throw();

                void startAiming() throw();
                void stopAiming() throw();
                void initializeDimensions() throw();
                void resetDimensions() throw();

                bool initialized;
                ConfigSet &papa;

            protected:
                void gotEvent(const ConfigEntry &eventSource) throw();

            public:
                Config(CImgBuffer::Config<CamImage>& src) 
                    throw(std::exception);
                Config(const Config &c, 
                       CImgBuffer::Config<CamImage> &src) 
                    throw(std::exception);
                virtual ~Config() throw();

                std::auto_ptr< Source<CamImage> > makeSource()
                    throw(std::exception);

                ConfigEntryUnsignedLong numImages;
                ConfigEntryUnsignedLong left, top, right, bottom;
                ConfigEntryTrigger      aim;
                ConfigEntryTrigger      stopAim;
                ConfigEntryDivider      divider;
                ConfigEntryTrigger      pause;
                ConfigEntryFile         imageFile;
                ConfigEntryTrigger      save;

                void controlStateChanged(AndorCamera::State to) 
                    throw(std::exception);

                typedef std::auto_ptr< InputConfig<CamImage> > Result;
                Result clone (CImgBuffer::Config<CamImage> &newMaster)
                    const throw()
                    { return Result(new Config(*this, newMaster)); }

                void printAdditionalConfigSets(ostream &o) throw();
        };

        class Method : public InputMethod<CamImage> {
          public:
            typedef std::auto_ptr< InputConfig<CamImage> > Result;
            Method() throw() {}
            ~Method() throw() {}

            Result makeConfig(CImgBuffer::Config<CamImage>& master) 
                throw(std::exception)
                { return Result(new Config(master)); }
        };

        /** This Source class provides a source that captures directly
         *  from Andor cameras present on the system. It needs the
         *  AndorCamera library to compile. */
        class Source : public CImgBuffer::Source<CamImage>, 
                       private ost::Thread
        {
          private:
            AndorCamera::ControlReference control;
            int width, height, numImages;
            AndorCamera::Acquisition acquisition;

            void run() throw();
            void acquire() throw();
            bool cancelAcquisition;

          public:
            Source(int l, int t, int w, int h, int numImages) 
                throw(std::exception);
            virtual ~Source() 
                throw(std::exception);

            virtual int dimx() throw(); 
            virtual int dimy() throw(); 
            virtual int image_quantity() throw(); 

            virtual void startPushing(Drain<CamImage> *target)
                throw(std::exception);
            virtual void stopPushing(Drain<CamImage> *target)
                throw(std::exception);
            virtual std::ostream& writeAcquisitionInfo(std::ostream &o)
                throw();
        };
    }
}

#endif
