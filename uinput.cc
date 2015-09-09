#include <linux/uinput.h>
#include <unistd.h>
#include <fcntl.h>
#include "util.h"

static void
syscall_ex(bool result, const char *err)
{
    if (!result) {
        Errno e;
        e << ": " << err;
        throw e;
    }
}



int
main(int argc, char *argv[])
{
    int uinputFd = open("/dev/uinput", O_WRONLY, O_NONBLOCK);
    if (uinputFd < 0) {
        Errno e;
        e << ": can't open uinput device";
        throw e;
    }

    syscall_ex(ioctl(uinputFd, UI_SET_EVBIT, EV_REL) == 0, "add relative events");
    syscall_ex(ioctl(uinputFd, UI_SET_EVBIT, EV_KEY) == 0, "add key events");
    syscall_ex(ioctl(uinputFd, UI_SET_EVBIT, EV_SYN) == 0, "add syn events");

    syscall_ex(ioctl(uinputFd, UI_SET_KEYBIT, BTN_0) == 0, "add button 0");
    syscall_ex(ioctl(uinputFd, UI_SET_KEYBIT, BTN_1) == 0, "add button 1");
    syscall_ex(ioctl(uinputFd, UI_SET_KEYBIT, BTN_2) == 0, "add button 2");

    syscall_ex(ioctl(uinputFd, UI_SET_RELBIT, REL_X) == 0, "add X relative");
    syscall_ex(ioctl(uinputFd, UI_SET_RELBIT, REL_Y) == 0, "add Y relative");

    struct uinput_user_dev mydev;
    memset(&mydev, 0, sizeof mydev);
    snprintf(mydev.name, UINPUT_MAX_NAME_SIZE, "android-pad");
    mydev.id.bustype = BUS_USB;
    mydev.id.vendor = 0xface;
    mydev.id.product = 0xface;
    mydev.id.version = 1;
    syscall_ex(write(uinputFd, &mydev, sizeof mydev) == sizeof mydev, "can't configure device");

    syscall_ex(ioctl(uinputFd, UI_DEV_CREATE) == 0, "create device failed");

    struct input_event ev[2];

    for (int j = 0; j < 100000; ++j) {
        int diff = j % 2 ? -1 : 1;
        for (int i = 0; i < 100; ++i) {
            usleep(100000);
            memset(ev, 0, sizeof ev);
            ev[0].type = EV_REL;
            ev[0].code = REL_X;
            ev[0].value = diff * 4;
            ev[1].type = EV_SYN;
            ev[1].code = 0;
            ev[1].value = 0;
            syscall_ex(write(uinputFd, ev, sizeof ev) == sizeof ev, "write event failed");
            syscall(fsync(uinputFd), "sync fd");
            std::clog << ".";
        }
    }

    for (;;)
        pause();



    exit(0);
}
