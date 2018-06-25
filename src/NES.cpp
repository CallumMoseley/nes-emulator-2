#include "NES.h"

NES::NES() : cpu() {}

void NES::start() {
    cpu.powerOn();

    for (;;) {
        cpu.op();
    }
}
