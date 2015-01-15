#include <iostream>
#include <map>
#include <vector>

using namespace std;

class Base
{

  virtual int foo () { return 0; };
};

class A : public Base
{
  double bla;
  double & a;

public:
  A () : a(bla) {}

};

class B
{
  map < int , int , less<int> > m;
};

int main(int argc, char ** argv)
{
  std::cout << sizeof(A) << "\n";
  std::cout << sizeof(B) << "\n";
  return 0;
}
