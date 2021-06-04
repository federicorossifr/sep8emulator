#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;
class Logger
{
	static ostream* of;
	static bool level,operand;
public:
	static void setLevel(bool l) {level = l;}
	static void setOperand(bool o) {operand=o;}
	static void printHex(int val,int ndigit);
	static void logInstruction(string mnem,int op1,int op2);
	static void logInstruction(string mnem,int op1,int op2,int l1,int l2);
	static void logInstruction(string mnem,string op1,int op2);
	static void logInstruction(string mnem,string op1,int op2,int l2);
	static void logInstruction(string mnem,int op1,string op2);
	static void logInstruction(string mnem,int op1,string op2,int l1);
	static void logInstruction(string mnem);
	static void setOutputStream(ostream* o);
	static void logMessage(string msg,bool cont=false);
	static void logRegister(string name,int val,int size);
	static void logIPAddress(int val);
};