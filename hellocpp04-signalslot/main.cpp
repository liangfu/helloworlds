
#include <stdio.h>
#include <stdlib.h>

struct SlotList
{
  int slot1(double d) {return d*d;}
  int slot2(double d) {return d+d;}
};


int main()
{
  typedef int (SlotList::*SignalType)(double d);
  
  SignalType signal = &SlotList::slot1;
  SlotList slots;
  
  int value = (slots.*signal)(3.14);

  fprintf(stderr,"value: %d\n",value);

  return 0;
}



