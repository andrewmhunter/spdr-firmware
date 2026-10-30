#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <iostream>

enum class RID {
    A,
    C,
    D,
    F,
    Temp,
    Alu,
    Sp,
    PcL,
    PcH,
    PcLH,
    MarL,
    MarH,
    MarLH,
    Mem,
    Out,
    Lcd,
    Kbd,
    Ins,
    CD,
    AC,
    AD,
    Prog,
    None,
};

std::ostream& operator<<(std::ostream& stream, RID reg);

#endif

