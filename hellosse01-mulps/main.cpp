#include <stdio.h>
#include <xmmintrin.h>

int main()
{
  int i;
  float a[4]={3,4,5,6};
  float b[4]={3,4,5,6};
  float v[4];
  
  __m128 _a=_mm_load_ps(a);
  __m128 _b=_mm_load_ps(b);
  __m128 _v=_mm_mul_ps(_a,_b);
  _mm_store_ps(v,_v);

  for (i=0;i<4;i++){fprintf(stderr,"%.0f,",v[i]);}
  
  return 0;
}
