#include "util.h"

void
syscall_ex(bool result, const char *err)
{
    if (!result) {
        Errno e;
        e << ": " << err;
        throw e;
    }
}
