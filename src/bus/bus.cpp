#include <ctime>
#include "bus.hpp"

bool Bus::nmiFalling(){
    if (this->pin_i_nmi_prev && !this->pin_i_nmi){
        this->pin_i_nmi_prev = this->pin_i_nmi;
        return true;
    }
    return false;
}

bool Bus::intFalling(){
    if (this->pin_i_int_prev && !this->pin_i_int){
        this->pin_i_int_prev = this->pin_i_int;
        return true;
    }
    return false;
}

void Bus::waitNanoSec(int ns){
    static struct timespec req{};
    req.tv_sec = 0;
    req.tv_nsec = ns;
    nanosleep(&req, nullptr);
}
