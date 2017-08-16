#ifndef ALUGRID_COMPATIBILITY_CHECK_HH_INCLUDED
#define ALUGRID_COMPATIBILITY_CHECK_HH_INCLUDED

#include <iostream>
#include <array>
#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <assert.h>


//Class to correct the element orientation to make bisection work in 3d
// It provides different algorithms to orientate a grid.
// Also implements checks for compatibility.
template <class VertexVector>
class BisectionCompatibility
{
  typedef BisectionCompatibility< VertexVector > ThisType;
public:
  // type of vertex coordinates stored inside the factory
  typedef VertexVector  VertexVectorType;

  typedef std::array<unsigned int, 3> FaceType;
  typedef std::vector< unsigned int > ElementType;
  typedef std::array<unsigned int, 2> EdgeType;
  typedef std::map< FaceType, EdgeType > FaceMapType;
  typedef std::pair< FaceType, EdgeType > FaceElementType;

protected:
  const VertexVectorType& vertices_;

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
  bool stevensonRefinement_;

  //the 2 nodes of the refinement edge
  EdgeType type0nodes_;  // = stevensonRefinement_ ? 0,3 : 0,1 ;
  //the faces opposite of type 0 nodes
  EdgeType type0faces_;
  //The interior node of a type 1 element
  int type1node_;  // = stevensonRefinement_ ? 1 : 2;
  //the face opposite of the interior node
  int type1face_;  // = 3 - type1node_ ;

public:
  //constructor taking elements
  //assumes standard orientation elemIndex % 2
  BisectionCompatibility( const VertexVectorType& vertices,
                          const std::vector<ElementType>& elements,
                          const bool stevenson)
    : vertices_( vertices ),
      elements_( elements ),
      elementOrientation_(elements_.size(), true),
      maxVertexIndex_(0),
      types_(elements_.size(), 0),
      stevensonRefinement_(stevenson),
      type0nodes_( stevensonRefinement_ ? EdgeType({0,3}) : EdgeType({0,1}) ),
      type0faces_( stevensonRefinement_ ? EdgeType({3,0}) : EdgeType({3,2}) ),
      type1node_( stevensonRefinement_ ? 1 : 2 ),
      type1face_( 3 - type1node_ )
  {
    //build the information about neighbours
    buildNeighbors();
  }

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

