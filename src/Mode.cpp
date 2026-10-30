#include "Mode.hpp"
#include <map>
#include <format>
#include <stdexcept>

int getSize(Size size) {
    switch (size) {
        case Size::Unsized:
            return 0;
        case Size::Byte:
        case Size::Page:
            return 1;
        case Size::Word:
            return 2;
    }
    throw std::invalid_argument{"Size invalid"};
}

std::ostream& operator<<(std::ostream& stream, Size size) {
    switch (size) {
        case Size::Byte:
            return stream << "byte";
        case Size::Word:
            return stream << "word";
        case Size::Page:
            return stream << "page";
        case Size::Unsized:
            return stream << "unsized";
    }
    // TODO: Error
    throw;
}

ModeData::ModeData(int dataSize, bool usesRegister, bool isDirect, Size defaultSize)
    :   dataSize{dataSize},
        usesRegister{usesRegister},
        isDirect{isDirect},
        defaultSize{defaultSize} {}

const ModeData& ModeData::get(Mode mode) {
    static std::map<Mode, ModeData> modeDatas {
//        {Mode::Implied, {0, false, false, Size::Unsized}},
        {Mode::Register, {0, true, false, Size::Unsized}},
        {Mode::Direct, {2, false, true, Size::Word}},
        {Mode::Immediate, {1, false, false, Size::Byte}},
        {Mode::ImmediateAddress, {2, false, false, Size::Word}},
        {Mode::Indirect, {0, true, true, Size::Unsized}},
        {Mode::Offset, {1, true, true, Size::Page}},
    };
    return modeDatas[mode];
}

ModeData::ModeData() {}

Address::Address(Mode mode) : mode{mode}, reg{RID::A} {
    if (ModeData::get(mode).usesRegister) {
        throw; // TODO: Error stuff.
    }
}

Address::Address(RID reg) : Address{Mode::Register, reg} {}

Address::Address(Mode mode, RID reg) : mode{mode}, reg{reg} {
    if (!ModeData::get(mode).usesRegister) {
        throw; // TODO: Error stuff.
    }
}

//Address::Address() : Address{Mode::Implied} {}

std::ostream& Address::print(std::ostream& stream, const std::string& argument) const {
    switch (this->mode) {
        case Mode::Register:
            stream << this->reg;
            break;
        case Mode::Direct:
            stream << '[' << argument << ']';
            break;
        case Mode::ImmediateAddress:
        case Mode::Immediate:
            stream << argument;
            break;
        case Mode::Indirect:
            stream << '[' << this->reg << ']';
            break;
        case Mode::Offset:
            stream << '[' << this->reg << ", " << argument << ']';
            break;
    }
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Address& address) {
    ModeData modeData = ModeData::get(address.mode);
    return address.print(stream, (modeData.dataSize == 2) ? "addr" : "imm");

    if (modeData.isDirect) {
        stream << "[";
    }

    if (modeData.usesRegister) {
        stream << address.reg;
        if (modeData.dataSize) {
            stream << ", ";
        }
    }

    switch (modeData.dataSize) {
        case 1:
            stream << "imm";
            break;
        case 2:
            stream << "addr";
            break;
        default:
            break;
    }

    if (modeData.isDirect) {
        stream << "]";
    }
    return stream;
}

std::strong_ordering operator<=>(
    const Address& address0,
    const Address& address1
) {
    if (address0.mode == address1.mode) {
        return address0.reg <=> address1.reg;
    }
    return address0.mode <=> address1.mode;
}

bool operator==(const Address& address0, const Address& address1) {
    return address0.mode == address1.mode && address0.reg == address1.reg;
}


SizedAddress::SizedAddress(Mode mode)
    : SizedAddress{Address{mode}} {}

SizedAddress::SizedAddress(RID reg)
    : SizedAddress{Address{reg}} {}

SizedAddress::SizedAddress(Mode mode, RID reg)
    : SizedAddress{{mode, reg}} {}

SizedAddress::SizedAddress(Address address)
    : SizedAddress{address, ModeData::get(address.mode).defaultSize} {}

SizedAddress::SizedAddress(Address address, Size size)
: address{address}, size{size} {
}

std::ostream& SizedAddress::print(std::ostream& stream, std::uint16_t argument) const {
    std::string arg = "";
    switch (this->size) {
        case Size::Byte:
        case Size::Page:
            arg = std::format("{:02x}", argument & 0xff);
            break;
        case Size::Word:
            arg = std::format("{:04x}", argument);
            break;
        default:
            break;
    }
    return this->address.print(stream, arg);
}

std::ostream& SizedAddress::print(std::ostream& stream, const std::string& argument) const {
    return this->address.print(stream, argument);
}

std::ostream& operator<<(
    std::ostream& stream,
    const SizedAddress& sizedAddress
) {
    return stream << sizedAddress.address << " (" << sizedAddress.size << ")";
}


AddressingMode::AddressingMode(std::vector<SizedAddress> mode): mode{mode} {

}

AddressingMode::AddressingMode(std::initializer_list<SizedAddress> mode)
    : mode{mode} {
}

bool AddressingMode::isImplied() const {
    return this->mode.size() <= 0;
}

std::ostream& AddressingMode::print(std::ostream& stream, std::uint16_t argument) const {
    bool first = true;
    for (const SizedAddress& address : this->mode) {
        if (!first) {
            stream << ", ";
        }
        address.print(stream, argument);
        first = false;
    }
    return stream;
}

std::ostream& AddressingMode::print(std::ostream& stream, const std::string& argument) const {
    bool first = true;
    for (const SizedAddress& address : this->mode) {
        if (!first) {
            stream << ", ";
        }
        address.print(stream, argument);
        first = false;
    }
    return stream;
}


std::ostream& operator<<(std::ostream& stream, const AddressingMode& mode) {
    bool first = true;
    for (const SizedAddress& address : mode.mode) {
        if (!first) {
            stream << ", ";
        }
        stream << address;
        first = false;
    }
    return stream;
}

bool operator==(const AddressingMode& mode0, const AddressingMode& mode1) {
    return mode0.mode == mode1.mode;
}

std::strong_ordering operator<=>(
    const AddressingMode& mode0,
    const AddressingMode& mode1
) {
    return mode0.mode <=> mode1.mode;
}

