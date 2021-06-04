# {	  		//0xF0 	0xF1 	0xF2 	0xF3 	0xF4 	0xF5 	0xF6	0xF7
# enum {	iSN1,	iSN2,	iSN3,	iSN4,	iCMD,	iBR,	iRSR,	iDMAPTR};
# LOAD FROM HDD SYS AND USR PROGRAM
# LOAD SYS PROGRAM FROM SECTOR 0
MOV $0x00,AL
OUT AL,0x00F0
OUT AL,0x00F1
OUT AL,0x00F2
OUT AL,0x00F3
# PREPARE DMA TABLE IN MEMORY AT 0x00 FOR SYSTEM
MOV $0x000000,DP
MOV DP,0x000000
MOV DP,0x000004
MOV $0x02,AL
MOV AL,0x000003
# TELL DMA DEVICE WHERE TO GET PARAMS WITH 3 OUTS FROM LEAST TO MOST SIG. BYTE
MOV $0x00,AL
OUT AL,0x00F7
OUT AL,0x00F7
OUT AL,0x00F7
# BUILD COMMAND IN AH
# READ - DMA - NO INT 0X04
MOV $0x04,AL
OUT AL,0x00F4
# CHECK FOR FINISH BIT IN RSR REGISTER
IN 0x00F6,AL
# AND WITH FINISH BIT
CMP $0x01,AL
JNE 0xff0030
# LOAD USR PROGRAM FROM SECTOR 2
MOV $0x02,AL
OUT AL,0x00F0
MOV $0x00,AL
OUT AL,0x00F1
OUT AL,0x00F2
OUT AL,0x00F3
# PREPARE DMA TABLE IN MEMORY AT 0x200000 FOR USER
MOV $0x200000,DP
MOV DP,0x200000
MOV $0x000000,DP
MOV DP,0x200004
MOV $0x01,AL
MOV AL,0x200003
# TELL DMA DEVICE WHERE TO GET PARAMS WITH 3 OUTS FROM LEAST TO MOST SIG. BYTE
MOV $0x00,AL
OUT AL,0x00F7
OUT AL,0x00F7
MOV $0x20,AL
OUT AL,0x00F7
# BUILD COMMAND IN AH
# READ - DMA - NO INT 0X04
MOV $0x04,AL
OUT AL,0x00F4
# CHECK FOR FINISH BIT IN RSR REGISTER
IN 0x00F6,AL
# AND WITH FINISH BIT
CMP $0x01,AL
JNE 0xff0071
# NOW IT HAS FINISHED
JMP 0x00