  double determinant( const int i ) const
  {
    Dune::FieldMatrix< double, 4, 3 > p( 0 );
    for( int j=0; j<4; ++j )
    {
      p[ j ] = vertices_[ elements_[ i ][ j ] ].first;
    }

    Dune::FieldMatrix< double, 3, 3 > matrix( 0 );

    for( int j=0; j<3; ++j )
    {
      matrix [j] = p[j+1] - p[0];
    }

    return matrix.determinant();
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

  double edgeLength( const int e, const int edge ) const
  {
    // ALBERTA numbering of edges (edges 2 and 3 swaped in comparison to DUNE refelement)
    //static const int edges[ 6 ][ 2 ] = { {0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3} };

    // DUNE reference element edge numbering
    static const int edges[ 6 ][ 2 ] = { {0,1}, {0,2}, {1,2}, {0,3}, {1,3}, {2,3} };

    const int vx0 = elements_[ e ][ edges[ edge ][ 0 ] ];
    const int vx1 = elements_[ e ][ edges[ edge ][ 1 ] ];
    return ( vertices_[ vx0 ].first - vertices_[ vx1 ].first ).two_norm2();
  }


  template <class Element>
  inline void rotate ( Element& element, int shift )
  {
    const int numVertices = 4 ;
    assert( element.size() == 4 );
    Element old( element );
    for( int j = 0; j < numVertices; ++j )
    {
      element[ j ] = old[ (j+shift) % numVertices ];
    }
  }

  int longestEdge ( int e ) const
  {
    int maxEdge = 0;
    double maxLength = edgeLength( e, 0 );
    for( int i = 1; i < 6; ++i )
    {
      const double length = edgeLength( e, i );
      if( length <= maxLength )
        continue;
      maxEdge = i;
      maxLength = length;
    }
    return maxEdge;
  }

  void sortForLongestEdge()
  {
    assert( !stevensonRefinement_ );

    static const int swapSuccessor[ 6 ] = { -1, 1, -1, -1, 3, -1 };

    // DUNE reference element
    static const int shift[ 6 ] = { 0, 0, 1, 3, 0, 2 };

    // ALBERTA reference element (edge 2 and 3 swaped compared to DUNE)
    //static const int shift[ 6 ] = { 0, 0, 3, 1, 0, 2 };

    std::cerr << "Marking longest edge for refinement..." << std::endl;

    const int numVertices = 4;

    //create the vertex priority List
    const int numberOfElements = elements_.size();
    for(int el = 0; el < numberOfElements ; ++el )
    {
      assert( int(elements_[ el ].size()) == numVertices );

      // find longest edge
      int edge = longestEdge( el );

      // mark longest edge as refinement edge
      if( shift[ edge ] > 0 )
      {
        rotate( elements_[ el ], shift[ edge ] );
      }

      if( swapSuccessor[ edge ] > 0 )
      {
        std::swap( elements_[ el ][  swapSuccessor[ edge ] ],
                   elements_[ el ][ (swapSuccessor[ edge ] + 1) % numVertices ] );
      }

#ifndef NDEBUG
      // make sure that the longest edge is the refinement edge (temporary assertion)
      const int refEdge = 0 ;//RefinementEdge< dimension >::value;
      const int longest = longestEdge( el );

      if( longest != refEdge )
      {
        std::cout << "refEdge = " << refEdge << " is not longest edge " << longest << std::endl;
        std::abort();
      }
#endif
    }

  }

  //an algorithm using only elements of type 0
  //it works by sorting the vertices in a global ordering
  //and tries to make as many reflected neighbors as possible.
  bool type0Algorithm( )
  {
    // convert ordering of refinement edge to stevenson
    alberta2Stevenson();

    // all elements are type 0
    std::fill( types_.begin(), types_.end(), 0 );

    std::list<unsigned int> vertexPriorityList;
    vertexPriorityList.clear();
    std::list<std::pair<FaceType, EdgeType> > activeFaceList; // use std::find to find
    std::vector<bool> doneElements(elements_.size(), false);

    ElementType el0 = elements_[0];
    //orientate E_0 (add vertices to vertexPriorityList)
    for(int vtx : el0)
    {
      vertexPriorityList.push_back ( vtx );
    }

    doneElements[0] = true;
    //add faces to activeFaceList, if not boundary
    //at beginning if face contains ref Edge,
    //else at the end
    FaceElementType faceElement;
    for(unsigned int i = 0; i < 4 ; ++i)
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
        if(i != type0faces_[0] && i != type0faces_[1] )
          activeFaceList.push_back(faceElement);
        else
          activeFaceList.push_front(faceElement);
      }
      else
        activeFaceList.erase(it);
    }

    //create the vertex priority List
    const int numberOfElements = elements_.size();
    for(int counter = 1; counter < numberOfElements ; ++counter)
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
      unsigned int faceInEl = getFaceIndex(el, faceElement.first);
      unsigned int nodeInEl = 3 - faceInEl;
      unsigned int faceInNeigh = getFaceIndex(neigh, faceElement.first);
      unsigned int nodeInNeigh = 3 - faceInNeigh;


