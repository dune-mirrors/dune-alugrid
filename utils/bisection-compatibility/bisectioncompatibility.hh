#ifndef ALUGRID_COMPATIBILITY_CHECK_HH_INCLUDED
#define ALUGRID_COMPATIBILITY_CHECK_HH_INCLUDED

#include <iostream>
#include <array>
#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <assert.h>

//#include <dune/grid/io/file/dgfparser/parser.hh>
//#include <dune/alugrid/common/alugrid_assert.hh>
//#include <dune/alugrid/common/declaration.hh>



//Class to correct the element orientation to make bisection work in 3d
// It provides different algorithms to orientate a grid.
// Also implements checks for compatibility.
class BisectionCompatibility
{
public:
  typedef std::array<unsigned int, 3> FaceType;
  typedef std::vector< unsigned int > ElementType;
  typedef std::array<unsigned int, 2> EdgeType;
  typedef std::map< FaceType, EdgeType > FaceMapType;
  typedef std::pair< FaceType, EdgeType > FaceElementType;

  //second node of the refinement edge (first is 0)
  const int type0node = stevensonRefinement_ ? 3 : 1 ;
  //The interior node of a type 1 element
  const int type1node = stevensonRefinement_ ? 1 : 2;
  //the face opposite of the interior node
  const int type1face = 3 - type1node ;

  //constructor taking elements
  //assumes standard orientation elemIndex % 2
  BisectionCompatibility(std::vector<ElementType >  elements, bool stevenson)
    : elements_(elements), elementOrientation_(elements_.size(),false), maxVertexIndex_(0), types_(elements_.size(),0), stevensonRefinement_(stevenson) {
    //build the information about neighbours
    buildNeighbors();
  };

  //check for strong compatibility
  bool stronglyCompatible ()
  {
    bool result = true;
    for(auto&& face : neighbours_)
    {
      result &= checkStrongCompatibility(face);
    }
    return result;
  }

  //check grid for compatibility
  //i.e. walk over all faces and check their compatibility.
  bool compatibilityCheck ()
  {
    for(auto&& face : neighbours_)
    {
      if(!checkFaceCompatibility(face)) return false;
    }
    return true;
  }

  bool make6CompatibilityCheck()
  {
    //set types to 0, and switch vertices 2,3 for elemIndex % 2
    applyStandardOrientation();
    bool result = compatibilityCheck();
    //set types to 0, and switch vertices 2,3 for elemIndex % 2
    applyStandardOrientation();
    return result;
  }

  //print the neighbouring structure
  void printNB()
  {
    for(auto&& face : neighbours_)
    {
      std::cout << face.first[0] << "," << face.first[1] << "," << face.first[2] << " : " << face.second[0] << ", " << face.second[1] << std::endl;
    }
  }

  //print an element with orientation and all refinement edges
  void printElement(int index)
  {
    ElementType el = elements_[index];
    EdgeType edge;
    std::cout << "[" << el[0] ;
    for(int i=1; i < 4; ++i)
      std::cout << ","<< el[i] ;
    std::cout << "]  Refinement Edges: ";
    for(int i=0; i< 4; ++i)
    {
      getRefinementEdge(el, i, edge, types_[index]);
      std::cout << "[" << edge[0] << "," << edge[1] << "] ";
    }
    std::cout << std::endl;
  }

