#ifndef CIMGBUFFER_INPUTMETHOD_H
#define CIMGBUFFER_INPUTMETHOD_H

#include <vector>
#include <memory>
#include <ConfigSet.hh>
#include <CImgBuffer/Config.h>
#include <CImgBuffer/Source.h>

namespace CImgBuffer {
    /** An InputConfig class is the base class for all config sets
     *  that can provide a Source object. */
    template <typename T>
    class InputConfig : public ConfigSet {
      protected:
        InputConfig() throw() : ConfigObject("", ""), ConfigSet() {}
      public:
        virtual bool may_be_autoloaded() const throw() { return true; }

        /** Make a Source object with the current settings. */
        virtual std::auto_ptr< Source<T> > makeSource() 
            throw(std::exception) = 0;
        /** Clone the current set and all its settings.
         *  \param newMaster The Config object that will own the clone.
         *                   Used to update references. */
        virtual std::auto_ptr< InputConfig<T> > clone
            (Config<T> &newMaster) const throw() = 0;
        /** This method is provided for compatibility with InputConfig
         *  and always fails. */
        InputConfig *clone() const throw(std::exception)
            { throw std::logic_error("No clone() method defined "
                                     "for InputConfig objects."); }
    };

    /** An InputMethod is a generator for InputConfig objects.
     *  The list of all existing InputMethod objects is hold
     *  statically within this class. */
    template <typename T>
    class InputMethod {
      private:
        static std::vector<InputMethod<T>*>* inputMethods;
        static void initInputMethods() throw();

      public:
        /** Register a new input method. The method will stay registered
         *  for the rest of the program runtime. */
        static void registerInputMethod
            (std::auto_ptr<InputMethod<T> > method) throw()
            { initInputMethods(); 
              inputMethods->push_back(method.release()); }
        /** Register a new input method. The method will stay registered
         *  for the rest of the program runtime.
         *  \param method The input method provided in this pointer
         *                will be deallocated by the InputMethod class. */
        static void registerInputMethod(InputMethod<T>* method) throw()
            { initInputMethods(); inputMethods->push_back(method); }
        /** \return The number of input methods currently registered. */
        static int numMethods() throw() 
            { initInputMethods(); return inputMethods->size(); }
        /** Get a specific input method. 
         *  \param index A number between 0 and numMethods()-1 */
        static InputMethod<T>& getMethod(int index) throw()
            { initInputMethods(); return *(*inputMethods)[index]; }

        InputMethod () throw () {}
        virtual ~InputMethod () throw() {}

        /** Make a new config for this input method. You may use
         *  config entries defined in the master config.
         *  \param master Config that will use this method. */
        virtual std::auto_ptr< InputConfig<T> > makeConfig
            (Config<T> &master) throw(std::exception) = 0;
    };
}
#endif
