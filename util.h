#include <exception>
#include <iostream>
#include <sstream>

#include <errno.h>
#include <string.h>

template <typename Type, typename Close> class Closer {
   Type item;
   Close close;
   bool released;
public:
   Closer(const Type &item_, const Close &close_) : item(item_), close(close_), released(false) {}
   ~Closer() { 
      if (!released) {
         close(item);
      }
   }
   operator Type &() { return item; }
   operator const Type &() const { return item; }
   Type &release() { released = true; return item;  }
};

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
    typedef void IsStreamable;
};

template <typename E, typename Object, typename Test = typename E::IsStreamable>
inline std::ostream &operator << (E &stream, const Object &o) {
    return stream.getStream() << o;
}

template <typename Stream> Stream &
operator << (Stream &s, const std::exception &ex) {
    return s << ex.what();
}


void syscall_ex(bool ok, const char *err);