  //an algorithm using only elements of type 0
  //it works by sorting the vertices in a global ordering
  //and tries to make as many reflected neighbors as possible.
  bool type0Algorithm()
  {
    if(!stevensonRefinement_)
    {
      BisectionCompatibility stevensonBisComp(elements_, true);
      stevensonBisComp.alberta2Stevenson();
      bool result = stevensonBisComp.type0Algorithm();
      stevensonBisComp.stevenson2Alberta();
      elementOrientation_ = stevensonBisComp.returnElements(elements_, false);
      return result;
    }
    std::list<unsigned int> vertexPriorityList;
    vertexPriorityList.clear();
    std::list<std::pair<FaceType, EdgeType> > activeFaceList; // use std::find to find
    std::vector<bool> doneElements(elements_.size(), false);

    //for now - no edge Priority, we just use the fact, that
    //each simplex know its desired refinement edge
    //std::map<EdgeType, int> edgePriorityMap;
    //buildEdgePriority (edgePriorityMap);

    ElementType el0 = elements_[0];
    //orientate E_0 (add vertices to vertexPriorityList)
    for(int vtx : el0)
      vertexPriorityList.push_back ( vtx );
    doneElements[0] =true;
    //add faces to activeFaceList, if not boundary
    //at beginning if face contains ref Edge,
    //else at the end
    FaceElementType faceElement;
    for(int i = 0; i < 4 ; ++i)
    {
      getFace(el0, i, faceElement);
      //do nothing for boundary
      if(faceElement.second[0] == faceElement.second[1])
        continue;
      auto it = std::find(activeFaceList.begin(),activeFaceList.end(), faceElement );
      //if face is not in active faces, insert
      if(it == activeFaceList.end())
      {
        //if face does not contain ref Edge
        if(i == 0 || i == type0node)
          activeFaceList.push_back(faceElement);
        else
          activeFaceList.push_front(faceElement);
      }
      else
        activeFaceList.erase(it);
    }

    //create the vertex priority List
    int numberOfElements = elements_.size();
    for(int counter = 1; counter < numberOfElements ; counter++)
    {
      //take element at first face from activeFaceList
      faceElement = *activeFaceList.begin();
      //el has been worked on
      int elIndex = faceElement.second[0];
      //neigh is to be worked on
      int neighIndex = faceElement.second[1];
      if(!doneElements[elIndex])
      {
        assert(doneElements[neighIndex]);
        std::swap(elIndex, neighIndex);
      }
      assert(!doneElements[neighIndex]);
      doneElements[neighIndex] = true;
      ElementType el = elements_[elIndex];
      ElementType & neigh = elements_[neighIndex];
      int faceInEl = getFaceIndex(el, faceElement.first);
      int faceInNeigh = getFaceIndex(neigh, faceElement.first);
      auto it = std::find(vertexPriorityList.begin(), vertexPriorityList.end(), neigh[faceInNeigh]);
      if(it == vertexPriorityList.end() )
      {
        it = std::find(vertexPriorityList.begin(), vertexPriorityList.end(), el[faceInEl]);
        //orientate element (add new vertex to vertexPriorityList)
        if(faceInEl == 0 && faceInNeigh == type0node )
        {
          it = std::find(vertexPriorityList.begin(), vertexPriorityList.end(), el[type0node]);
          it++;
        }
        else if (faceInEl == type0node && faceInNeigh == 0 )
        {
          it = std::find(vertexPriorityList.begin(), vertexPriorityList.end(), el[0]);
          it++;
        }
        vertexPriorityList.insert(it, neigh[faceInNeigh]);
      }
      auto it0 = std::find_first_of(vertexPriorityList.begin(), vertexPriorityList.end(), neigh.begin(), neigh.end());
      if(*it0 != neigh[0])
      {
        auto helpIt = std::find(neigh.begin(), neigh.end(), *it0);
        std::swap(neigh[0],*helpIt);
        elementOrientation_[neighIndex] = !(elementOrientation_[neighIndex]);
      }
      ++it0;
      auto it1 = std::find_first_of(it0,vertexPriorityList.end(), neigh.begin() + 1, neigh.end());
      if(*it1 != neigh[1])
      {
        auto helpIt = std::find(neigh.begin(), neigh.end(), *it1);
        std::swap(neigh[1],*helpIt);
        elementOrientation_[neighIndex] = !(elementOrientation_[neighIndex]);
      }
      ++it1;
      auto it2 = std::find_first_of(it1,vertexPriorityList.end(), neigh.begin() + 2, neigh.end());
      if(*it2 != neigh[2])
      {
        auto helpIt = std::find(neigh.begin(), neigh.end(), *it2);
        std::swap(neigh[2],*helpIt);
        elementOrientation_[neighIndex] = !(elementOrientation_[neighIndex]);
      }
      //add and remove faces from activeFaceList
      for(int i = 0; i < 4 ; ++i)
      {
        getFace(neigh, i, faceElement);
        //do nothing for boundary
        if(faceElement.second[0] == faceElement.second[1])
          continue;
        auto it = std::find(activeFaceList.begin(),activeFaceList.end(), faceElement );
        //if face is not in active faces, insert
        if(it == activeFaceList.end())
        {
          //if face does not contain ref Edge
          if(i == 0 || i == type0node)
            activeFaceList.push_back(faceElement);
          else
            activeFaceList.push_front(faceElement);
        }
        else
          activeFaceList.erase(it);
      }
    }
    assert(activeFaceList.empty());
    return compatibilityCheck();
  }


