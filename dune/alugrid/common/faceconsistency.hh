#ifndef ALUGRID_FACE_CONSISTENCY_HH_INCLUDED
#define ALUGRID_FACE_CONSISTENCY_HH_INCLUDED

#include <iostream>
#include <array>
#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <random>

#include <dune/common/timer.hh>


//Class to correct the element orientation to make twist free 3d cubes work
//we expect an edge-consistent grid
template< class InteriorFaceMap, class BoundaryFaceMap>
class FaceConsistency
{
public:

  typedef std::array<unsigned int, 2> NeighbourType;
  typedef typename InteriorFaceMap::key_type FaceType;
  typedef std::vector<unsigned int> ElementType;
  typedef std::pair<FaceType, std::pair<unsigned int, unsigned int> > FaceNeighbourType;

protected:
  //the elements to be renumbered
  std::vector<ElementType> elements_;
  //the neighbouring structure
  InteriorFaceMap neighbours_;
  //
  BoundaryFaceMap boundaryFaces_;
  //
  unsigned int numNonPerEl_;

public:
  //constructor taking elements
  FaceConsistency( const std::vector<ElementType>& elements, const InteriorFaceMap & interiorFaces, const BoundaryFaceMap & boundaryFaces, const unsigned numNonPeriodicElem)
    : elements_( elements ), neighbours_(interiorFaces), boundaryFaces_(boundaryFaces), numNonPerEl_(numNonPeriodicElem)
  {
    //build the information about neighbours
    Dune::Timer timer;
    buildNeighbors();
#ifndef NDEBUG
    std::cout << "Build neighbors took " << timer.elapsed() << " sec." << std::endl;
#endif
  }

  //check grid for compatibility
  //i.e. walk over all faces and check their compatibility.
  bool consistencyCheck (bool verbose = false)
  {
    for(auto&& faceNeighbour : neighbours_)
    {
      if(!checkFaceConsistency(faceNeighbour, verbose)) return false;
    }
    return true;
  }

  //print the neighbouring structure
  void printNB()
  {
    for(auto&& face : neighbours_)
    {
      std::cout << face.first[0] << "," << face.first[1] << "," << face.first[2] << "," << face.first[3] << " : " << face.second[0] << ", " << face.second[1] << std::endl;
    }
  }

  //print an element with orientation and all refinement edges
  void printElement(int index)
  {
    ElementType el = elements_[index];
    std::cout << "[" << el[0] ;
    for(int i=1; i < 8; ++i)
      std::cout << ","<< el[i] ;
    std::cout <<  "]" <<  std::endl;
  }

  //For an edge consistent grid, face consistency is always possible
  void makeFaceConsistent()
  {
    InteriorFaceMap untreatedFaces(neighbours_);

    //Repeat until all faces have been treated
    while(!untreatedFaces.empty())
    {
      auto startFace = untreatedFaces.begin();
      unsigned int startElIndex = startFace->second.first;
      ElementType el = elements_[startElIndex];
      int startFaceIndexInEl = getFaceIndex(el, startFace->first);
      int oppIndex = (startFaceIndexInEl % 2) ? startFaceIndexInEl + 1 : startFaceIndexInEl - 1;
      FaceType face = getFace(el, startFaceIndexInEl);
      FaceType oppFace = getFace(el, oppIndex);
      walkIntoDirection( face, startElIndex, untreatedFaces);
      walkIntoDirection( oppFace, startElIndex, untreatedFaces);
    }
  }

  //gets a face and an element index to start from
  //walks and reorients until end of direction
  //removes all treated faces from untreatedFaces
  //
  //Recursive implementation
  //
  // - check termination conditions
  // - getFaceIndex
  // - checkFaceOrientation
  // - swapelement if necessary
  // - remove face from untreated
  // - get opposite unsorted face
  // - move on
  void walkIntoDirection(FaceType unsortedFace, unsigned int previousElIndex, InteriorFaceMap & untreatedFaces)
  {
    FaceType currentFaceKey = unsortedFace;
    std::sort(currentFaceKey.begin(), currentFaceKey.end() );

    auto currentFace = untreatedFaces.find( currentFaceKey );
    if( currentFace == untreatedFaces.end() )
      return;
    unsigned int  currentElIndex = (previousElIndex == currentFace->second.first) ? currentFace->second.second : currentFace->second.first;

    ElementType el = elements_[currentElIndex];
    int indexInEl = getFaceIndex( el, unsortedFace );
    FaceType faceInEl = getFace( el, indexInEl );

    if( faceInEl[1] != unsortedFace[1] || faceInEl[2] != unsortedFace[2] )
    {
      alugrid_assert( faceInEl[0] == unsortedFace[0] );
      alugrid_assert( faceInEl[3] == unsortedFace[3] );
      el = swapElementOrientation( currentElIndex, indexInEl );
    }
    untreatedFaces.erase( currentFaceKey );

    int oppIndex = (indexInEl %2 == 0) ? indexInEl + 1 : indexInEl - 1;
    FaceType oppositeFace = getFace( el, oppIndex );
    walkIntoDirection(oppositeFace, currentElIndex, untreatedFaces);
  }



