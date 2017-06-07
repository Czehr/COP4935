// Input example of use of "volatile" type modifier
volatile int a;
volatile int *b;

void foo()
{
  for (volatile int y = 0; y < 10; y++) {
  }
}
