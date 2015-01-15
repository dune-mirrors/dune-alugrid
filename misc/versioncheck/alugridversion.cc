#include <config.h>
#include <cstdlib>
#include <iostream>
#include <cstring>

int main(int argc, char ** argv)
{
  if(argc >= 2)
  {
    std::string argument (argv[1]);
    if( argument == "-v" )
    {
      std::cout << PACKAGE_VERSION << std::endl;
      return 0;
    }

    if( argument == "-c" && argc >= 3 )
    {
      // check wether given version is bigger than actual
      // package version
      int result = std :: strcmp(PACKAGE_VERSION,argv[2]);
      std::cout << result << std::endl;
      return result;
    }
  }

  std::cerr << "usage: " << argv[0] << std::endl;
  std::cerr << "  -c <version number to check>  compares version numbers" << std::endl;
  std::cerr << "     result is -1 if version number is smaller, 0 if equal, and 1 if larger! \n" << std::endl;
  std::cerr << "  -v print version number of package " << std::endl;
  exit(1);

  return 0;
}
