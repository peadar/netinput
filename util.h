#include <exception>
#include <iostream>
#include <sstream>

#include <errno.h>
#include <string.h>

class Exception : public std::exception {
    mutable std::ostringstream str;
    mutable std::string intermediate;
public:
    Exception() {
    }

    Exception(const Exception &rhs) {
        str << rhs.str.str();
    }

    ~Exception() throw () {
    }

    const char *what() const throw() {
        intermediate = str.str();
        return intermediate.c_str();
    }
    std::ostream &getStream() const { return str; }
    typedef void IsStreamable;
};

class Errno : public Exception {
    int syserror;
public:
    Errno(int syserror_ = -1) : syserror(syserror_) {
        if (syserror == -1)
            syserror = errno;
        getStream() << strerror(syserror);
    }
};

template <typename E, typename Object, typename Test = typename E::IsStreamable>
inline const E &operator << (const E &stream, const Object &o) {
    stream.getStream() << o;
    return stream;
}



