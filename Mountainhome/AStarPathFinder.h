#ifndef _ASTARPATHFINDER_H_
#define _ASTARPATHFINDER_H_

#include "PathManager.h"

class AStarNode {
public:
    AStarNode(PathNode *node, AStarNode *parent, int gCost, int hCost);

    int fCost() const;
    int hCost() const;

    void setGCost(int value);
    void setParent(AStarNode *parent);

    PathNode *getPathNode() const;
    AStarNode *getParent() const;

    bool operator<(const AStarNode &other);

private:
    AStarNode *_parent;
    PathNode *_pathNode;
    int _gCost, _hCost;
};

typedef std::list<AStarNode*> AStarList;

class AStarPathFinder : public PathManager {
public:
    AStarPathFinder(Vector3 dimensions);

    virtual int getPath(Vector3 start, Vector3 end, Path &path);

private:
    void parseConnectedNode(AStarNode *currentNode, const PathEdge &edge, Vector3 goal);
    int fillPath(Vector3 start, Vector3 end, AStarNode *currentNode, Path &path);

    int distance(Vector3 start, Vector3 end);

private:
    AStarList _openList;
    AStarList _closedList;
};

#endif
