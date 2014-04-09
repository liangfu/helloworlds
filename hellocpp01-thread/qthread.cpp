
#include "qthread.h"

class Test : public Thread
{
  char m_str[1024];
public:
  Test(){sprintf(m_str,".");}
  void run(){
	while (!this->m_quitFlag){
	  fprintf(stderr,"%s",m_str);
	}
  }
};

int main(int argc, char * argv[])
{
  int i;
  Test * t=new Test();
  for (i=0;i<40;i++)
  {
	t->start();
	Test::sleep(1);
	t->quit();
	fprintf(stderr,"|");
  }
  delete t;
  return 0;
}
