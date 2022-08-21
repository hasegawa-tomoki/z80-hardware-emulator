#include <ctime>
#include "bus.hpp"

void Bus::waitNanoSec(int ns){
    static struct timespec req{};
    req.tv_sec = 0;
    req.tv_nsec = ns;
    nanosleep(&req, nullptr);
}
