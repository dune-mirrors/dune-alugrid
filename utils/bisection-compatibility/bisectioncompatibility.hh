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
  const bool stevensonRefinement_ = true ;
  const int bisectionType = 1;
  const int type1node = stevensonRefinement_ ? 1 : 3;
  const int type1face = numFaces - type1node - 1;

  //constructor taking elements
  BisectionCompatibility(std::vector<ElementType >  elements)
    : elements_(elements), maxVertexIndex_(0) {
    buildNeighbors();
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
    EdgeType edge;
    std::cout << "[" << el[0] ;
    for(int i=1; i < numFaces; ++i)
      std::cout << ","<< el[i] ;
    std::cout << "]  Refinement Edges: ";
    for(int i=0; i< numFaces; ++i)
    {
      getRefinementEdge(el, i, edge, bisectionType);
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
        fixNode(el, priorityNode);
      else if(freeFace > -1)
      {
        nodePriority[el[freeFace]] = currNodePriority;
        fixNode(el, freeFace);
        --currNodePriority;
      }
      else //fix a random node
      {
        nodePriority[el[0]] = currNodePriority;
        fixNode(el, 0);
        --currNodePriority;
      }

      FaceElementType faceElement;
      //walk over all faces
      //add face 2 to freeFaces - if already free -great, match and keep, if active and not free, match and erase
      getFace(el,type1face,faceElement);
      getFace(el,type1face, face);
      if(freeFaces.find(face) != freeFaces.end())
      {
        while(!checkFaceCompatibility(faceElement))
        {
          rotate(el);
        }
        freeFaces.find(face)->second[1] = elIndex;
      }
      else if(activeFaces.find(face) != activeFaces.end())
      {
        while(!checkFaceCompatibility(faceElement))
        {
          rotate(el);
        }
        activeFaces.erase(face);
      }
      else
      {
        freeFaces.insert({{face,{elIndex,elIndex}}});
      }
      //add others to activeFaces - if already there, delete, if already free, match and erase
      for(auto&& i : {0,1,2,3})
      {
        if (i == type1face) continue;
        getFace(el,i,face);
        getFace(el,i,faceElement);
        int neighborIndex = faceElement.second[0] == elIndex ? faceElement.second[1] : faceElement.second[0];
        if(freeFaces.find(face) != freeFaces.end())
          while(!checkFaceCompatibility(faceElement))
          {
            rotate(elements_[neighborIndex]);
          }
        else if(activeFaces.find(face) != activeFaces.end())
        {
          if(!checkFaceCompatibility(faceElement))
          {
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

private:

  //check face for compatibility
  bool checkFaceCompatibility(std::pair<FaceType, EdgeType> face)
  {
    EdgeType edge1,edge2;
    if(face.second[0] != face.second[1])
    {
      getRefinementEdge(elements_[face.second[0]], face.first, edge1, bisectionType);
      getRefinementEdge(elements_[face.second[1]], face.first, edge2, bisectionType);
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

  void fixNode(ElementType& el, int node)
  {
    if(!(node == type1node))
    {
      //swap the node at the right position
      std::swap(el[node],el[type1node]);
      //also swap to other nodes to keep the volume positive
      //2 and 0 are never type1node
      std::swap(el[0],el[2]);
    }
  }

  //The rotations that keep the type 1 node fixed
  void rotate(ElementType& el)
  {
    if(stevensonRefinement_)
    {
      std::swap(el[0],el[2]);
      std::swap(el[0],el[3]);
    }
    //ALBERTA refinement
    else
    {
      std::swap(el[0],el[2]);
      std::swap(el[0],el[1]);
    }
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
  void getRefinementEdge(ElementType el, int faceIndex, EdgeType & edge, int type )
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
          edge = {el[0],el[2]};
          break;
        case 3 :
          edge =  {el[1],el[3]};
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
          edge =  {el[0],el[2]};
          break;
        case 3 :
          edge =  {el[1],el[3]};
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
          edge = {el[0],el[2]};
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
          edge = {el[0],el[2]};
          break;
        case 3 :
          edge = {el[1],el[2]};
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
  void getRefinementEdge(ElementType el, FaceType face, EdgeType & edge, int type )
  {
    getRefinementEdge(el, getFaceIndex(el, face), edge, type);
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
  void buildNeighbors()
  {
    if(!neighbours_.empty())
      neighbours_.clear();
    FaceType face;
    EdgeType indexPair;

    unsigned int index = 0;
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
      for(int i=0; i < numFaces ; ++i)
        maxVertexIndex_ = std::max(maxVertexIndex_, el[i]);
    }
  }

private:
  //the elements to be renumbered
  std::vector<ElementType> elements_;
  //the neighbouring structure
  FaceMapType neighbours_;
  //The maximum Vertex Index
  unsigned int maxVertexIndex_;

}; //class bisectioncompatibility



#endif //ALUGRID_COMPATIBILITY_CHECK_HH_INCLUDED
