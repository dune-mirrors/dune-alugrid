//***********************************************************************
//
//  Example program how to use ALUGrid.
//  Author: Robert Kloefkorn
//
//  This little program read one of the macrogrids and generates a grid.
//  The  grid is refined and coarsend again.
//
//***********************************************************************
#include <iostream>
using namespace std;

// include serial part of ALUGrid

int evalVertexTwist(int twist, int vertex)
{
 return (twist < 0 ?
          (9 - vertex + twist) % 4 :
          (vertex + twist) % 4);
}

int evalEdgeTwist(int twist, int edge)
{
 return (twist < 0 ?
              (8 - edge + twist) % 4 :
              (edge + twist) % 4);
}

int vertex2Face (int i)
{
  return (i < 4) ? (4 - i) % 4 : i - 4 ;
}

int main()
{
  std::cout << "Hexa Vertex Twists\n";
  for(int tw=0; tw <8; ++tw)
  {
    std::cout << "{";
    for(int vx=0; vx<4; ++vx)
    {
      std::cout << evalVertexTwist(tw-4,vx);
      if(vx < 3)
      {
        std::cout << ",";
      }
      else
      {
        if(tw < 7)
          std::cout << "},";
        else
          std::cout << "} ";
      }
    }
    std::cout << " // twist = " << tw-4 << "\n";
  }

  std::cout << "Hexa Edge Twist \n";
  for(int tw=0; tw <8; ++tw)
  {
    std::cout << "{";
    for(int edge=0; edge<4; ++edge)
    {
      std::cout << evalEdgeTwist(tw-4,edge);
      if(edge < 3)
      {
        std::cout << ",";
      }
      else
      {
        if(tw < 7)
          std::cout << "},";
        else
          std::cout << "} ";
      }
    }
    std::cout << " // twist = " << tw-4 << "\n";
  }

  std::cout << "Hexa Vertex 2 Face \n";
  for(int vx=0; vx<8; ++vx)
  {
    int newvx = vertex2Face(vx);
    int fvx = (vx < 4) ? 0 : 1;
    std::cout << "{" << fvx << "," << newvx;

    if(vx < 7) std::cout << "},";
    else std::cout << "} ";
    std::cout << "// vx = " << vx << "\n";
  }
  return 0;
}
