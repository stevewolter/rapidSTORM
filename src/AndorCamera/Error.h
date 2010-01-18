#ifndef ANDORCAMERA_ERROR_H
#define ANDORCAMERA_ERROR_H

#include <stdexcept>

namespace AndorCamera {
    /** Exception message construction class for Andor SDK. */
    class Error : public std::exception {
        private:
            /** The numeric code Andor functions return. */
            unsigned int andorReturnCode;
            /** The string that holds the buffer with \c _what.
             *  \sa _what*/
            std::string error_message;
            /** A character sequence giving a textual description
             *  of this error */
            const char *_what;

        public:
            /** Construct an error from the andor return code,
             *  the name of the SDK wrapper function the error
             *  happened in and an optional message.
             *
             *  \param andor_code Return code of an Andor function.
             *                    Will not be checked; even DRV_SUCCESS
             *                    results in success.
             *  \param locSpec    Name of the calling function or code
             *                    section.
             *  \param message    Optional message describing the error
             *                    source */
            Error(unsigned int andor_code, const char *locSpec, 
                    const std::string& message = "");
            /** printf-equivalent error constructor */
            Error(const std::string& message, ...);
            /** Destructor. Deallocates the error_message */
            virtual ~Error() throw() {}
            /** Return a character sequence describing the error. */
            virtual const char *what() const throw();

            unsigned int andorCode() const { return andorReturnCode; }
    };
}

#endif
