#ifndef ALUGRID_COMPATIBILITY_CHECK_HH_INCLUDED
#define ALUGRID_COMPATIBILITY_CHECK_HH_INCLUDED

#include <iostream>
#include <array>
#include <map>
#include <vector>
#include <algorithm>
#include <assert.h>

//#include <dune/grid/io/file/dgfparser/parser.hh>
//#include <dune/alugrid/common/alugrid_assert.hh>
//#include <dune/alugrid/common/declaration.hh>



//Class to correct the element orientation to make bisection work in 3d
class BisectionCompatibility
{
public:
  typedef std::array<unsigned int, 3> FaceType;
  typedef std::vector< unsigned int > ElementType;
  typedef std::array<unsigned int, 2> EdgeType;
  typedef std::map< FaceType, EdgeType > FaceMapType;
  typedef std::pair< FaceType, EdgeType > FaceElementType;


  const int numFaces = 4;
  const int numRotations = 6;
  const bool stevensonRefinement_ = true ;
  const int bisectionType = 1;

  //constructor taking elements
  BisectionCompatibility(std::vector<ElementType >  elements)
    : origElements_(elements), debug_({0}), dbgCounter_(0), maxVertexIndex_(0) {
    buildNeighbors(true); reOrder(); buildElements(); buildOrientations(); buildNeighbors(false);
  };


  //check grid for compatibility
  bool compatibilityCheck ()
  {
    for(auto&& face : neighbours_)
    {
      if(!checkFaceCompatibility(face)) return false;
    }
    return true;
  }

  //check a subset of the elements for compatibility
  bool compatibilityCheck (unsigned int index)
  {
    for(auto&& face : neighbours_)
    {
      if(face.second[1] >= index) continue;
      if(!checkFaceCompatibility(face)) return false;
    }
    return true;
  }

  //print the neighbouring structure
  void printNB()
  {
    for(auto&& face : neighbours_)
    {
      std::cout << face.first[0] << "," << face.first[1] << "," << face.first[2] << " : " << face.second[0] << ", " << face.second[1] << std::endl;
    }
  }

  //print an element with orientation and al refinement edges
  void printElement(int index)
  {
    ElementType el = elements_[index];
    bool orientation =orientations_[index];
    EdgeType edge;
    std::cout << "[" << el[0] ;
    for(int i=1; i < numFaces; ++i)
      std::cout << ","<< el[i] ;
    std::cout << "]  orientation: " << orientation << " Refinement Edges: ";
    for(int i=0; i< numFaces; ++i)
    {
      getRefinementEdge(el, i, edge, orientation, bisectionType);
      std::cout << "[" << edge[0] << "," << edge[1] << "] ";
    }
    std::cout << std::endl;
  }