      auto it = std::find(vertexPriorityList.begin(), vertexPriorityList.end(), neigh[nodeInNeigh]);
      if(it == vertexPriorityList.end() )
      {
        it = std::find(vertexPriorityList.begin(), vertexPriorityList.end(), el[nodeInEl]);
        //orientate element (add new vertex to vertexPriorityList)
        //This does a bit of magic by knowing that
        // the refinement edge is not contained in
        // face 0  and face type0node_.
        // So if we are in the mixed case, we do not want to
        // make reflected neighbors, but instead rather
        //that the children are reflected.
        // So we choose to insert the vertex after the
        // faceInNeigh index.
        if( (faceInEl == type0faces_[0] && faceInNeigh == type0faces_[1] ) || (faceInEl == type0faces_[1] && faceInNeigh == type0faces_[0] ) )
        {
          it = std::find(vertexPriorityList.begin(), vertexPriorityList.end(), el[faceInNeigh]);
          ++it;
        }
        vertexPriorityList.insert(it, neigh[nodeInNeigh]);
      }
      ElementType newNeigh({0,0,0});
      bool neighOrientation = elementOrientation_[neighIndex];
      auto it0 = std::find_first_of(vertexPriorityList.begin(), vertexPriorityList.end(), neigh.begin(), neigh.end());
      newNeigh[0] = *it0;
      ++it0;
      auto it1 = std::find_first_of(it0,vertexPriorityList.end(), neigh.begin(), neigh.end());
      newNeigh[1] = *it1;
      ++it1;
      auto it2 = std::find_first_of(it1,vertexPriorityList.end(), neigh.begin(), neigh.end());
      newNeigh[2] = *it2;
      for(unsigned int i =0 ; i < 3; ++i)
      {
        if( newNeigh[i] != neigh[i] )
        {
          auto neighIt = std::find(neigh.begin(),neigh.end(),newNeigh[i]);
          std::swap(*neighIt,neigh[i]);
          neighOrientation = ! neighOrientation;
        }
      }
      elementOrientation_[neighIndex] = neighOrientation;
      //add and remove faces from activeFaceList
      for(unsigned int i = 0; i < 4 ; ++i)
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
          if(i != type0faces_[0] && i != type0faces_[1] )
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
    // convert to stevenson ordering
    alberta2Stevenson();

    //set all types to 1
    std::fill( types_.begin(), types_.end(), 1 );

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
      int freeNode = -1;
      for(int i = 0; i < 4; ++i)
      {
        int tmpPrio = nodePriority[el[i]];
        //if a node has positive priority
        if( tmpPrio > -1 )
        {
          if( priorityNode < 0 || tmpPrio > nodePriority[el[priorityNode]] )
            priorityNode = i;
        }
        getFace(el,3 - i,face );
        //if we have a free face, the opposite node is good to be fixed
        if(freeFaces.find(face) != freeFaces.end())
          freeNode = i;
      }
      if(priorityNode > -1)
      {
        fixNode(el, priorityNode);
      }
      else if(freeNode > -1)
      {
        nodePriority[el[freeNode]] = currNodePriority;
        fixNode(el, freeNode);
        --currNodePriority;
      }
      else //fix a random node
      {
        nodePriority[el[type1node_]] = currNodePriority;
        fixNode(el, type1node_);
        --currNodePriority;
      }

      FaceElementType faceElement;
      //walk over all faces
      //add face 2 to freeFaces - if already free -great, match and keep, if active and not free, match and erase
      getFace(el,type1face_, faceElement);
      getFace(el,type1face_, face);

