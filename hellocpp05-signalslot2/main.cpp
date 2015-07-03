
#include <stdio.h>
#include <stdlib.h>

struct Object
{
  Object(Object * parent=0){}
};

struct SlotClass : public Object
{
  SlotClass():Object(){}
  void slot1(int val) {fprintf(stderr,"value: %d\n", val*val);}
};

typedef void (SlotClass::*SignalFunction)(int);

struct SignalClass : public Object
{
  SlotClass * m_slots;
  SignalClass():Object(),m_slots(0){}
  void (SlotClass::*signal1)(int);
  void trigger(){
	if (!this->m_slots){fprintf(stderr,"m_slots not connected!");return;}
	((*(this->m_slots)).*signal1)(3);
  }
};

static void connect(SignalClass * signals, SignalFunction & signal, 
					SlotClass * slots,     SignalFunction slot)
{
  signals->m_slots=slots;
  signal = slot;
}

int main()
{
  SlotClass * slots = new SlotClass();
  SignalClass * signals = new SignalClass();
  connect(signals,signals->signal1,slots,&SlotClass::slot1);
  signals->trigger();
  return 0;
}