  bool makeCompatible()
  {
    //the currently active Faces.
    //and the free faces that can still be adjusted at the end.
    FaceMapType activeFaces, freeFaces;
    //the finished elements. The number indicates the fixed node
    //if it is -1, the element has not been touched yet.
    std::vector<int> nodePriority;
    nodePriority.resize(maxVertexIndex_, -1);
    int currNodePriority =maxVertexIndex_;


    //walk over all elements
    for(int elIndex =0 ; elIndex < elements_.size(); ++elIndex)
    {
      //if no node is constrained and no face is active, fix one (e.g. smallest index)
      ElementType & el = elements_[elIndex];
      int priorityNode = -1;
      FaceType face;
      int freeFace = -1;
      for(int i = 0; i < 4; ++i)
      {
        if(nodePriority[el[i]] > 0)
        {
          if(priorityNode < 0)
            priorityNode = i;
          else if(nodePriority[el[i]] > nodePriority[el[priorityNode]])
            priorityNode = i;
        }
        getFace(el,i,face );
        if(freeFaces.find(face) != freeFaces.end())
          freeFace = i;
      }
      if(priorityNode > -1)
        elements_[elIndex] = fixNode(el, priorityNode, true);
      else if(freeFace > -1)
      {
        nodePriority[el[freeFace]] = currNodePriority;
        elements_[elIndex] = fixNode(el, freeFace, true);
        --currNodePriority;
      }
      else //fix a random node
      {
        nodePriority[el[0]] = currNodePriority;
        elements_[elIndex] = fixNode(el, 0, true);
        --currNodePriority;
      }

      FaceElementType faceElement;
      //walk over all faces
      //add face 2 to freeFaces - if already free -great, match and keep, if active and not free, match and erase
      getFace(el,2,faceElement);
      getFace(el,2, face);
      if(freeFaces.find(face) != freeFaces.end())
      {
        while(!checkFaceCompatibility(faceElement))
        {
          elements_[elIndex] = matchingRotate(elements_[elIndex]);
        }
        freeFaces.find(face)->second[1] = elIndex;
      }
      else if(activeFaces.find(face) != activeFaces.end())
      {
        while(!checkFaceCompatibility(faceElement))
        {
          elements_[elIndex] = matchingRotate(elements_[elIndex]);
        }
        activeFaces.erase(face);
      }
      else
      {
        freeFaces.insert({{face,{elIndex,elIndex}}});
      }
      //add others to activeFaces - if already there, delete, if already free, match and erase
      for(auto&& i : {0,1,3})
      {
        getFace(el,i,face);
        getFace(el,i,faceElement);
        int neighborIndex = faceElement.second[0] == elIndex ? faceElement.second[1] : faceElement.second[0];
        if(freeFaces.find(face) != freeFaces.end())
          while(!checkFaceCompatibility(faceElement))
          {
            std::cout << i <<  "active free" ;
            elements_[neighborIndex] = matchingRotate(elements_[neighborIndex]);
          }
        else if(activeFaces.find(face) != activeFaces.end())
        {
          if(!checkFaceCompatibility(faceElement))
          {
            std::cout << "FAILED" << std::endl << std::endl;
            for(int k =0 ; k < nodePriority.size() ; ++k)
              std::cout << "[" << k << ", " << nodePriority[k] << "]" << std::endl;
            return false;
          }
          activeFaces.erase(face);
        }
        else
        {
          activeFaces.insert({{face,{elIndex,elIndex}}});
        }
      }
    }
    //now postprocessing of freeFaces. possibly - not really necessary, has to be thought about.
    for(auto&& face : freeFaces)
    {
      int elementIndex = face.second[0];
      int neighborIndex = face.second[1];
      //give refinement edge positive priority
      //and non-refinement edge negative priority less than -1 and counting
    }
  }

  //make grid compatible
  //easiest algorithm :
  //try all positive rotations of all elements
  //and check each time for compatibility
  //results in 6^n effort. (stop at first possibility)
  bool makeCompatibleRecursively(unsigned int index)
  {
    if(index < origElements_.size())
    {
      debugOutput(index);
      for(int i = 0; i<numRotations ; ++i)
      {
        elements_[index] = rotate(origElements_[ordering_[index]], i);
        if(compatibilityCheck(index))
        {
          if( makeCompatibleRecursively(index+1) )
            return true;
        }
        //if this did not work, first try switching the orientation
        orientations_[index] = !orientations_[index];
        if(compatibilityCheck(index))
        {
          if( makeCompatibleRecursively(index+1) )
            return true;
        }
      }
    }
    else if( compatibilityCheck() )
    {
      std::cout << std::endl << std::endl << "Iterations: " << dbgCounter_ << std::endl ;
      return true;
    }
    return false;
  }

  //return elements in their original ordering
  void returnElements(std::vector<ElementType> & elements, std::vector<bool> & orientations)
  {
    for(unsigned int i = 0; i < elements_.size(); ++i)
    {
      elements[ordering_[i]] = elements_[i];
      orientations[ordering_[i]] =orientations_[i];
    }
  }

  //get the orientation of an element
  bool orientation(unsigned int index)
  {
    return orientations_[index];
  }

private:

  //check face for compatibility
  bool checkFaceCompatibility(std::pair<FaceType, EdgeType> face)
  {
    EdgeType edge1,edge2;
    if(face.second[0] != face.second[1])
    {
      getRefinementEdge(elements_[face.second[0]], face.first, edge1, orientations_[face.second[0]], bisectionType);
      getRefinementEdge(elements_[face.second[1]], face.first, edge2, orientations_[face.second[1]], bisectionType);
      if(edge1 != edge2)
      {
        /*   std::cerr << "Face: " << face.first[0] << ", " << face.first[1] << ", " << face.first[2]
           << " has refinement edge: " << edge1[0] << ", " << edge1[1] <<
           " from one side and: " << edge2[0] << ", " << edge2[1] <<
           " from the other." << std::endl;        */
        return false;
      }
    }
    return true;
  }


  ElementType matchingRotate(ElementType el)
  {
    if(stevensonRefinement_)
      return rotate(el, 7);
    else
      return rotate(el,4);
  }


  ElementType fixNode(ElementType el, int node, bool orientation)
  {
    if(stevensonRefinement_)
    {
      std::swap(el[node],el[1]);
    }
    else //ALBERTA
    {
      if(orientation)
        std::swap(el[node],el[3]);
      else
        std::swap(el[node],el[2]);
    }
    return el;
  }

  //all positive permutations of the elements
  //we need all positive rotations
  ElementType rotate(ElementType el, int rotation)
  {
    if(stevensonRefinement_)
    {
      switch(rotation)
      {
      case 0 :
        return el;
      //rotate face 1,2,3
      case 1 :
        return {el[0], el[3], el[1], el[2]};
      //rotate face 1,2,3
      case 2 :
        return {el[0], el[2], el[3], el[1]};
      //rotate face 0,1,2
      case 3 :
        return {el[2], el[0], el[1], el[3]};
      //rotate face 0,1,2
      case 4 :
        return {el[1], el[2], el[0], el[3]};
      //flip 0,1 and 2,3
      case 5 :
        return {el[1], el[0], el[3], el[2]};
      //reverse
      case 6 :
        return {el[3], el[2], el[1], el[0]};
      //reversion of 1
      case 7 :
        return {el[2], el[1], el[3], el[0]};
      //reversion of 2
      case 8 :
        return {el[1], el[3], el[2], el[0]};
      //reversion of 3
      case 9 :
        return {el[3], el[1], el[0], el[2]};
      //reversion of 4
      case 10 :
        return {el[3], el[0], el[2], el[1]};
      //reversion of 5
      case 11 :
        return {el[2], el[3], el[0], el[1]};
      default :
        return el;
      }
    }
    //ALBERTA refinement
    else
    {
      switch(rotation)
      {
      case 0 :
        return el;
      //reverse
      case 1 :
        return {el[3], el[2], el[1], el[0]};
      //rotate face 1,2,3
      case 2 :
        return {el[0], el[3], el[1], el[2]};
      //rotate face 1,2,3
      case 3 :
        return {el[0], el[2], el[3], el[1]};
      //rotate face 0,1,2
      case 4 :
        return {el[1], el[2], el[0], el[3]};
      //reversion of 3
      case 5 :
        return {el[1], el[3], el[2], el[0]};
      default :
        return el;
      }
    }
    return el;
  }

  //get the sorted face of an element to a corresponding index
  void getFace(ElementType el, int faceIndex, FaceType & face )
  {
    switch(faceIndex)
    {
    case 0 :
      face = {el[0],el[1],el[2]};
      break;
    case 1 :
      face = {el[0],el[1],el[3]};
      break;
    case 2 :
      face = {el[0],el[2],el[3]};
      break;
    case 3 :
      face = {el[1],el[2],el[3]};
      break;
    default :
      std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR TETRAHEDRONS" << std::endl;
      abort();
    }
    std::sort(face.begin(), face.end());
    return;
  }

  void getFace(ElementType el, int faceIndex, FaceElementType & faceElement)
  {
    FaceType face;
    getFace(el, faceIndex, face);
    faceElement = *neighbours_.find(face);
  }

