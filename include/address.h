#ifndef ADDRESS_H
#define ADDRESS_H

// Page table access
void setPTE(int vp, int frame);
int  getPTE(int vp);

// Address translation
int getPhysicalAddress(int va);

// Instruction address parsing
int getAddress(char inst[]);

#endif