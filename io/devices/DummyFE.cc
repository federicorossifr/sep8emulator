#include "DummyFE.h"

mem_row_t DummyFE::getRegister(io_addr_t addr) {
	uint8_t index = (addr & 0x0003); //LAST TWO BIT
	switch(index) {
		case RBR: {
			return registers[RBR];
		}
		case RSR: {
			return registers[RSR];
		}
		default: return 0x0;
	}
}


void DummyFE::setRegister(io_addr_t addr,mem_row_t val) {
	uint8_t index = (addr & 0x0003); //LAST TWO BIT
	switch(index) {
		case RCR: 
			if(startCMD==val) {
				registers[RBR] = _backend->nextValue();
				registers[RSR] = 0x01; //Set finish bit
				if((registers[RCR]&0x02) == 0x02) { //If interrupts are enabled
					*_int_ref = true; 
					_io->notifyPIC();
				}
			} else {
				registers[RCR] = val;
			}
			break;
		default: break;
	}	
}

void DummyFE::setInterruptWire(bool* b) {
	_int_ref=b;
}