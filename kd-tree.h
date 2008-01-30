/**
 * Header file for 3d-Tree
 *
 * Author : Ricardo Marroquim
 * Date created : 08-01-2007
 *
 * To make this kd-tree more generic must create a template function
 * to support insertion and search function.
 * As it is ItemPtr must implement a "cartesian" method that returns
 * it's x, y, and z coordinates (CGAL-style).
 * Also, distance function assumes item is a pointe and uses CGAL
 * squared_distance function.
 *
 **/

#ifndef __KDTREE__
#define __KDTREE__

#include <vector>
#include <map>
#include "surfels.h"
//#include <assert.h>

using namespace std;

///
/// Refinement criteria for kd-tree
///
template <class ItemPtr>
struct KdTreeRefine {

  /// Decides whether or not to split a leaf node
  static bool split (const Box& world, const vector<ItemPtr>& items) {
    return false;
  }
};

///
/// A simple refinement criteria for leaf nodes based on occupancy.
/// A node is refined whenever it hold more than "Max" items
///
template <class ItemPtr, int Max=2>
struct OverflowRefine : public KdTreeRefine <ItemPtr> {

  static void merge ( ItemPtr a, ItemPtr b ) {

    Point centroid;
    Vector normal;
    // compute centroid of point list
    centroid[0] += a->p[0]*a->radius() + b->p[0]*b->radius();
    centroid[1] += a->p[1]*a->radius() + b->p[1]*b->radius();
    centroid[2] += a->p[2]*a->radius() + b->p[2]*b->radius();
    normal += a->normal()*a->radius();
    normal += b->normal()*b->radius();

    centroid /= a->radius() + b->radius();
    normal /= a->radius() + b->radius();

/*     centroid[0] = a->p[0] + b->p[0]; */
/*     centroid[1] = a->p[1] + b->p[1]; */
/*     centroid[2] = a->p[2] + b->p[2]; */
/*     normal = a->normal(); */
/*     normal += b->normal(); */

/*     centroid /= 2.0; */
/*     normal /= 2.0; */

    double max = sqrt(squared_distance(a->p, centroid)) + a->radius();
    double dist_radius_b = sqrt(squared_distance(b->p, centroid)) + b->radius();

    if (dist_radius_b > max)
      max = dist_radius_b;
  
    a->setPosition(centroid);
    a->setNormal(normal);
    a->setRadius(max);
  }

  /// Split a leaf node iff the list contains more than Max items
  static bool split (const Box& world, vector<ItemPtr>& items, vector<int> *mergedItems) {
    if (items.size() == 1) {
      mergedItems->push_back(items[0]->id());
      return false;
    }

    //    assert(items.size() == 2);
    if (items.size() != 2)
      cout << items.size() << endl;

    double d = sqrt(items[0]->p.squared_distance(items[1]->p));

    if (mergedItems->size() == Max)
      return true;
    else if (d >= (items[0]->r + items[1]->r))
      return true;

    mergedItems->push_back(items[1]->id());

    // Merge the two items
    if ((d + items[1]->r) > items[0]->r) {
      merge (items[0], items[1]);
    }

    items.pop_back();

    return false;
  }
};

///
/// A KdTree of Pointers to Objects
///
/// @param K Kernel
/// @param ItemPtr item pointer type to be stored
/// @param Refine refinement criteria
///
template < class ItemPtr, class Refine = OverflowRefine<ItemPtr> >
class KdTreeNode {

  /// List of what is actually stored in a leaf node (non-leaf nodes stores only one reference)
  typedef vector<ItemPtr> ItemPtrList;
  typedef const KdTreeNode * NodePtr;

  typedef multimap < double, ItemPtr, std::greater<double> > K_NearestMap;
  typedef pair<double, ItemPtr> K_NearestPair;

private:

  /// Split dimension, 0 for X, 1 for Y and 2 for Z (internal nodes only)
  int split_dim;

  /// Coordinate where the node space was split in two (internal nodes only)
  double split_coord;

  /// Pointers to left and right subtrees (if both are null it is a leaf node)
  KdTreeNode * son[2];
  
  /// Pointer to parent node
  KdTreeNode * father;

  /// List of pointers to points contained in node (if it is leaf) or point that
  /// splits this node in two
  ItemPtrList PtrList;

  /// Ids of merged items in this node
  vector<int> mergedItems;

  /// Node's coordinates
  Box world;

public:
  
  /// constructor
  KdTreeNode (const Box& w) : world(w) {
    son [0] = son [1] = 0;
    split_dim = 0;
    split_coord = 0;
    father = 0;
  }

  /// constructor
  KdTreeNode (KdTreeNode* f, const Box& w) : father(f), world(w) {
    son [0] = son [1] = 0;
    split_dim = 0;
    split_coord = 0;
  }

