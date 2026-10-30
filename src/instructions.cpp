#include "InstructionSet.hpp"
#include "Mode.hpp"
#include <iostream>
#include <iomanip>

InstructionSet::InstructionSet() {
    noOpOpcode = instruction("nop", {})
        .opcode;

    haltOpcode = instruction("hlt", {})
        .add({Line::halt})
        .opcode;

    interuptOpcode = interupt()
        .addPushStart()
        .addPush(RID::PcL)
        .addPush(RID::PcH)
        .addPushEnd(RID::F)
        .add(RID::Alu, RID::PcL, {Line::zero})
        .add(RID::Alu, RID::Temp, {Line::zero})
        .add(RID::Alu, RID::PcH, {Line::inc})
        .add({Line::nop})
        .add({Line::clear_irq})
        .opcode;

    instructionLoads(RID::A);
    instruction("mov", {{RID::A}, {Mode::Indirect, RID::CD}})
        .add(RID::C, RID::MarL)
        .add(RID::D, RID::MarH)
        .add(RID::Mem, RID::A);
    instruction("mov", {{RID::A}, {Mode::Offset, RID::C}})
        .addLoadImm(RID::MarH)
        .add(RID::C, RID::MarL)
        .add(RID::Mem, RID::A);

    instructionLoads(RID::C);
    instructionLoads(RID::D);
    instructionLoads(RID::F);

    instructionStores(RID::A);
    instruction("mov", {{Mode::Offset, RID::C}, {RID::A}})
        .addLoadImm(RID::MarH)
        .add(RID::C, RID::MarL)
        .add(RID::A, RID::Mem);
    instruction("mov", {{Mode::Indirect, RID::CD}, {RID::A}})
        .add(RID::C, RID::MarL)
        .add(RID::D, RID::MarH)
        .add(RID::A, RID::Mem);

    instructionStores(RID::C);
    instructionStores(RID::D);

    instructionTransfer(RID::C, RID::A);
    instructionTransfer(RID::A, RID::C);
    instructionTransfer(RID::D, RID::A);
    instructionTransfer(RID::A, RID::D);
    instructionTransfer(RID::D, RID::C);
    instructionTransfer(RID::C, RID::D);

    instructionTransfer(RID::F, RID::A);
    instructionTransfer(RID::A, RID::F);
    instructionTransfer(RID::Sp, RID::A);
    instructionTransfer(RID::A, RID::Sp);

    instructionXch(RID::A, RID::C);
    instructionXch(RID::A, RID::D);
    instructionXch(RID::C, RID::D);

    instructionXchMem(RID::A);
    instructionXchMem(RID::C);
    instructionXchMem(RID::D);


    instructionAluBinary("add", Line::add);
    instructionAluBinary("addc", Line::add, Flag::C);

    instructionAluBinary("sub", Line::sub, FlagSet::always(), true);
    instructionAluBinary("subb", Line::sub, Flag::C, true);

    instructionAluBinaryDirect("cmp", Line::sub, FlagSet::always(), true, false)
        .add(RID::Temp, RID::A);
    instructionAluBinaryImm("cmp", Line::sub, FlagSet::always(), true, false)
        .add(RID::Temp, RID::A);

    instructionAluBinaryDirect("cmpb", Line::sub, Flag::C, true, false)
        .add(RID::Temp, RID::A);
    instructionAluBinaryImm("cmpb", Line::sub, Flag::C, true, false)
        .add(RID::Temp, RID::A);

    instructionAluUnary("not", RID::A, Line::b_not);

    instructionAluBinary("and", Line::b_and);
    instructionAluBinary("or", Line::b_or);
    instructionAluBinary("xor", Line::b_xor);
    instructionAluBinary("bit", Line::b_and, FlagSet::never(), false, false);

    instructionAluUnary("inc", RID::A, Line::inc);
    instructionAluUnary("dec", RID::A, Line::dec);
    instructionAluUnary("inc", RID::C, Line::inc);
    instructionAluUnary("dec", RID::C, Line::dec);
    instructionAluUnary("inc", RID::D, Line::inc);
    instructionAluUnary("dec", RID::D, Line::dec);

    instructionAluUnaryMem("inc", Line::inc);
    instructionAluUnaryMem("dec", Line::dec);
    instructionAluUnaryIndirect("inc", Line::inc);
    instructionAluUnaryIndirect("dec", Line::dec);

    instructionAluUnary("incc", RID::A, Line::passthrough, Flag::C);
    instructionAluUnary("incc", RID::C, Line::passthrough, Flag::C);
    instructionAluUnary("incc", RID::D, Line::passthrough, Flag::C);

    instructionAluUnary("decb", RID::A, Line::dec, Flag::C);
    instructionAluUnary("decb", RID::C, Line::dec, Flag::C);
    instructionAluUnary("decb", RID::D, Line::dec, Flag::C);

    instructionAluUnaryMem("incc", Line::passthrough, Flag::C);
    instructionAluUnaryMem("decb", Line::dec, Flag::C);
    instructionAluUnaryIndirect("incc", Line::passthrough, Flag::C);
    instructionAluUnaryIndirect("decb", Line::dec, Flag::C);

    instructionAluUnary("shl", RID::A, Line::lsh);
    instructionAluUnary("rol", RID::A, Line::lsh, Flag::C);

    instruction("shr", {{RID::A}})
        .add(RID::A, RID::Temp)
        .add(RID::Alu, RID::Temp, {Line::lsh, Line::flip, Line::set_flags})
        .add(RID::Alu, RID::A, {Line::passthrough, Line::flip});

    instruction("ror", {{RID::A}})
        .add(RID::A, RID::Temp)
        .add(RID::Alu, RID::Temp, {Line::lsh, Line::flip, Line::set_flags})
            .with(Flag::C, {{Line::carry}})
        .add(RID::Alu, RID::A, {Line::passthrough, Line::flip});

    instructionAluUnaryMem("shl", Line::lsh);
    instructionAluUnaryMem("rol", Line::lsh, Flag::C);

    instruction("shr", {{Mode::Direct}})
        .addLoadDirect(RID::Temp)
        .add(RID::Alu, RID::Temp, {Line::lsh, Line::flip, Line::set_flags})
        .add(RID::Alu, RID::Temp, {Line::passthrough, Line::flip})
        .add(RID::Temp, RID::Mem);

    instruction("ror", {{Mode::Direct}})
        .addLoadDirect(RID::Temp)
        .add(RID::Alu, RID::Temp, {Line::lsh, Line::flip, Line::set_flags})
            .with(Flag::C, {{Line::carry}})
        .add(RID::Alu, RID::Temp, {Line::passthrough, Line::flip})
        .add(RID::Temp, RID::Mem);

    instructionJump("jmp", FlagSet::always());

    instruction("jmp", {{RID::CD}})
        .add(RID::C, RID::PcL)
        .add(RID::D, RID::PcH);

    instructionJump("jz", Flag::Z);
    instructionJump("jnz", Flag::NZ);
    instructionJump("jc", Flag::C);
    instructionJump("jnc", Flag::NC);
    instructionJump("jn", Flag::S);
    instructionJump("jp", Flag::NS);

    instructionPush(RID::A);
    instructionPush(RID::C);
    instructionPush(RID::D);
    instructionPush(RID::F);

    instructionPop(RID::A);
    instructionPop(RID::C);
    instructionPop(RID::D);
    instructionPop(RID::F);

    instruction("phc", {})
        .addPushStart()
        .addPush(RID::PcL)
        .addPushEnd(RID::PcH);

    instruction("ret", {})
        .addPopStart()
        .addPop(RID::PcH)
        .addPopEnd(RID::PcL)
            .with(FlagSet::always(), {{Line::counter_enable}})
        .add({Line::counter_enable})
        .add({Line::counter_enable});

    rtiOpcode = instruction("rti", {})
        .addPopStart()
        .addPop(RID::F)
        .addPop(RID::PcH)
        .addPopEnd(RID::PcL)
        .opcode;

    instruction("out", {{RID::A}})
        .add(RID::A, RID::Out);

    instruction("in", {{RID::A}})
        .add(RID::Kbd, RID::A);

    instruction("lcd", {{RID::A}})
        .add(RID::A, RID::Lcd, {Line::logic});

    instruction("lcc", {{RID::A}})
        .add(RID::A, RID::Lcd);

    instruction("prog", {})
        .add(RID::Prog, RID::None)
        .add(RID::Prog, RID::None)
        .add(RID::Prog, RID::None)
        .add(RID::Prog, RID::None)
        .add(RID::Prog, RID::None)
        .add(RID::Prog, RID::None)
        .add(RID::Prog, RID::None)
        .add(RID::Prog, RID::None)
        .add(RID::Prog, RID::None)
        .add(RID::Prog, RID::None)
        .add(RID::Prog, RID::None)
        .add(RID::Prog, RID::A);


    // New instructions

    instructionStores(RID::F);

    instruction("clr", {{RID::A}})
        .add(RID::Alu, RID::A, {Line::zero});

    instruction("clr", {{RID::C}})
        .add(RID::Alu, RID::C, {Line::zero});

    instruction("clr", {{RID::D}})
        .add(RID::Alu, RID::D, {Line::zero});
    
    instruction("flip", {{RID::A}})
        .add(RID::A, RID::Temp)
        .add(RID::Alu, RID::A, {Line::passthrough, Line::flip});
    
    instruction("mov", {{RID::AD}, {Mode::ImmediateAddress}})
        .addLoadImm(RID::A)
        .addLoadImm(RID::D);
    
    instruction("mov", {{RID::CD}, {Mode::ImmediateAddress}})
        .addLoadImm(RID::C)
        .addLoadImm(RID::D);
    
    // Instructions for FORTH

    instruction("mov", {{RID::D}, {Mode::Offset, RID::C}})
        .addLoadImm(RID::MarH)
        .add(RID::C, RID::MarL)
        .add(RID::Mem, RID::D);

    instruction("mov", {{Mode::Offset, RID::C}, {RID::D}})
        .addLoadImm(RID::MarH)
        .add(RID::C, RID::MarL)
        .add(RID::D, RID::Mem);
    
    instruction("xch", {{RID::A}, {Mode::Offset, RID::C}})
        .addLoadImm(RID::MarH)
        .add(RID::C, RID::MarL)
        .add(RID::A, RID::Temp)
        .add(RID::Mem, RID::A)
        .add(RID::Temp, RID::Mem);
    
    instruction("xch", {{RID::D}, {Mode::Offset, RID::C}})
        .addLoadImm(RID::MarH)
        .add(RID::C, RID::MarL)
        .add(RID::D, RID::Temp)
        .add(RID::Mem, RID::D)
        .add(RID::Temp, RID::Mem);
    
    instruction("mov", {{RID::A}, {Mode::Indirect, RID::C}})
        .add(RID::Alu, RID::Temp, {Line::minus_one})
        .add(RID::Alu, RID::MarH, {Line::dec})
        .add(RID::C, RID::MarL)
        .add(RID::Mem, RID::A);

    instruction("mov", {{Mode::Indirect, RID::C}, {RID::A}})
        .add(RID::Alu, RID::Temp, {Line::minus_one})
        .add(RID::Alu, RID::MarH, {Line::dec})
        .add(RID::C, RID::MarL)
        .add(RID::A, RID::Mem);
    
    instruction("mov", {{RID::D}, {Mode::Indirect, RID::C}})
        .add(RID::Alu, RID::Temp, {Line::minus_one})
        .add(RID::Alu, RID::MarH, {Line::dec})
        .add(RID::C, RID::MarL)
        .add(RID::Mem, RID::D);
    
    instruction("mov", {{Mode::Indirect, RID::C}, {RID::D}})
        .add(RID::Alu, RID::Temp, {Line::minus_one})
        .add(RID::Alu, RID::MarH, {Line::dec})
        .add(RID::C, RID::MarL)
        .add(RID::D, RID::Mem);
    
    instruction("fpush", {{RID::A}})
        .add(RID::C, RID::Temp)
        .add(RID::Alu, RID::C, {Line::dec})
        .add(RID::Alu, RID::Temp, {Line::minus_one})
        .add(RID::Alu, RID::MarH, {Line::dec})
        .add(RID::C, RID::MarL)
        .add(RID::A, RID::Mem);
    
    instruction("fpush", {{RID::D}})
        .add(RID::C, RID::Temp)
        .add(RID::Alu, RID::C, {Line::dec})
        .add(RID::Alu, RID::Temp, {Line::minus_one})
        .add(RID::Alu, RID::MarH, {Line::dec})
        .add(RID::C, RID::MarL)
        .add(RID::D, RID::Mem);
    
    instruction("fpop", {{RID::A}})
        .add(RID::Alu, RID::Temp, {Line::minus_one})
        .add(RID::Alu, RID::MarH, {Line::dec})
        .add(RID::C, RID::MarL)
        .add(RID::Mem, RID::A)
        .add(RID::C, RID::Temp)
        .add(RID::Alu, RID::C, {Line::inc});

    instruction("fpop", {{RID::D}})
        .add(RID::Alu, RID::Temp, {Line::minus_one})
        .add(RID::Alu, RID::MarH, {Line::dec})
        .add(RID::C, RID::MarL)
        .add(RID::Mem, RID::D)
        .add(RID::C, RID::Temp)
        .add(RID::Alu, RID::C, {Line::inc});


    instruction("fpush", {{RID::AD}})
        .add(RID::Alu, RID::Temp, {Line::minus_one})
        .add(RID::Alu, RID::MarH, {Line::dec})
        .add(RID::C, RID::Temp)
        .add(RID::Alu, RID::Temp, {Line::dec})
        .add(RID::Temp, RID::MarL)
        .add(RID::D, RID::Mem)
        .add(RID::Alu, RID::C, {Line::dec})
        .add(RID::C, RID::MarL)
        .add(RID::A, RID::Mem);

    instruction("fpop", {{RID::AD}})
        .add(RID::Alu, RID::Temp, {Line::minus_one})
        .add(RID::Alu, RID::MarH, {Line::dec})
        .add(RID::C, RID::MarL)
        .add(RID::Mem, RID::A)
        .add(RID::C, RID::Temp)
        .add(RID::Alu, RID::Temp, {Line::inc})
        .add(RID::Temp, RID::MarL)
        .add(RID::Mem, RID::D)
        .add(RID::Alu, RID::C, {Line::inc});
    
    instruction("jsr", {{Mode::ImmediateAddress}})
        .addPushStart()
        .addPush(RID::PcH)
        .addPushEnd(RID::PcL)
        .addLoadImm(RID::Temp)
        .addLoadImm(RID::PcH)
        .add(RID::Temp, RID::PcL);



    debugPrintOpcode = instruction("dbg_print", {{Mode::Direct}})
        .addLoadImm(RID::Temp)
        .addLoadImm(RID::Temp)
        .opcode;
    

}

