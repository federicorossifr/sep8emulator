#SYS PRIMITIVE FOR PROGRAM CONTROL IN OF DUMMY DEVICE
#READ RSR
IN 0x0012,AL
#AND WITH FINISH BIT
AND $0x01,AL
JZ 0x00
#DATA IS READY
IN 0x0011,AL
IRET