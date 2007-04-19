#include <config.h>
#include <iostream>

int main(int argc, char ** argv)
{
  if(argc < 2) 
  {
    std::cerr << "usage: " << argv[0] << " <version number to check> " << std::endl;  
    std::cerr << "result is -1 if version number is smaller, 0 if equal, and 1 if larger! \n";
    exit(1);
  }
 
  // check wether given version is bigger than actual 
  // package version 
  int result = strcmp(argv[1],PACKAGE_VERSION); 
  std::cout << result; 
  return result;
}
