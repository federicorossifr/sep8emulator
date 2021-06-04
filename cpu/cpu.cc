#include "cpu.h"
#define CPU_H
#include "../io/io.h"
#include <algorithm>
#include <iostream>
#include <cstring>
#include <iomanip>



cpu::cpu(Memory& m):_memory(m) {
		fill(_memory.begin(),_memory.end(),0);
		ip=0xff0000;
		f=0x00;
		sp=0x0000;
		dp=0x0000;
		al=ah=0x00;
		interrupted = false;
}
cpu::~cpu() {
	return;
}

addr_t cpu::fetchAddress(addr_t start,bool io/*=false*/) {
	mem_row_t addr1 = _memory[start++];
	mem_row_t addr2 = _memory[start++];
	mem_row_t addr3 = (io)?0x00:_memory[start++];
	addr_t addr = ((addr_t)addr1) + ((addr_t)addr2 << 8) + ((addr_t) addr3 << 16);
	ip+=(io)?2:0;
	return addr;
}

void cpu::writeAddress(addr_t start,addr_t value) {
	_memory[start++] = value&0x0000ff;
	_memory[start++] = (mem_row_t)((value&0x00ff00)>>8);
	_memory[start++] = (mem_row_t)((value&0xff0000)>>16);
}

void cpu::loadProgram(mem_row_t* program,int size,bool usr/*=0*/) {
	if(!usr) {
		if(size > SYS_END - SYS_START) {
			Logger::logMessage("SYS exceeds max allowed size");
			exit(1);			
		}
		memcpy(_memory.begin(),program,size);
	}
	else {
		if(size > USR_END - USR_START) {
			Logger::logMessage("USR exceeds max allowed size");
			exit(1);			
		}		
		memcpy(_memory.begin()+USR_START,program,size);
	}
}

void cpu::loadBios(mem_row_t* bios,int size) {
	if(size > BIOS_END - BIOS_START) {
		Logger::logMessage("BIOS exceeds max allowed size");
		exit(1);
	}
	memcpy(_memory.begin()+0xff0000,bios,size);
}

void cpu::dump() {
	Logger::logRegister("AL",al,2);
	Logger::logRegister("AH",ah,2);
	Logger::logRegister("IDTP",idtp,6);
	Logger::logRegister("SP",sp,6);
	Logger::logRegister("PMSP",pmsp,6);
	Logger::logRegister("F",f,2);
	Logger::logRegister("CF",getFlag(CF),1);
	Logger::logRegister("ZF",getFlag(ZF),1);
	Logger::logRegister("SF",getFlag(SF),1);
	Logger::logRegister("OF",getFlag(OF),1);
	Logger::logRegister("IF",getFlag(IF),1);
	Logger::logRegister("US",getFlag(US),1);
}
void cpu::andOrFlags(mem_row_t res){
	clearFlag(CF);
	clearFlag(OF);
	((res&0X80)>>7)?setFlag(SF):clearFlag(SF);
	(res)?clearFlag(ZF):setFlag(ZF);
}
void cpu::computeFlags(alu_op op,mem_row_t op1) {
	switch(op){
		case SHL:
		{
			((op1&0X80)>>7)?setFlag(CF):clearFlag(CF);
			clearFlag(OF);
			(op1&0x7f)?clearFlag(ZF):setFlag(ZF);
			(op1&0x40)?setFlag(SF):clearFlag(SF);
			break;
		}
		case SHR:
		{
			clearFlag(SF);
			clearFlag(OF);
			(op1&0xfe)?clearFlag(ZF):setFlag(ZF);
			(op1&0X01)?setFlag(CF):clearFlag(CF);
			break;
		}
		case NOT:
		{
			clearFlag(CF);
			clearFlag(OF);
			(op1!=0xff)?clearFlag(ZF):setFlag(ZF);
			(!(op1&0X80))?setFlag(SF):clearFlag(SF);
		}
		default:break;

	}

}
void cpu::computeFlags(alu_op op,mem_row_t op1,mem_row_t op2) {
	switch(op) {
		case ADD:
		{
			int sign_res = ((signed int)op1)+((signed int)op2);
			int8_t sop1 = (signed int)op1;
			int8_t sop2 = (signed int)op2;
			(0xFF - op1 < op2)?setFlag(CF):clearFlag(CF); //as natural
			(( sign_res<0 && sop1>0 && sop2>0)||(sign_res>0 && sop1<0 && sop2<0))? setFlag(OF):clearFlag(OF); // as integer
			(!(mem_row_t)(op1+op2))? setFlag(ZF):clearFlag(ZF);
			((sign_res) < 0)? setFlag(SF):clearFlag(SF);
			break;
		}
		case SUB:
		{
			int sign_res = ((signed int)op1)-((signed int)op2);
			(op1 < op2)?setFlag(CF):clearFlag(CF);
			((sign_res) == 0x0)? setFlag(ZF):clearFlag(ZF);
			((sign_res) < 0)? setFlag(SF):clearFlag(SF);
			break;
		}
		case AND: 
			andOrFlags(op1 & op2); break;
		case OR: 
			andOrFlags(op1 | op2); break;
		case CMP: 
			//op2 is source,op 1 is dest
			computeFlags(SUB,op1,op2); break;
		default:break;
	}
}
void cpu::conditionalJump(bool f) {
	if(f)
		ip = dest;
}

