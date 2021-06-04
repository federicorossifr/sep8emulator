#include "Logger.h"
ostream* Logger::of;
bool	 Logger::level;
bool	 Logger::operand;
void Logger::logInstruction(string mnem,int op1,int op2,int l1,int l2) {
	if(!level) return;
	*of << "\t" << mnem << " ";
	printHex(op1,l1);
	*of << ",";
	printHex(op2,l2);
	*of << endl;
}

void Logger::logInstruction(string mnem,int op1,int op2) {
	logInstruction(mnem,op1,op2,6,6);
}

void Logger::logInstruction(string mnem,string op1,int op2,int l2) {
	if(!level) return;	
	*of << "\t" << mnem << " " << op1;
	*of << ",";
	printHex(op2,l2);
	*of << endl;
}

void Logger::logInstruction(string mnem,string op1,int op2) {
	logInstruction(mnem,op1,op2,6);
}

void Logger::logInstruction(string mnem,int op1,string op2,int l1) {
	if(!level) return;	
	*of << "\t" << mnem << " ";
	printHex(op1,l1);
	if(op2.length() > 0)
		*of << "," << op2;
	*of << endl;
}

void Logger::logInstruction(string mnem,int op1,string op2) {
	logInstruction(mnem,op1,op2,6);	
}


void Logger::logInstruction(string mnem) {
	if(!level) return;	
	*of << "\t" << mnem << endl;
}

void Logger::printHex(int val,int ndigit) {
	if(!level) return;
	if(operand) *of<<"$";
	*of << "0x" << hex << setw(ndigit) << setfill('0') << val;
}

void Logger::setOutputStream(ostream* o) {
	of = o;
	level = 1;
}

void Logger::logMessage(string message,bool cont/*=false*/) {
	if(!level) return;
	*of << message;
	if(!cont) *of << endl;
}

void Logger::logRegister(string name,int val,int size) {
	if(!level) return;	
	*of << "[" << name << "]\t";
	printHex(val,size);
	*of << endl;
}

void Logger::logIPAddress(int val) {
	if(!level) return;	
	*of << "[";
	printHex(val,6);
	*of << "]\t";
}