  void returnElements(std::vector<ElementType> & elements)
  {
    elements = elements_;
  }


private:

  FaceType convert( std::vector<unsigned> face)
  {
    FaceType newFace;
    for(unsigned i = 0; i< std::tuple_size<FaceType>::value; ++i)
    {
      newFace[i] = face[i];
    }
    return newFace;
  }

  ElementType swapElementOrientation(const unsigned int elIndex, const unsigned int localFaceIndex )
  {
    ElementType & el = elements_[elIndex];
    switch (localFaceIndex){
      case 0:
      case 1:
        std::swap(el[2],el[4]);
        std::swap(el[3],el[5]);
        break;
      case 2:
      case 3:
        std::swap(el[1],el[4]);
        std::swap(el[3],el[6]);
        break;
      case 4:
      case 5:
        std::swap(el[1],el[2]);
        std::swap(el[5],el[6]);
        break;
    }
    return el;
  }


  //check face for compatibility
  bool checkFaceConsistency(FaceNeighbourType faceNeighbour, bool verbose = false)
  {
    FaceType face = faceNeighbour.first;
    ElementType el0 = elements_[faceNeighbour.second.first];
    ElementType el1 = elements_[faceNeighbour.second.second];

    int indexInEl0 = getFaceIndex(el0, face);
    int indexInEl1 = getFaceIndex(el1, face);
    FaceType faceInEl0 = getFace(el0, indexInEl0);
    FaceType faceInEl1 = getFace(el1, indexInEl1);
    for(int i = 0 ; i < 4; ++i)
    {
      if(faceInEl0[i] != faceInEl1[i])
      {
        if( verbose )
        {
          std::cout << "Face Orientation does not match for elements: " ;
          printElement(faceNeighbour.second.first);
          printElement(faceNeighbour.second.second);
        }
        return false;
      }
    }
    return true;
  }


  //get the face of an element to a corresponding index
  //the index coincides with the missing vertex
  FaceType getFace(ElementType el, int faceIndex)
  {
    ElementType face;
    switch(faceIndex)
    {
    case 0 :
      face = {el[0],el[2],el[4],el[6]};
      break;
    case 1 :
      face = {el[1],el[3],el[5],el[7]};
      break;
    case 2 :
      face = {el[0],el[1],el[4],el[5]};
      break;
    case 3 :
      face = {el[2],el[3],el[6],el[7]};
      break;
    case 4 :
      face = {el[0],el[1],el[2],el[3]};
      break;
    case 5 :
      face = {el[4],el[5],el[6],el[7]};
      break;
    default :
      std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR HEXAHEDRONS" << std::endl;
      std::abort();
    }
    return convert(face);
  }

  //get FaceIndex for a (sorted) face
  int getFaceIndex(const ElementType & el, const FaceType & face)
  {
    FaceType sortedFace = face;
    std::sort(sortedFace.begin(),sortedFace.end());

    FaceType compareFace;
    for(int i = 0; i < 6; ++i)
    {
      compareFace = getFace(el, i);
      std::sort(compareFace.begin(), compareFace.end());
      if(compareFace[0] != sortedFace[0])
        continue;
      if(compareFace[1] != sortedFace[1])
        continue;
      if(compareFace[2] != sortedFace[2])
        continue;
      assert(compareFace[3] == sortedFace[3]);
      return i;
    }
    return -1;
  }

  //add periodicneighbours to the interiorface structure passed from the grid
  void buildNeighbors()
  {
    //start at first periodic element
    for(unsigned int i = numNonPerEl_ ; i < elements_.size(); ++i)
    {
      ElementType el = elements_[i];
      ElementType vecFace = {el[0],el[1],el[2],el[3]};
      FaceType face0 = convert(vecFace);
      std::sort(face0.begin(), face0.end());
      auto it = boundaryFaces_.find(face0);
      neighbours_.insert(std::make_pair(face0, std::make_pair(i, it->second)));

      FaceType face1 = convert(ElementType{{el[4],el[5],el[6],el[7]}});
      std::sort(face1.begin(), face1.end());
      it = boundaryFaces_.find(face1);
      neighbours_.insert(std::make_pair(face1, std::make_pair(i, it->second)));
    }
  }


}; //class FaceConsistency



#endif //ALUGRID_FACE_CONSISTENCY_HH_INCLUDED