  /// destructor
  ~KdTreeNode () {
    for (int i = 0; i < 2; ++i) {
      if (son [i] != 0) delete son[i];
    }
  }

  /// Returns a pointer to the kd-tree node which contains point p
  /// @param p point which should be inside a descendant
  const KdTreeNode* searchLeaf (const Point& p) const {
    if (PtrList.size() == 1) { // internal node, contains only one object
      if (p == PtrList[0]->p) // found match
	return this;
      else {
	if (p[split_dim] > split_coord)
	  return son[1]->searchLeaf (p);
	else
	  return son[0]->searchLeaf (p);
      }
    }
    else if (PtrList.size() > 1) { // leaf node, search entire list for a match
      for (int i = 0; i < PtrList.size(); ++i) {
	if (PtrList[i]->p == p)
	  return this;
      }
      return NULL;
    }
  }


  KdTreeNode* next ( void ) const {
    if (son[0] != NULL)
      return son[0];
    if (son[1] != NULL)
      return son[1];

    if (father != NULL)
      return father->next(this);

    return NULL;
  }

  /// Traverses the kd-tree in breadthest-first order
  KdTreeNode* next ( const KdTreeNode* currentNode ) const {
    if (son[0] == currentNode) {
      if (son[1] != NULL)
	return son[1];
    }

    if (son[1] == currentNode) {
      if (father != NULL)
	return father->next(this);
      else
	return NULL;
    }
    return NULL;
  }

  /// Sets the number of merged items
  void setMergedItems (vector<int> m) { 
    mergedItems = m; 
  }

  vector<int>* mergedElements( void ) { return &mergedItems; }

  /// Returns the left son of this node
  /// @return left son
  const KdTreeNode* left(void) const {
    return son[0];
  }

  /// Returns the right son of this node
  /// @return right son
  const KdTreeNode* right(void) const {
    return son[1];
  }

  /// Returns the parent of this node
  /// @return parent node
  const KdTreeNode* parent(void) const {
    return father;
  }

  /// Searches for the leaf node containing p
  /// @param p Given point
  /// @return Pointer to node containing p
  const KdTreeNode* search (Point p) {
    if (isLeaf())
      return this;

    if (p[split_dim] < split_coord)
      return son[0]->search (p);
    else
      return son[1]->search (p); 
  }

  /// Computes the squared distance from a given point to the nodes box
  /// @param p Given point
  /// @return Squared distance
  double sqrtDistanceToBox (const Point& p) const {

    Point cls = p; //closest point on box to p

    if (p.x() <= world.xmin())
      cls = Point(world.xmin(), cls.y(), cls.z());
    else if (p.x() >= world.xmax())
      cls = Point(world.xmax(), cls.y(), cls.z());

    if (p.y() <= world.ymin())
      cls = Point(cls.x(), world.ymin(), cls.z());
    else if (p.y() >= world.ymax())
      cls = Point(cls.x(), world.ymax(), cls.z());

    if (p.z() <= world.zmin())
      cls = Point(cls.x(), cls.y(), world.zmin());
    else if (p.z() >= world.zmax())
      cls = Point(cls.x(), cls.y(), world.zmax());

    return p.squared_distance(cls);
  }

  /// Inserts a point in the set of k-nearest points
  /// If the set exceeds the k-limit, removes the point farthest away (first of set)
  /// @param item Neighbor to be inserted
  /// @param dist distance from p to search point
  /// @param k_nearest Ordered set of nearest neighbors
  /// @param k Number of nearest neighbors to find
  /// @return The distance to the farthest point in the map
  double insertNeighbor (ItemPtr item, double dist, K_NearestMap& k_nearest, unsigned int k) const {
    k_nearest.insert ( K_NearestPair (dist, item) );
    // if list has more than k nearest neighbors, remove first element (greater distance)
    if (k_nearest.size() > k)
      k_nearest.erase( k_nearest.begin() );
    return k_nearest.begin()->first;
  }

  /// Computes the k-nearest neighbors inside this node to the given point p
  /// @param p Given point
  /// @param k_nearest Ordered set of nearest neighbors
  /// @param k Number of nearest neighbors to find
  /// @return Number of distance comparisons made
  int kNearestLocalNeighbors (const Point& p, K_NearestMap& k_nearest, unsigned int k) const { 
    int comps = 0;
    double minDist;
    if (k_nearest.empty())
      minDist = HUGE; // empty set of nearest points
    else
      minDist = k_nearest.begin()->first; // greatest distance of all points in map

    // compute distance to all points inside the node (in case of internal nodes there is only one)
    for (unsigned int i = 0; i < PtrList.size(); ++i) {
      Point q = PtrList[i]->p;
      if (PtrList[i]->p != p) { // Check if not trying to insert itself
	double dist = p.squared_distance (q);
	++comps;
	if (dist < minDist || k_nearest.size() < k)
	  minDist = insertNeighbor (PtrList[i], dist, k_nearest, k);
      }
    }
    return comps;
  }