  //get the sorted initial refinement edge of a face of an element
  //this has to be adjusted, when using stevensonRefinement
  //orientation switches indices 2 and 3 in the internal ordering
  void getRefinementEdge(ElementType el, int faceIndex, EdgeType & edge, bool orientation, int type )
  {
    if(type == 0)
    {
      if(stevensonRefinement_)
      {
        switch(faceIndex)
        {
        case 0 :
          edge = {el[0],el[2]};
          break;
        case 1 :
          edge = {el[0],el[3]};
          break;
        case 2 :
          if(orientation)
            edge = {el[0],el[2]};
          else
            edge =  {el[0],el[3]};
          break;
        case 3 :
          if(orientation)
            edge =  {el[1],el[3]};
          else
            edge = {el[1],el[2]};
          break;
        default :
          std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR TETRAHEDRONS" << std::endl;
          abort();
        }
      }
      else //ALBERTA Refinement
      {
        switch(faceIndex)
        {
        case 0 :
          edge = {el[0],el[1]};
          break;
        case 1 :
          edge = {el[0],el[1]};
          break;
        case 2 :
          if(orientation)
            edge =  {el[0],el[2]};
          else
            edge = {el[0],el[3]};
          break;
        case 3 :
          if(orientation)
            edge =  {el[1],el[3]};
          else
            edge =  {el[1],el[2]};
          break;
        default :
          std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR TETRAHEDRONS" << std::endl;
          abort();
        }
      }
    }
    else if(type == 1)
    {
      if(stevensonRefinement_)
      {
        switch(faceIndex)
        {
        case 0 :
          edge = {el[0],el[2]};
          break;
        case 1 :
          edge = {el[0],el[3]};
          break;
        case 2 :
          if(orientation)
            edge = {el[0],el[2]};
          else
            edge =  {el[0],el[3]};
          break;
        case 3 :
          edge =  {el[2],el[3]};
          break;
        default :
          std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR TETRAHEDRONS" << std::endl;
          abort();
        }
      }
      else //ALBERTA Refinement
      {
        switch(faceIndex)
        {
        case 0 :
          edge = {el[0],el[1]};
          break;
        case 1 :
          edge = {el[0],el[1]};
          break;
        case 2 :
          if(orientation)
            edge = {el[0],el[2]};
          else
            edge = {el[0],el[3]};
          break;
        case 3 :
          if(orientation)
            edge = {el[1],el[2]};
          else
            edge = {el[1],el[3]};
          break;
        default :
          std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR TETRAHEDRONS" << std::endl;
          abort();
        }
      }
    }
    else
    {
      std::cerr << "no other types than 0 and 1 implemented." << std::endl;
      abort();
    }
    std::sort(edge.begin(),edge.end());
    return;
  }

  //get the sorted initial refinement edge of a face of an element
  void getRefinementEdge(ElementType el, FaceType face, EdgeType & edge, bool orientation, int type )
  {
    getRefinementEdge(el, getFaceIndex(el, face), edge, orientation, type);
    return;
  }

  //get the index of a face in an elements
  //this could be improved by exploiting that faces are sorted
  int getFaceIndex(ElementType el, FaceType face)
  {
    for(int i =0; i<numFaces ; ++i)
    {
      if(!( el[i] == face[0] || el[i] == face[1]  || el[i] == face[2] ) )
        return numFaces - i - 1 ;
    }
    return -1;
  }