      const auto freeFacesEnd = freeFaces.end();
      const auto freeFaceIt = freeFaces.find(face);
      if( freeFaceIt != freeFacesEnd)
      {
        while(!checkFaceCompatibility(faceElement))
        {
          rotate(el);
        }
        freeFaceIt->second[1] = elIndex;
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
      //for(int i=0; i<4; ++i ) //auto&& i : {0,1,2,3})
      for(auto&& i : {0,1,2,3})
      {
        if (i == type1face_) continue;

        getFace(el,i,face);
        getFace(el,i,faceElement);

        unsigned int neighborIndex = faceElement.second[0] == elIndex ? faceElement.second[1] : faceElement.second[0];
        if(freeFaces.find(face) != freeFacesEnd)
        {
          while(!checkFaceCompatibility(faceElement))
          {
            rotate(elements_[neighborIndex]);
          }
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
    /*
    for(auto&& face : freeFaces)
    {
      unsigned int elementIndex = face.second[0];
      unsigned int neighborIndex = face.second[1];
      //give refinement edge positive priority
      //and non-refinement edge negative priority less than -1 and counting
    }
    */
    return true;
  }

  void returnElements(std::vector<ElementType> & elements,
                      std::vector<bool>& elementOrientation,
                      std::vector<int>& types,
                      const bool stevenson = false )
  {
    if( stevenson )
    {
      alberta2Stevenson();
    }
    else
    {
      stevenson2Alberta();
    }

    //This needs to happen, before the boundaryIds are
    //recreated in the GridFactory
    const int nElements = elements_.size();
    for( int el = 0; el<nElements; ++el )
    {
      // in ALU only elements with negative orientation can be inserted
      if( ! elementOrientation_[ el  ] )
      {
        // the refinement edge is 0 -- 1, so we can swap 2 and 3
        std::swap( elements_[ el ][ 2 ], elements_[ el ][ 3 ] );
      }
    }

    elements = elements_;
    elementOrientation = elementOrientation_;
    types = types_;
  }

  void stevenson2Alberta()
  {
    if( stevensonRefinement_ )
    {
      swapRefinementType();
    }
  }

  void alberta2Stevenson()
  {
    if( ! stevensonRefinement_ )
    {
      swapRefinementType();
    }
  }

private:
  void swapRefinementType()
  {
    const int nElements = elements_.size();
    for( int el = 0; el<nElements; ++el )
    {
      elementOrientation_[ el ] = !elementOrientation_[ el ];
      std::swap(elements_[ el ][ 1 ], elements_[ el ][ 3 ]);
    }

    // swap refinement flags
    stevensonRefinement_ = ! stevensonRefinement_;
    type0nodes_ = stevensonRefinement_ ? EdgeType({0,3}) : EdgeType({0,1}) ;
    type0faces_ = stevensonRefinement_ ? EdgeType({3,0}) : EdgeType({3,2}) ;
    type1node_ = stevensonRefinement_ ? 1 : 2 ;
    type1face_ = ( 3 -  type1node_ );
  }

  //switch vertices 2,3 for all elements with elemIndex % 2
  void applyStandardOrientation ()
  {
    int i = 0;
    for(auto & element : elements_ )
    {
      if ( i % 2 == 0 )
      {
        std::swap(element[2],element[3]);
        elementOrientation_[i] = ! elementOrientation_[i];
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
      if( elFaceIndex != neighFaceIndex ||
         !(elFaceIndex == type0faces_[0] && neighFaceIndex == type0faces_[1]) ||
         !(elFaceIndex == type0faces_[1] && neighFaceIndex == type0faces_[0]) )
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
    if(!(node == type1node_))
    {
      //swap the node at the right position
      std::swap(el[node],el[type1node_]);
      //also swap two other nodes to keep the volume positive
      //2 and 0 are never type1node_
      std::swap(el[(type1node_+1)%4],el[(type1node_+2)%4]);
    }
  }

  //The rotations that keep the type 1 node fixed
  void rotate(ElementType& el)
  {
    std::swap(el[(type1node_+1)%4],el[(type1node_+2)%4]);
    std::swap(el[(type1node_+1)%4],el[(type1node_+3)%4]);
  }

  //get the sorted face of an element to a corresponding index
  //the index coincides with the missing vertex
  void getFace(ElementType el, int faceIndex, FaceType & face )
  {
    switch(faceIndex)
    {
    case 3 :
      face = {el[1],el[2],el[3]};
      break;
    case 2 :
      face = {el[0],el[2],el[3]};
      break;
    case 1 :
      face = {el[0],el[1],el[3]};
      break;
    case 0 :
      face = {el[0],el[1],el[2]};
      break;
    default :
      std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR TETRAHEDRONS" << std::endl;
      std::abort();
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
          edge = {el[0],el[3]};
          break;
        case 3 :
          edge =  {el[1],el[3]};
          break;
        default :
          std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR TETRAHEDRONS" << std::endl;
          std::abort();
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
          std::abort();
        }
      }
    }
    else if(type == 1 || type == 2)
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
          edge = {el[0],el[3]};
          break;
        case 3 :
          edge =  {el[2],el[3]};
          break;
        default :
          std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR TETRAHEDRONS" << std::endl;
          std::abort();
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
          edge = {el[0],el[3]};
          break;
        case 3 :
          edge = {el[1],el[3]};
          break;
        default :
          std::cerr << "index " << faceIndex << " NOT IMPLEMENTED FOR TETRAHEDRONS" << std::endl;
          std::abort();
        }
      }
    }
    else
    {
      std::cerr << "no other types than 0, 1, 2 implemented." << std::endl;
      std::abort();
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
        return 3 - i ;
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
    // clear existing structures
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
}; //class bisectioncompatibility



#endif //ALUGRID_COMPATIBILITY_CHECK_HH_INCLUDED
