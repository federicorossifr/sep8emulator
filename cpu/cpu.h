#include <iostream>

#ifndef TYPES_H
#include "../types.h"
#define TYPES_H
#endif

#ifndef IOPIC_H
#include "../io/devices/IOPic.h"
#define IOPIC_H
#endif

using namespace std;


class IOModule;
class cpu
{
	mem_row_t al,ah,f,source,opcode,app0,app1,app2,app3; //8BIT REGISTERS
	addr_t ip,dp,sp,pmsp,dest,idtp; //24 BIT REGISTERS
	bool interrupted = false;
	IOPic* pic;
	IOModule* _io;
	Memory& _memory;

	addr_t fetchAddress(addr_t start,bool io=false);
	void writeAddress(addr_t,addr_t);
	void setFlag(flag_t fl) {
		f |= 0b00000001 << fl;
	}
	void clearFlag(flag_t fl) {
		f &= ~(0b00000001 << fl);
	}
	bool getFlag(flag_t fl) {  
		mem_row_t mask = 0b00000001;
		mask = mask << fl;
		return (f&mask) >> fl;
	}
	void computeFlags(alu_op,mem_row_t,mem_row_t);
	void computeFlags(alu_op,mem_row_t);
	void andOrFlags(mem_row_t);
	void conditionalJump(bool);
	void validateMemoryWriteAccess(addr_t t) { // US=0 SYSTEM MODE US=1 USER MODE
		t=t%MEM_SIZE;
		if(!getFlag(US)) return;
		if(getFlag(US) && ((t >> 21) == 0x0)) {
		    cout << hex << t << endl;			
			throw INVALID_MEM_ACCESS;
		}
	};
	void validateProtectedInstruction() {
		if(getFlag(US)) throw INVALID_PRI_INSTR;
	}
	addr_t getInterruptGate(int_type_t t) {
		addr_t gate = t;
		return idtp+(gate << 3);
	}
	void handleInterruptRequest(int_type_t t) {
		if(getFlag(US)) {
			addr_t tmp = sp;
			sp = pmsp;
			pmsp = tmp;			
		}
		sp-=3;writeAddress(pmsp,ip); //SAVE CURRENT INSTRUCTION POINTER
		_memory[--pmsp] = f; // SAVE CURRENT FLAG CONTENT
		f&=0x00; //RESET FLAG CONTENT
		ip = getInterruptGate(t);
		interrupted = false;		
	}
public:
	cpu(Memory& m);
	~cpu();
	void loadProgram(mem_row_t*,int,bool usr=false);
	void loadBios(mem_row_t*,int);
	void emulate();
	void dump();
	void setInterrupted() {interrupted=true;}
	void plugPIC(IOPic* p) {p->setInterruptWire(&interrupted);pic=p;}
	void plugIO(IOModule* i) {_io = i;}
};