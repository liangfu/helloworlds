
#include <stdio.h>
#include <stdlib.h>

class Foo
{
public:
  int add(int i, int j)
  {
	return i+j;
  }
  int mult(int i, int j)
  {
	return i*j;
  }
  static int negate(int i)
  {
	return -i;
  }
};

int bar1(int i, int j, Foo* pFoo, int(Foo::*pfn)(int,int))
{
  return (pFoo->*pfn)(i,j);
}

typedef int(Foo::*Foo_pfn)(int,int);

int bar2(int i, int j, Foo* pFoo, Foo_pfn pfn)
{
  return (pFoo->*pfn)(i,j);
}

typedef int(*PFN)(int);

int bar3(int i, PFN pfn)
{
  return pfn(i);
}

int main()
{
  Foo foo;
  fprintf(stderr, "Foo::add(2,4) = %d\n" ,bar1(2,4, &foo, &Foo::add));
  fprintf(stderr, "Foo::mult(3,5) = %d\n", bar2(3,5, &foo, &Foo::mult));
  fprintf(stderr, "Foo::negate(6) = %d\n" , bar3(6, &Foo::negate));
  return 0;
}