  //An algorithm using only elements of type 1
  bool type1Algorithm()
  {
    //set all types to 1
    for(auto & type : types_ )
      type = 1;

    //the currently active Faces.
    //and the free faces that can still be adjusted at the end.
    FaceMapType activeFaces, freeFaces;
    //the finished elements. The number indicates the fixed node
    //if it is -1, the element has not been touched yet.
    std::vector<int> nodePriority;
    nodePriority.resize(maxVertexIndex_ +1, -1);
    int currNodePriority =maxVertexIndex_;

    const unsigned int numberOfElements = elements_.size();
    //walk over all elements
    for(unsigned int elIndex =0 ; elIndex < numberOfElements; ++elIndex)
    {
      //if no node is constrained and no face is active, fix one (e.g. smallest index)
      ElementType & el = elements_[elIndex];
      int priorityNode = -1;
      FaceType face;
      int freeFace = -1;
      for(int i = 0; i < 4; ++i)
      {
        //if a node has positive priority
        if(nodePriority[el[i]] > -1)
        {
          if(priorityNode < 0)
            priorityNode = i;
          //if it has maximum priority choose this index
          else if(nodePriority[el[i]] > nodePriority[el[priorityNode]])
            priorityNode = i;
        }
        getFace(el,i,face );
        //if we have a free face, the opposite node is good to be fixed
        if(freeFaces.find(face) != freeFaces.end())
          freeFace = i;
      }
      if(priorityNode > -1)
      {
        fixNode(el, priorityNode);
      }
      else if(freeFace > -1)
      {
        nodePriority[el[freeFace]] = currNodePriority;
        fixNode(el, freeFace);
        --currNodePriority;
      }
      else //fix a random node
      {
        nodePriority[el[type1node]] = currNodePriority;
        fixNode(el, type1node);
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
        unsigned int neighborIndex = faceElement.second[0] == elIndex ? faceElement.second[1] : faceElement.second[0];
        if(freeFaces.find(face) != freeFaces.end())
          while(!checkFaceCompatibility(faceElement))
          {
            rotate(elements_[neighborIndex]);
          }
        else if(activeFaces.find(face) != activeFaces.end())
        {
          if(!checkFaceCompatibility(faceElement))
          {
            checkFaceCompatibility(faceElement,true) ;
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
    //now postprocessing of freeFaces. possibly - not really necessary, has to be thought about
    //useful for parallelization .
    for(auto&& face : freeFaces)
    {
      unsigned int elementIndex = face.second[0];
      unsigned int neighborIndex = face.second[1];
      //give refinement edge positive priority
      //and non-refinement edge negative priority less than -1 and counting
    }
    return true;
  }

  std::vector<bool> returnElements(std::vector<ElementType> & elements, bool ALUexport = true)
  {
    for(unsigned int i =0 ; i < elements_.size(); ++i)
    {
      if(ALUexport && elementOrientation_[i])
        std::swap(elements_[i][2], elements_[i][3]);
    }
    elements = elements_;
    return elementOrientation_;
  }

  void stevenson2Alberta()
  {
    for(auto&& el : elements_)
    {
      std::swap(el[1],el[3]);
    }
  }


  void alberta2Stevenson()
  {
    for(auto&& el : elements_)
    {
      std::swap(el[1],el[3]);
    }
  }

private:

  //switch vertices 2,3 for all elements with elemIndex % 2
  void applyStandardOrientation ()
  {
    int i = 0;
    for(auto & element : elements_ )
    {
      if ( i % 2 == 0 )
      {
        std::swap(element[2],element[3]);
      }
      ++i;
    }
    types_.resize(elements_.size(), 0);
  }

  //check face for compatibility
  bool checkFaceCompatibility(std::pair<FaceType, EdgeType> face, bool verbose = true)
  {
    EdgeType edge1,edge2;
    int elIndex = face.second[0];
    int neighIndex = face.second[1];
    if(elIndex != neighIndex)
    {
      getRefinementEdge(elements_[elIndex], face.first, edge1, types_[elIndex]);
      getRefinementEdge(elements_[neighIndex], face.first, edge2, types_[neighIndex]);
      if(edge1 != edge2)
      {
        if (verbose)
        {
         /* std::cerr << "Face: " << face.first[0] << ", " << face.first[1] << ", " << face.first[2]
          << " has refinement edge: " << edge1[0] << ", " << edge1[1] <<
          " from one side and: " << edge2[0] << ", " << edge2[1] <<
          " from the other." << std::endl; */
          printElement(elIndex);
          printElement(neighIndex);
        }
        return false;
      }
    }
    return true;
  }

   //check face for strong compatibility
   //this check currently only works for a global ordered set of vertices
  bool checkStrongCompatibility(std::pair<FaceType, EdgeType> face, bool verbose = false)
  {
    int elIndex = face.second[0];
    int neighIndex = face.second[1];
    if(elIndex != neighIndex)
    {
      int elFaceIndex = getFaceIndex(elements_[elIndex], face.first);
      int neighFaceIndex = getFaceIndex(elements_[neighIndex], face.first);
      //if the local face indices coincide, they are reflected neighbors
      // if the refinement edge is not contained in the shared face, we
      // have reflected neighbors of the children, if the face is in the same direction
      // and the other edge of the refinement edge is the missing one.
      if(elFaceIndex != neighFaceIndex || !(elFaceIndex == 0 && neighFaceIndex == type0node) || !(elFaceIndex == type0node && neighFaceIndex == 0) )
      {
        if (verbose)
        {
         /* std::cerr << "Face: " << face.first[0] << ", " << face.first[1] << ", " << face.first[2]
          << " has refinement edge: " << edge1[0] << ", " << edge1[1] <<
          " from one side and: " << edge2[0] << ", " << edge2[1] <<
          " from the other." << std::endl; */
          printElement(elIndex);
          printElement(neighIndex);
        }
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
      std::swap(el[(type1node+1)%4],el[(type1node+2)%4]);
    }
  }

  //The rotations that keep the type 1 node fixed
  void rotate(ElementType& el)
  {
    std::swap(el[(type1node+1)%4],el[(type1node+2)%4]);
    std::swap(el[(type1node+1)%4],el[(type1node+3)%4]);
  }

  //get the sorted face of an element to a corresponding index
  //the index coincides with the missing vertex
  void getFace(ElementType el, int faceIndex, FaceType & face )
  {
    switch(faceIndex)
    {
    case 0 :
      face = {el[1],el[2],el[3]};
      break;
    case 1 :
      face = {el[0],el[2],el[3]};
      break;
    case 2 :
      face = {el[0],el[1],el[3]};
      break;
    case 3 :
      face = {el[0],el[1],el[2]};
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
        case 3 :
          edge = {el[0],el[2]};
          break;
        case 2 :
          edge = {el[0],el[3]};
          break;
        case 1 :
          edge = {el[0],el[3]};
          break;
        case 0 :
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
        case 3 :
          edge = {el[0],el[1]};
          break;
        case 2 :
          edge = {el[0],el[1]};
          break;
        case 1 :
          edge =  {el[0],el[2]};
          break;
        case 0 :
          edge =  {el[1],el[3]};
          break;
        default :
          std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR TETRAHEDRONS" << std::endl;
          abort();
        }
      }
    }
    else if(type == 1 || type == 2)
    {
      if(stevensonRefinement_)
      {
        switch(faceIndex)
        {
        case 3 :
          edge = {el[0],el[2]};
          break;
        case 2 :
          edge = {el[0],el[3]};
          break;
        case 1 :
          edge = {el[0],el[2]};
          break;
        case 0 :
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
        case 3 :
          edge = {el[0],el[1]};
          break;
        case 2 :
          edge = {el[0],el[1]};
          break;
        case 1 :
          edge = {el[0],el[type1node == 2 ? 3 :2]};
          break;
        case 0 :
          edge = {el[1],el[type1node == 2 ? 3 :2]};
          break;
        default :
          std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR TETRAHEDRONS" << std::endl;
          abort();
        }
      }
    }
    else
    {
      std::cerr << "no other types than 0, 1, 2 implemented." << std::endl;
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
    for(int i =0; i<4 ; ++i)
    {
      if(!( el[i] == face[0] || el[i] == face[1]  || el[i] == face[2] ) )
        return i ;
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
      for(int i = 0; i< 4; ++i)
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
      for(int i=0; i < 4 ; ++i)
        maxVertexIndex_ = std::max(maxVertexIndex_, el[i]);
    }
  }

  //build the structure of announced edges
  void buildEdgePriority(std::map<EdgeType, int> & edges)
  {
    EdgeType edge;
    for(auto & el : elements_)
    {
      getRefinementEdge(el, 2 , edge,0);
      auto pos = edges.find(edge);
      if(pos != edges.end())
        pos->second += 1;
      else
        edges.insert({edge, 1});
    }
  }

private:
  //the elements to be renumbered
  std::vector<ElementType> elements_;
  std::vector<bool> elementOrientation_;
  //the neighbouring structure
  FaceMapType neighbours_;
  //The maximum Vertex Index
  unsigned int maxVertexIndex_;
  //the element types
  std::vector<int> types_;
  //true if stevenson notation is used
  //false for ALBERTA
  const bool stevensonRefinement_;

}; //class bisectioncompatibility



#endif //ALUGRID_COMPATIBILITY_CHECK_HH_INCLUDED