  /// Computes the k-nearest neighbors inside the kd-tree to a given point
  /// @param p Given point
  /// @param k_nearest Ordered set of nearest neighbors
  /// @param k Number of nearest neighbors to find
  int kNearestNeighbors (const Point& p, K_NearestMap& k_nearest, unsigned int k) const {
    int comps = 0;
    // Computes the distance to this node's itens
    comps = kNearestLocalNeighbors (p, k_nearest, k);

    if (!isLeaf()) // Descend to child nodes
      {
	// Compute distance from p to son's box
	double dists[2] = {son[0]->sqrtDistanceToBox (p), son[1]->sqrtDistanceToBox (p)};
	comps += 2;
	int order[2] = {0, 1};

	// Arrange in order of distances (closest first)
	if (dists[1] < dists[0]) { 
	  order[0] = 1;
	  order[1] = 0;
	}
     
	// Check distances to sons in ordered way, closest son first
	// Only checks if distance to son's box is less than distance to
	// farthes k-neighbor so far, or if hasn't found k-neighbots yet
	double best = k_nearest.begin()->first;
	if (dists[order[0]] < best || k_nearest.size() < k)
	  comps += son[order[0]]->kNearestNeighbors (p, k_nearest, k);
	best = k_nearest.begin()->first;
	if (dists[order[1]] < best || k_nearest.size() < k)
	  comps += son[order[1]]->kNearestNeighbors (p, k_nearest, k);

      }

    return comps;
  }

  /// Inserts a pointer to an object in this kd-tree node
  /// @param level kd-tree level of this node
  /// @param p pointer to object
  /// @param fatherPtr reference to the pointer inside the father which point to this node
  void insert (int level, const ItemPtr p) {

    if (son[0] == 0 && son[1] == 0) { // leaf node
      PtrList.push_back(p);

      // Check if overflow criteria is met
      if (Refine::split (world, PtrList, &mergedItems)) {

	/// Check largest box dimension for subdivision
	split_dim = (world.max().x() - world.min().x() > world.max().y() - world.min().y()) ? 0 : 1;
	split_dim = (world.max().pos(split_dim) - world.min().pos(split_dim) > world.max().z() - world.min().z()) ? split_dim : 2;

	/// Search for point closest to the center of split_dim
	//double center = 0.5 * (world.max().pos(split_dim) + world.min().pos(split_dim));

	//assert (PtrList.size() == 2);
	double center = (PtrList[0]->p[split_dim] + PtrList[1]->p[split_dim]) * 0.5;
	//	double minDist = HUGE;

/* 	ItemPtr middleItem = NULL; */
/* 	for (unsigned int i = 0; i < PtrList.size(); ++i) { */
/* 	  double dist = fabs (center - PtrList[i]->p[split_dim]); */
	  
/* 	  if (dist < minDist) { */
/* 	    minDist = dist; */
/* 	    middleItem = PtrList[i]; */
/* 	  } */
/* 	} */

	// Defines the position of the space partition
	//split_coord = middleItem->p[split_dim];
	split_coord = center;

	// Create two son nodes
	KdTreeNode * newLeftNode = new KdTreeNode(this, leftBox());
	KdTreeNode * newRightNode = new KdTreeNode(this, rightBox());
	son[0] = newLeftNode;
	son[1] = newRightNode;

	if (PtrList[0]->p[split_dim] < split_coord) {
	  son[0]->insert(level + 1, PtrList[0]);
	  son[1]->insert(level + 1, PtrList[1]);
	  son[0]->setMergedItems(mergedItems);
	}
	else {
	  son[1]->insert(level + 1, PtrList[0]);
	  son[0]->insert(level + 1, PtrList[1]);
	  son[1]->setMergedItems(mergedItems);
	}


	// Insert items from list into child nodes (except middle item)
/* 	for (unsigned int i = 0; i < PtrList.size(); ++i) { */
/* 	  if (PtrList[i]->p[split_dim] < split_coord) */
/* 	    son[0]->insert(level + 1, PtrList[i]); */
/* 	  else if (PtrList[i]->p[split_dim] > split_coord) */
/* 	    son[1]->insert(level + 1, PtrList[i]); */
/* 	} */

	// Insert split object into this node's empty list
	PtrList.clear();
	mergedItems.clear();

      }
    }
    else { // internal node, continue descending
      if (p->p[split_dim] < split_coord)
	son[0]->insert(level + 1, p);
      else
	son[1]->insert(level + 1, p);   
    }
  }
  
  /// Returns the number of pointers to items inserted into this node
  /// @return Size of item list
  int itemPtrCount () const {
    return PtrList.size();
  }