  //build the structure containing the neighbors
  //consists of a face and the two indices belonging to
  //the elements that share said face
  //boundary faces have two times the same index
  //this is executed in the constructor
  void buildNeighbors(bool original)
  {
    if(!neighbours_.empty())
      neighbours_.clear();
    FaceType face;
    EdgeType indexPair;

    unsigned int index = 0;
    if(!original)
    {
      std::cout << "buildNeighbors false" << std::endl;
      for(auto&& el : elements_)
      {
        for(int i = 0; i< numFaces; ++i)
        {
          getFace(el, i, face);
          auto faceInList = neighbours_.find(face);
          if(faceInList == neighbours_.end())
          {
            indexPair = {index, index};
            neighbours_.insert(std::make_pair (face, indexPair ) );
          }
          else
          {
            faceInList = neighbours_.find(face);
            assert(faceInList != neighbours_.end());
            faceInList->second[1] = index;
          }
        }
        ++index;
      }
    }
    else
    {
      std::cout << "buildNeighbors true" << std::endl;
      for(auto&& el : origElements_)
      {
        for(int i = 0; i< numFaces; ++i)
        {
          getFace(el, i, face);
          auto faceInList = neighbours_.find(face);
          if(faceInList == neighbours_.end())
          {
            indexPair = {index, index};
            neighbours_.insert(std::make_pair (face, indexPair ) );
          }
          else
          {
            faceInList = neighbours_.find(face);
            assert(faceInList != neighbours_.end());
            faceInList->second[1] = index;
          }
        }
        ++index;
      }
    }
  }

  //create orientations
  //to be filled into insertUniquetetra
  void buildOrientations()
  {
    std::cout << "buildOrientations"  <<std ::endl;
    orientations_.resize(origElements_.size());
    for(unsigned int i = 0; i<orientations_.size(); ++i)
      // orientations_[i] = ordering_[i]%2;
      orientations_[i] = 0;
  }

  //sort elements by ordering
  void buildElements()
  {
    std::cout << "buildElements" << std::endl;
    elements_.resize(origElements_.size());
    for(unsigned int i = 0; i< origElements_.size(); ++i)
      elements_ [i] = origElements_[ordering_[i]];

    for(auto&& el : elements_)
      for(int i=0; i < numFaces ; ++i)
        maxVertexIndex_ = std::max(maxVertexIndex_, el[i]);
  }

  //create an ordering such that elements are
  //always connected
  void reOrder()
  {
    std::cout << "reOrder" << std::endl;
    ordering_.resize(origElements_.size(),0);
    ordering_[0] = 0;
    FaceType face;
    for(unsigned int i =1; i<origElements_.size(); ++i)
    {
      //go through all faces up to i
      for(unsigned int j =0; j < i; ++j)
      {
        for(int faceIndex =0; faceIndex< numFaces; ++faceIndex)
        {
          getFace(origElements_[ordering_[j]], faceIndex, face);
          auto nb = neighbours_.find(face);
          //if neighbour is not in ordering
          if(!inOrdering(i, nb->second[0]))
          {
            ordering_[i] = nb->second[0];
            ++i;
          }
          if(!inOrdering(i, nb->second[1]))
          {
            ordering_[i] = nb->second[1];
            ++i;
          }
        }
      }
    }
    assert(ordering_[ordering_.size()-1] != 0);
  }

  //for reOrder():
  //check if index was already inserted
  bool inOrdering(unsigned int currentMax, unsigned int index)
  {
    for(unsigned int i =0; i < currentMax; ++i)
    {
      if(ordering_[i] == index)
        return true;
    }
    return false;
  }



  //some debug output for the recursion
  void debugOutput(unsigned int index)
  {
    if(index > debug_.size ())
      debug_.resize(index,0);
    else
      debug_[index] +=1;

    if(dbgCounter_ % 1000 == 0)
    {
      std::cout << debug_.size()  << "," << minElement(debug_) << "  ";
      debug_.clear();
    }
    dbgCounter_ ++;
  }

  unsigned int minElement( std::vector<unsigned int> vec)
  {
    for(unsigned int i =0; i< vec.size(); ++i)
      if(vec[i] !=0 ) return i;

    return vec.size();
  }


private:
  //the ordering
  std::vector< unsigned int > ordering_;
  //the original elements
  std::vector<ElementType> origElements_;
  //the elements to be renumbered
  std::vector<ElementType> elements_;
  //the neighbouring structure
  FaceMapType neighbours_;
  //the element orientation
  std::vector<bool> orientations_;

  unsigned int maxVertexIndex_;

  std::vector<unsigned int> debug_;
  unsigned int dbgCounter_;



}; //class bisectioncompatibility



#endif //ALUGRID_COMPATIBILITY_CHECK_HH_INCLUDED
