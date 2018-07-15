#ifndef CPU_H
#define CPU_H

#include "controller.h"
#include "PPU.h"
#include "cart.h"
#include "types.h"

struct status {
    u8 c : 1;
    u8 z : 1;
    u8 i : 1;
    u8 d : 1;
    u8 s : 2;
    u8 v : 1;
    u8 n : 1;
};

class CPU {
private:
    int tickCount = 0;
    int lastTickCount = 0;

    PPU* ppu = nullptr;
    controller* ctrl1 = nullptr;
    controller* ctrl2 = nullptr;

    u8 a = 0, x = 0, y = 0, s = 0;
    u16 pc = 0;

    status p;

    u8* ram = nullptr;

    bool nmiLow = false;
    bool lastNmiLow = false;
    bool nmiEdgeDetected = false;
    bool nmiActive = false;

    bool irqLow = false;
    bool irqLevelDetected = false;
    bool irqActive = false;

    void tick(int n);
    void tick();

    // Addressing mode tools
    u16 immAddr();
    u16 zpAddr();
    u16 zpxAddr();
    u16 zpyAddr();
    u16 absAddr();
    u16 absxAddr(u8 opcode);
    u16 absyAddr(u8 opcode);
    u16 indirxAddr();
    u16 indiryAddr(u8 opcode);

    void setZN(u8 val);

    void branch(u8 flag, u8 test);
    void compare(u16 addr, u8 reg);

    const char* opcodeNames[256] =
    //x0      x1          x2        x3            x4        x5        x6        x7          x8    x9        xa      xb    xc          xd        xe        xf
    {"BRK b","ORA (d,X)","cop b"  ,"ora d,S"    ,"Tsb d"  ,"ORA d"  ,"ASL d"  ,"ora [d]"  ,"PHP","ORA #"  ,"ASL A","phd","Tsb a"    ,"ORA a"  ,"ASL a"  ,"ora al",
     "BPL r","ORA (d),Y","Ora (d)","ora (d,S),Y","Trb d"  ,"ORA d,X","ASL d,X","ora [d],Y","CLC","ORA a,Y","Inc A","tcs","Trb a"    ,"ORA a,X","ASL a,X","ora al,X",
     "JSR a","AND (d,X)","jsl al" ,"and d,S"    ,"BIT d"  ,"AND d"  ,"ROL d"  ,"and [d]"  ,"PLP","AND #"  ,"ROL A","pld","BIT a"    ,"AND a"  ,"ROL a"  ,"and al",
     "BMI r","AND (d),Y","And (d)","and (d,S),Y","Bit d,X","AND d,X","ROL d,X","and [d],Y","SEC","AND a,Y","Dec A","tsc","Bit a,X"  ,"AND a,X","ROL a,X","and al,X",
     "RTI"  ,"EOR (d,X)","wdm"    ,"eor d,S"    ,"mvp s,d","EOR d"  ,"LSR d"  ,"eor [d]"  ,"PHA","EOR #"  ,"LSR A","phk","JMP a"    ,"EOR a"  ,"LSR a"  ,"eor al",
     "BVC r","EOR (d),Y","Eor (d)","eor (d,S),Y","mvn s,d","EOR d,X","LSR d,X","eor [d],Y","CLI","EOR a,Y","Phy"  ,"tcd","jmp al"   ,"EOR a,X","LSR a,X","eor al,X",
     "RTS"  ,"ADC (d,X)","per rl" ,"adc d,S"    ,"Stz d"  ,"ADC d"  ,"ROR d"  ,"adc [d]"  ,"PLA","ADC #"  ,"ROR A","rtl","JMP (a)"  ,"ADC a"  ,"ROR a"  ,"adc al",
     "BVS r","ADC (d),Y","Adc (d)","adc (d,S),Y","Stz d,X","ADC d,X","ROR d,X","adc [d],Y","SEI","ADC a,Y","Ply"  ,"tdc","Jmp (a,X)","ADC a,X","ROR a,X","adc al,X",
     "Bra r","STA (d,X)","brl rl" ,"sta d,S"    ,"STY d"  ,"STA d"  ,"STX d"  ,"sta [d]"  ,"DEY","Bit #"  ,"TXA"  ,"phb","STY a"    ,"STA a"  ,"STX a"  ,"sta al",
     "BCC r","STA (d),Y","Sta (d)","sta (d,S),Y","STY d,X","STA d,X","STX d,Y","sta [d],Y","TYA","STA a,Y","TXS"  ,"txy","Stz a"    ,"STA a,X","Stz a,X","sta al,X",
     "LDY #","LDA (d,X)","LDX #"  ,"lda d,S"    ,"LDY d"  ,"LDA d"  ,"LDX d"  ,"lda [d]"  ,"TAY","LDA #"  ,"TAX"  ,"plb","LDY a"    ,"LDA a"  ,"LDX a"  ,"lda al",
     "BCS r","LDA (d),Y","Lda (d)","lda (d,S),Y","LDY d,X","LDA d,X","LDX d,Y","lda [d],Y","CLV","LDA a,Y","TSX"  ,"tyx","LDY a,X"  ,"LDA a,X","LDX a,Y","lda al,X",
     "CPY #","CMP (d,X)","rep #"  ,"cmp d,S"    ,"CPY d"  ,"CMP d"  ,"DEC d"  ,"cmp [d]"  ,"INY","CMP #"  ,"DEX"  ,"wai","CPY a"    ,"CMP a"  ,"DEC a"  ,"cmp al",
     "BNE r","CMP (d),Y","Cmp (d)","cmp (d,S),Y","pei d"  ,"CMP d,X","DEC d,X","cmp [d],Y","CLD","CMP a,Y","Phx"  ,"stp","jml (a)"  ,"CMP a,X","DEC a,X","cmp al,X",
     "CPX #","SBC (d,X)","sep #"  ,"sbc d,S"    ,"CPX d"  ,"SBC d"  ,"INC d"  ,"sbc [d]"  ,"INX","SBC #"  ,"NOP"  ,"xba","CPX a"    ,"SBC a"  ,"INC a"  ,"sbc al",
     "BEQ r","SBC (d),Y","Sbc (d)","sbc (d,S),Y","pea a"  ,"SBC d,X","INC d,X","sbc [d],Y","SED","SBC a,Y","Plx"  ,"xce","jsr (a,X)","SBC a,X","INC a,X","sbc al,X"};

public:
    cart* gameCart = nullptr;

    CPU(PPU* ppu, controller* ctrl1, controller* ctrl2);
    ~CPU();

    void powerOn();
    void reset();
    
    void op();

    u8 readMemory(u16 addr);
    void writeMemory(u16 addr, u8 v);
};

#endif
