#ifndef CIMGBUFFER_STM_METHOD_H
#define CIMGBUFFER_STM_METHOD_H

#include "InputMethod.h"
#include "Source.h"
#include <fstream>

namespace CImgBuffer {
    template <typename T>
    class STMSource : public virtual Source<T>, public simparm::ConfigSet {
      public:
        STMSource() : ConfigObject("STM_Show", "Input options") {}
        virtual int dimx() throw() { return 0; }
        virtual int dimy() throw() { return 0; }
        virtual int image_quantity() throw() { return 0; }

        virtual simparm::ConfigObject& getConfig() throw() {return *this;}
    };

    template <typename T>
    class STMInputConfig : public virtual InputConfig<T> {
      private:
        Config<T>& master;
        
      public:
        STMInputConfig(Config<T>& master) throw() 
        : ConfigObject("STM", "Localizations file"),
          master(master)
        {
            register_entry(&master.inputFile);
            register_entry(&master.firstImage);
            register_entry(&master.lastImage);
        }

        std::auto_ptr< Source<T> > makeSource() 
            throw(std::exception)
        {
            std::ifstream tester(master.inputFile().c_str(), std::ios::in);
            std::string line;
            std::getline(tester, line);
            tester.close();

            for (size_t i = 0; i < line.size(); i++)
                if (line[i] != ' ' && line[i] != '\r' && 
                        (line[i] < '0' || line[i] > '9') && line[i] != '.'
                        && line[i] != 'e' && line[i] != 'E')
                    throw std::logic_error("No valid STM file: Invalid "
                       "character in first line");
            
            std::auto_ptr< STMSource<T> > source(new STMSource<T>());
            source->push_back( master.inputFile );
            return std::auto_ptr< Source<T> >( source );
        }

        std::auto_ptr< InputConfig<T> > clone
            (Config<T> &newMaster) const throw() 
            { return std::auto_ptr< InputConfig<T> >
                (new STMInputConfig<T>(newMaster)); }
    };

    template <typename T>
    class STMInputMethod : public InputMethod<T> {
      public:
        std::auto_ptr< InputConfig<T> > makeConfig
            (Config<T> &m) throw(std::exception)
        { 
            return std::auto_ptr< InputConfig<T> >
                (new STMInputConfig<T>(m));
        }
    };
    
};

#endif