void cpu::emulate() {
	bool running = true;
	Logger::logMessage("[IP]\t\t[OP]\t[DECODED]");
	while(running) {
		//FETCH
		try {
		mem_row_t rawInstruction = _memory[ip++];
		opcode_t format = static_cast<opcode_t>(rawInstruction >> 5);
		Logger::setOperand(false);
		Logger::logIPAddress(ip-1);
		Logger::printHex(rawInstruction,2);
		switch(format) {
			case F2: //OP (DP),AL/AH
				source = _memory[dp]; break;
			case F3: // OP AL/AH,(DP)
				dest = dp; break;
				validateMemoryWriteAccess(dest);							
			case F4: //OP imm,AL/AH
				Logger::setOperand(true);
				source = _memory[ip++]; break;
			case F5: // OP src_addr,AL/AH
				source = _memory[fetchAddress(ip)]; ip+=3;break;
			case F6:	// OP AH/AL, dst_addr 
			case F7:    // JMP addr
				dest = fetchAddress(ip);
				validateMemoryWriteAccess(dest);										
				ip+=3; break;
			default: break;
		}
		//DECODE&EXECUTE
		switch(rawInstruction) {
			//================================================================	F0		
			case 0x1:
				Logger::logInstruction("NOP"); break;
			case 0x2: //MOV AL,AH
				Logger::logInstruction("MOV AL,AH");
				ah = al; break;
			case 0x3: //MOV AH,AL
				Logger::logInstruction("MOV AH,AL");
				al = ah; break;
			case 0x4: //INC DP
				Logger::logInstruction("INC DP");
				dp++; break;
			case 0x5: //SHL AL
				Logger::logInstruction("SHL AL");			
			 	computeFlags(SHL,al); al = al << 1; break;
			case 0x6: //SHR AL
				Logger::logInstruction("SHR AL");						
				computeFlags(SHR,al); al = al >> 1;break; 
			case 0x7: //NOT AL
				Logger::logInstruction("NOT AL");						
				computeFlags(NOT,al); al = ~al;break;
			case 0x8: //SHL AH
				Logger::logInstruction("SHL AH");						
				computeFlags(SHL,ah); ah = ah << 1;break; 
			case 0x9: //SHR AH
				Logger::logInstruction("SHR AH");						
				computeFlags(SHR,ah); ah = ah >> 1;break; 
			case 0xA: //NOT AH
				Logger::logInstruction("NOT AH");						
				computeFlags(NOT,ah); ah = ~ ah;break; 
			case 0xB: // PUSH AL
				Logger::logInstruction("PUSH AL");									
				_memory[--sp] = al; break;
			case 0xC: //POP AL
				Logger::logInstruction("POP AL");												
				al = _memory[sp++]; break;
			case 0xD: // PUSH AH
				Logger::logInstruction("PUSH AH");												
				_memory[--sp] = ah; break;
			case 0xE: //POP AH
				Logger::logInstruction("POP AH");												
				ah = _memory[sp++]; break;				
			case 0xF: // PUSH DP
				Logger::logInstruction("PUSH DP");												
				writeAddress(sp-3,dp);sp-=3; break;
			case 0x10: //POP DP
				Logger::logInstruction("POP DP");												
				dp = fetchAddress(sp); sp+=3; break;
			case 0x11: //RET
				Logger::logInstruction("RET");												
				ip = fetchAddress(sp); sp+=3; break;
			case 0x12: //IRET
				Logger::logInstruction("IRET");												
				validateProtectedInstruction();		
				f = _memory[pmsp++]; ip = fetchAddress(pmsp); pmsp+=3; 
				if(getFlag(US)) {
					addr_t tmp = sp;
					sp = pmsp;
					pmsp = tmp;
				}
				break;
			case 0x13: //CLI
				Logger::logInstruction("CLI");												
				validateProtectedInstruction();			
				clearFlag(IF); break;
			case 0x14: //STI
				Logger::logInstruction("STI");												
				validateProtectedInstruction();			
				setFlag(IF); break;
			case 0x16: //STUM
				Logger::logInstruction("STUM");												
				validateProtectedInstruction();			
				setFlag(US); //NO BREAK NEEDED HERE IT SHARES INSTR. WITH EXCHSP
			case 0x15: //EXCHSP
			{
				Logger::logInstruction("EXCHSP");													
				validateProtectedInstruction();				
				addr_t tmp = sp;
				sp = pmsp;
				pmsp = tmp;
				break;
			}


			//================================================================



			//================================================================	F1			
			case 0x20: //IN offset,AL
			{
				validateProtectedInstruction();				
				io_addr_t addr = fetchAddress(ip,true);
				Logger::logInstruction("IN",addr,"AL",4);
				IODeviceFE* dev = _io->find(addr);
				if(dev)
					al = dev->getRegister(addr);
				break;
			}
			case 0x21: //OUT AL,offset
			{
				validateProtectedInstruction();
				io_addr_t addr = fetchAddress(ip,true);
				Logger::logInstruction("OUT","AL",addr,4);
				IODeviceFE* dev = _io->find(addr);
				if(dev)
					dev->setRegister(addr,al);
				break;
			} 
			case 0x22: //MOV operand,DP
				dp=fetchAddress(ip);
				Logger::setOperand(true);
				Logger::logInstruction("MOV",dp,"DP");ip+=3; break;
			case 0x23: //MOV operand,SP
				Logger::setOperand(true);			
				Logger::logInstruction("MOV",(sp=fetchAddress(ip)),"SP");ip+=3; break;
			case 0x24: //MOV (address),DP
			{
				addr_t src_addr = _memory[fetchAddress(ip)]; ip+=3;
				Logger::logInstruction("MOV",src_addr,"DP");							
				dp = fetchAddress(src_addr); break;
			}
			case 0x25: //MOV DP,(address)	
			{
				addr_t dst_addr = fetchAddress(ip); ip+=3;
				Logger::logInstruction("MOV","DP",dst_addr);											
				validateMemoryWriteAccess(dst_addr);
				validateMemoryWriteAccess(dst_addr+1);
				validateMemoryWriteAccess(dst_addr+2);
				writeAddress(dst_addr,dp); break;
			}
			case 0x26: //LIDTP ---> DIFFERENT FROM BOOK DUE TO AUTHOR ERROR
			{
				validateProtectedInstruction();
				addr_t idtp_addr = fetchAddress(ip); ip+=3;
				Logger::logInstruction("LIDTP",idtp_addr,"");
				idtp = idtp_addr;
				break;
			}
			//================================================================


			//================================================================ F2
			case 0x40: //MOV (DP),AH/AL => F2 FORMAT OPERAND IN source
				Logger::logInstruction("MOV (DP),AL");
				al = source; break;
			case 0x41: //CMP (DP),AL
				Logger::logInstruction("CMP (DP),AL");			
				(al == source)? setFlag(ZF):clearFlag(ZF); break;
			case 0x42: //ADD (DP),AL
				Logger::logInstruction("ADD (DP),AL");			
				computeFlags(ADD,al,source);
				al+=source;  
				break; 			
			case 0x43: //SUB (DP),AL
				Logger::logInstruction("SUB (DP),AL");			
				computeFlags(SUB,al,source);
				al-=source;
				break;
			case 0x44: //AND (DP),AL
				Logger::logInstruction("AND (DP),AL");			
				computeFlags(AND,al,source);
				al&= source;
				break;
			case 0x45: //OR (DP),AL
				Logger::logInstruction("OR (DP),AL");			
				computeFlags(OR,al,source);
				al|= source;
				break;

			case 0x46: //MOV (DP),AH => F2 FORMAT OPERAND IN source
				Logger::logInstruction("MOV (DP),AH");			
				ah = source; break;
			case 0x47: //CMP (DP),AH
				Logger::logInstruction("CMP (DP),AH");						
				computeFlags(CMP,ah,source); break;
			case 0x48: //ADD (DP),AH
				Logger::logInstruction("ADD (DP),AH");						
				computeFlags(ADD,source,ah);
				ah+=source; 
				break;
			case 0x49: //SUB (DP),AH
				Logger::logInstruction("SUB (DP),AH");						
				computeFlags(SUB,source,ah);
				ah-=source;
				break;
			case 0x4A: //AND (DP),AH
				Logger::logInstruction("AND (DP),AH");						
				computeFlags(AND,source,ah);
				ah&=source;
				break;			
			case 0x4B: // OR(DP),AH
				Logger::logInstruction("OR (DP),AH");									
				computeFlags(OR,source,ah); 
				ah|=source;
				break;	
			//================================================================

			
			//================================================================ F3
			case 0x60: //MOV AL,(DP)
				Logger::logInstruction("MOV AL,(DP)");			
				_memory[dest] = al; 
				break;				
			case 0x61: //MOV AH,(DP)
				Logger::logInstruction("MOV AH,(DP)");						
				_memory[dest] = ah; break;
			//================================================================


			//================================================================ F4
				//(OPERAND IN source)
			case 0x80: //MOV operand,AL
				Logger::logInstruction("MOV",source,"AL",2);
				al = source; break;
			case 0x81: //CMP operand,AL
				Logger::logInstruction("CMP",source,"AL",2);			
				computeFlags(CMP,al,source); break;
			case 0x82: //ADD operand,AL
				Logger::logInstruction("ADD",source,"AL",2);						
				computeFlags(ADD,al,source);				
				al+=source;    
				break; 
			case 0x83: //SUB operand,AL
				Logger::logInstruction("SUB",source,"AL",2);						
				computeFlags(SUB,al,source);
				al-=source; 
				break; 
			case 0x84: //AND operand,AL
				Logger::logInstruction("AND",source,"AL",2);			
				computeFlags(AND,al,source);
				al&= source;
				break;
			case 0x85: //OR operand,AL
				Logger::logInstruction("OR",source,"AL",2);						
				computeFlags(OR,al,source);
				al|= source;
				break;
			case 0x86: //MOV operand,AH
				Logger::logInstruction("MOV",source,"AH",2);						
				ah = source; break;
			case 0x87: //CMP operand,AH
				Logger::logInstruction("CMP",source,"AH",2);									
				computeFlags(CMP,ah,source); break;
			case 0x88: //ADD operand,AH
				Logger::logInstruction("ADD",source,"AH",2);									
				computeFlags(ADD,ah,source);
				ah+=source; 
				break; 
			case 0x89: //SUB operand,AH
				Logger::logInstruction("SUB",source,"AH",2);									
				computeFlags(SUB,ah,source);
				ah-=source;
				break; 
			case 0x8A: //AND operand,AH
				Logger::logInstruction("AND",source,"AH",2);									
				computeFlags(AND,ah,source);
				ah&= source;
				break;
			case 0x8B: //OR operand,AH
				Logger::logInstruction("OR",source,"AH",2);									
				computeFlags(OR,ah,source);
				ah|= source;
				break;
			case 0x8C: //INT type
				Logger::logInstruction("INT",source,"",2);									
				handleInterruptRequest(source); break;
			//================================================================


			//================================================================ F5
			case 0xA0: //MOV ADDR,AL
				Logger::logInstruction("MOV",source,"AL",2);
				al = source; break;
			case 0xA1: //CMP ADDR,AL
				Logger::logInstruction("CMP",source,"AL",2);
				computeFlags(CMP,al,source); break;
			case 0xA2: //ADD ADDR,AL
				Logger::logInstruction("ADD",source,"AL",2);
				al+=source;	computeFlags(ADD,al,source); break;
			case 0xA3: //SUB ADDR,AL
				Logger::logInstruction("SUB",source,"AL",2);
				al-=source;	computeFlags(SUB,al,source); break;
			case 0xA4: //AND ADDR,AL
				Logger::logInstruction("AND",source,"AL",2);
				al&=source;	computeFlags(AND,al,source); break;
			case 0xA5: //OR ADDR,AL
				Logger::logInstruction("OR",source,"AL",2);
				al|=source;	computeFlags(OR,al,source); break;
			case 0xA6: //MOV ADDR,AH
				Logger::logInstruction("MOV",source,"AH",2);
				ah = source; break;
			case 0xA7: //CMP ADDR,AH
				Logger::logInstruction("CMP",source,"AH",2);
				computeFlags(CMP,ah,source); break;
			case 0xA8: //ADD ADDR,AH
				Logger::logInstruction("ADD",source,"AH",2);				
				ah+=source;	computeFlags(ADD,ah,source); break;
			case 0xA9: //SUB ADDR,AH
				Logger::logInstruction("SUB",source,"AH",2);				
				ah-=source;	computeFlags(SUB,ah,source); break;
			case 0xAA: //AND ADDR,AH
				Logger::logInstruction("AND",source,"AH",2);				
				ah&=source;	computeFlags(AND,ah,source); break;
			case 0xAB: //OR ADDR,AH
				Logger::logInstruction("OR",source,"AH",2);				
				ah|=source;	computeFlags(OR,ah,source); break;

			//================================================================


			//================================================================== F6
			case 0xC0: //MOV AL,ADDR
				Logger::logInstruction("MOV","AL",dest);												
				_memory[dest] = al; break;
			case 0xC1: //MOV AH,ADDR
				Logger::logInstruction("MOV","AH",dest);
				_memory[dest] = ah; break;
			//================================================================


			//================================================================== F7
			case 0xE0: //JMP ADDR
				Logger::logInstruction("JMP",dest,"");
				ip = dest; break;
			case 0xE1: //JE ADDR	
				Logger::logInstruction("JE",dest,"");			
				conditionalJump(getFlag(ZF)); break;
			case 0xE2: //JNE ADDR	
				Logger::logInstruction("JNE",dest,"");			
				conditionalJump(!getFlag(ZF)); break;
			case 0xE3: //JA ADDR	
				Logger::logInstruction("JA",dest,"");			
				conditionalJump(!(getFlag(CF)||getFlag(ZF))); break; //both flags have to be 0
			case 0xE4: //JAE ADDR	
				Logger::logInstruction("JAE",dest,"");			
				conditionalJump(!getFlag(CF)); break;
			case 0xE5: //JB ADDR	
				Logger::logInstruction("JB",dest,"");			
				conditionalJump(getFlag(CF)); break;
			case 0xE6: //JBE ADDR	
				Logger::logInstruction("JBE",dest,"");			
				conditionalJump((getFlag(CF)&&getFlag(ZF))); break;	//both flags have to be 1
			case 0xE7: //JG ADDR
				Logger::logInstruction("JG",dest,"");			
				conditionalJump((getFlag(CF)&&getFlag(ZF))); break;	//both flags have to be 1
			case 0xE8: //JGE ADDR	
				Logger::logInstruction("JGE",dest,"");			
				conditionalJump((getFlag(SF)==getFlag(OF))); break; //flags have to be equal
			case 0xE9: //JL ADDR	
				Logger::logInstruction("JL",dest,"");			
				conditionalJump((getFlag(SF)!=getFlag(OF))); break; //flags have to be different
			case 0xEA: //JLE ADDR	
				Logger::logInstruction("JLE",dest,"");			
				conditionalJump((getFlag(ZF)==1)||(getFlag(OF)!=getFlag(SF))); break;
			case 0xEB: //JZ ADDR
				Logger::logInstruction("JZ",dest,"");			
				conditionalJump(getFlag(ZF)); break;	
			case 0xEC: //JNZ ADDR	
				Logger::logInstruction("JNZ",dest,"");			
				conditionalJump(!getFlag(ZF)); break;
			case 0xED: //JC ADDR	
				Logger::logInstruction("JC",dest,"");			
				conditionalJump(getFlag(CF)); break;
			case 0xEE: //JNC ADDR	
				Logger::logInstruction("JNC",dest,"");			
				conditionalJump(!getFlag(CF)); break;
			case 0xEF: //JO ADDR	
				Logger::logInstruction("JO",dest,"");			
				conditionalJump(getFlag(OF)); break;
			case 0xF0: //JNO ADDR	
				Logger::logInstruction("JNO",dest,"");			
				conditionalJump(!getFlag(OF)); break;
			case 0xF1: //JS ADDR
				Logger::logInstruction("JS",dest,"");			
				conditionalJump(getFlag(SF)); break;	
			case 0xF2: //JNS ADDR
				Logger::logInstruction("JNS",dest,"");						
				conditionalJump(!getFlag(SF)); break;
			case 0xF3: //CALL ADDR   
				Logger::logInstruction("CALL",dest,"");			
				sp-=3; writeAddress(sp,ip); ip = dest; break;
			case 0x0: //HLT
				Logger::logInstruction("HLT");
				validateProtectedInstruction();
				running=false; break;
			default:
				cout << "\tNVI";
				running=false;
		}

		if(interrupted && getFlag(IF)) {
			int_type_t int_type = pic->getIntType();
			Logger::logMessage("\t[CPU] Got interrupt request: ",1);
			Logger::printHex(int_type,2);
			Logger::logMessage("");
			handleInterruptRequest(int_type);
		}
		} catch(int ex_code) {
			Logger::logMessage("\n[CPU] Exception: ",1);
			Logger::printHex(ex_code,2);	
			Logger::logMessage("");					
			handleInterruptRequest(ex_code);
		}
	}
	Logger::logMessage("[MACHINE] Halted");
}	