  /// Returns the ith element of the item list
  /// @param id The element position
  /// @return ith element of item list
  const ItemPtr element (int id) const {
    return PtrList[id];
  }

  /// Returns one of two sons of this node
  /// @param id Son identification (0 = left/bottom; 1 = right/top)
  /// @return Child node
  KdTreeNode * getSon(int id) {
    if (id == 0 || id == 1)
      return son[id];
    return NULL;
  }

  /// Returns the position where the split occurred
  /// @return Split coordinate
  double splitCoordinate(void) {
    return split_coord;
  }

  /// Returns the axis where the split occurred
  /// @return Split dimension (axis)
  int splitDim(void) {
    return split_dim;
  }

  /// Returns if this is leaf node or not
  /// @return 1 if leaf node, 0 otherwise
  bool isLeaf () const { return son[0] == 0 && son[1] == 0; }
  
  /// Returns the world coordinates
  /// @return World coodinates of this node
  const Box getBox(void) const { return world; }


private :

  /// Computes the left son world coordinates
  /// @return Left son world coordinates
  Box leftBox (void) const {
    Box leftWorld;
    Point p_max;
    if (split_dim == 0) // recompute x coordinate range	
      p_max = Point (split_coord, world.ymax(), world.zmax());
    else if (split_dim == 1)
      p_max = Point (world.xmax(), split_coord, world.zmax());
    else
      p_max = Point (world.xmax(), world.ymax(), split_coord);

    leftWorld = Box (world.min(), p_max);
    return leftWorld;
  }

  /// Computes the right son world coordinates
  /// @return Right son world coordinates
  Box rightBox (void) const {
    Box rightWorld;
    Point p_min;
    if (split_dim == 0) // recompute x coordinate range	
      p_min = Point (split_coord, world.ymin(), world.zmin());
    else if (split_dim == 1)
      p_min = Point (world.xmin(), split_coord, world.zmin());
    else
      p_min = Point (world.xmin(), world.ymin(), split_coord);
    rightWorld = Box (p_min, world.max());
    return rightWorld;
  }
};

/**
 * Base class for a kd-tree data structure -- only 3D!
 *
 * @param K CGAL Kernel to use
 * @param ItemPtr The kd-tree will store pointers of this type
 * @param Refine give criterion
 *
 **/
template <class ItemPtr, class Refine = OverflowRefine<ItemPtr> >
class KdTree {

public:
  /// List of what is actually stored in a leaf node (non-leaf nodes stores only one reference)
  typedef vector<ItemPtr> ItemPtrList;

  /// kd-tree node
  typedef KdTreeNode<ItemPtr, Refine> Node;
  
  /// Map definitions for k-nearest neighbors algorithm
  typedef multimap < double, ItemPtr, std::greater<double> > K_Map;
  typedef multimap <double, Point*, std::greater<double> >::iterator K_MapIterator;
  typedef pair<double, ItemPtr> K_NearestPair;

protected:

  /// kd-tree root node
  KdTreeNode <ItemPtr, Refine> * root;

public:

  /// Main Constructor
  KdTree (const Box& theWorld) : root (0) {
    root = new KdTreeNode <ItemPtr, Refine> (theWorld);
  }

  /// destructor
  virtual ~KdTree() {
    delete root;
  }

  /// Inserts a pointer to an object in this kd-tree
  /// @param p pointer to object
  virtual void insert (const ItemPtr item) {
    Point *p = &(item->p);
    // Check if point is inside kd-tree world before inserting
    if ( (p->x() >= root->getBox().xmin()) && (p->x() <= root->getBox().xmax()) &&
	 (p->y() >= root->getBox().ymin()) && (p->y() <= root->getBox().ymax()) &&
	 (p->z() >= root->getBox().zmin()) && (p->z() <= root->getBox().zmax()))
      root->insert (0, item);
  }

  /// Returns the number of pointer to items inserted into this kd-tree
  virtual int itemPtrCount (void) const { 
    return root->itemPtrCount();
  }

  /// Returns an iterator to the leaf node containing a given point
  /// @param p Given point
  /// @return Pointer to leaf node containing p
  const Node* search (const Point& p) const {
    return root->search (p);
  }

  /// Returns the nearesNeighbor to a given point
  /// @param p Given point.
  /// @return Pointer to the nearest neighbor object.
  ItemPtrList kNearestNeighbors (const Point& p, unsigned int k, int &comps) const {

    K_Map k_nearest;
    comps = root->kNearestNeighbors (p, k_nearest, k);

    ItemPtrList k_cls;
    K_MapIterator i;

    for (i = k_nearest.begin (); i != k_nearest.end(); ++i)
      k_cls.push_back (i->second);

    return k_cls;
  }
  
  /// Returns pointer to root node
  /// @return Pointer to root node
  Node* begin(void) {
    return root;
  }

};

#endif
