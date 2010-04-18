/*
 *  MHReducedWorldModel.cpp
 *  Mountainhome
 *
 *  Created by loch on 4/17/10.
 *  Copyright 2010 Mountainhome Project. All rights reserved.
 *
 */

#include "MHReducedWorldModel.h"

#include <algorithm>
#include <Render/RenderContext.h>

class LODIndexArray {
    struct Face;
    typedef unsigned int IndexArrayIndex;
    typedef unsigned int VertexArrayIndex;
    typedef std::list<Face*>  FaceList;
    typedef std::map<VertexArrayIndex, FaceList> FaceMap;

    struct Face {
        Vector3 normal;
        IndexArrayIndex  indexArrayIndices[3];
        VertexArrayIndex vertexArrayIndices[3];

        Face(int index, VertexArrayIndex *indices, Vector3 *verts) {
            for (int i = 0; i < 3; i++) {
                indexArrayIndices [i] = index+i;
                vertexArrayIndices[i] = indices[indexArrayIndices[i]];
            }

            Vector3 one = verts[vertexArrayIndices[1]] - verts[vertexArrayIndices[0]];
            Vector3 two = verts[vertexArrayIndices[2]] - verts[vertexArrayIndices[1]];
            normal = one.crossProduct(two);
            normal.normalize();

//            if (normal != Vector3(0, 1, 0)) {
//            if (Math::lt(normal.z, 0)) {
//                Info("BRENT");
//                Info("BRENT: " << verts[vertexArrayIndices[0]]);
//                Info("BRENT: " << verts[vertexArrayIndices[1]]);
//                Info("BRENT: " << verts[vertexArrayIndices[2]]);
//                Info("BRENT");
//                Info("BRENT: " << one);
//                Info("BRENT: " << two);
//                Info("BRENT");
//                Info("BRENT: " << normal);
//                Info("/BRENT");
//            }
        }

        bool hasVertexArrayIndex(VertexArrayIndex index) {
            return vertexArrayIndices[0] == index ||
                   vertexArrayIndices[1] == index ||
                   vertexArrayIndices[2] == index;
        }
    };

private:
    FaceList _allFaces;
    FaceMap  _faceMapping;

    int _indexCount;
    VertexArrayIndex *_indices;
    Vector3 *_verts;
    AABB3 _aabb;

private:
    void getFacesUsingIndexFromList(VertexArrayIndex index, const FaceList &from, FaceList &to) {
        FaceList::const_iterator itr = from.begin();
        for (; itr != from.end(); itr++) {
            if ((*itr)->hasVertexArrayIndex(index)) {
                to.push_back(*itr);
            }
        }
    }

    float calculateCurvature(VertexArrayIndex indexA, VertexArrayIndex indexB) {
        FaceList aFaces, bFaces;
        aFaces = _faceMapping[indexA];
        getFacesUsingIndexFromList(indexB, aFaces, bFaces);
        ASSERT(bFaces.size() > 0);

        float totalCurvature = 0;
        FaceList::iterator aItr, bItr;
        for (aItr = aFaces.begin(); aItr != aFaces.end(); aItr++) {

            // Compare each of the shared faces to each of indexA's faces, looking for the
            // smallest curvature. This will be the cost of the individual face that will
            // be disapearing. Note, this is what allows us to drop edges that are
            // actually on edges. Consider the cube example, where there is a vertex
            // sitting on a cube edge. We know we can make it go away, at zero cost, and
            // this is how that is accounted for.
            float minCurvature = 1;
            for (bItr = bFaces.begin(); bItr != bFaces.end(); bItr++) {
                float currentCurvature = (*aItr)->normal.dotProduct((*bItr)->normal);
                currentCurvature = (1 - currentCurvature) / 2.0;
                minCurvature = std::min(minCurvature, currentCurvature);
            }
            totalCurvature = std::max(totalCurvature, minCurvature);
        }

        return totalCurvature;
    }

    /*! Gets a foating point value representing the cost of collapsing the vertex at
     *  indexA into the vertex at indexB. This value is computed as a function of the
     *  length of the edge we'd be collapsing (longer edges mean smaller differences will
     *  have a greater impact) and a value representing the curvature of the polygons
     *  around indexA. */
    float determineCost(VertexArrayIndex indexA, VertexArrayIndex indexB) {
        // I should use squared length here.
        float edgeLength = (_verts[indexA] - _verts[indexB]).length();
        return edgeLength * calculateCurvature(indexA, indexB);
    }

    int findIndexToMergeWith(IndexArrayIndex index, float maxCost = 0.05) {
        // Never merge corners into anything.
        if (isOnCorner(index)) { return -1; }

        FaceList faceList = _faceMapping[_indices[index]];
        float minCost = maxCost;
        int minIndex = -1;

        FaceList::iterator itr = faceList.begin();
        for (; itr != faceList.end(); itr++) {
            for (int i = 0; i < 3; i++) {
                int curIndex = (*itr)->indexArrayIndices[i];
                if (_indices[index] != _indices[curIndex]) {

                    // Never merge boundry verts that don't share a boundry.
                    if (!canMerge(index, curIndex)) { continue; }

                    float curCost = determineCost(_indices[index], _indices[curIndex]);
                    Info("    Cost: " << curCost);

                    if (curCost >= 0 && curCost <= minCost) {
                        minIndex = curIndex;
                        minCost = curCost;
                    }
                }
            }
        }

        return minIndex;
    }

    void calculateStuff() {
        clear_list(_allFaces);
        _faceMapping.clear();

        // Build all of the faces.
        Info("Index Count: " << _indexCount);
        for (int i = 0; i < _indexCount; i+=3) {
            Face *face = new Face(i, _indices, _verts);
            Info("Face: ");
            Info("    " << _verts[face->vertexArrayIndices[0]]);
            Info("    " << _verts[face->vertexArrayIndices[1]]);
            Info("    " << _verts[face->vertexArrayIndices[2]]);
            Info("    " << face->normal);

            if (face->normal != Vector3(0, 0, 0)) {
                _allFaces.push_back(face);
            } else {
                delete face;
            }
        }

        delete[] _indices;
        _indexCount = _allFaces.size() * 3;
        _indices = new VertexArrayIndex[_indexCount];
        FaceList::iterator itr = _allFaces.begin();
        for (int i = 0; itr != _allFaces.end(); itr++, i+=3) {
            for (int j = 0; j < 3; j++) {
                _indices[i+j] = (*itr)->vertexArrayIndices[j];
                (*itr)->indexArrayIndices[j] = i+j;
            }
        }

        // Map all of the indices to faces.
        FaceList tempList;
        for (int i = 0; i < _indexCount; i++) {
            getFacesUsingIndexFromList(i, _allFaces, tempList);
            _faceMapping[i] = tempList;
            tempList.clear();
        }
    }

    bool isOnBoundry(IndexArrayIndex index) {
        Vector3 vert = _verts[_indices[index]];
        Vector3 min = _aabb.getMin();
        Vector3 max = _aabb.getMax();
        return Math::eq(vert.x, min.x) || Math::eq(vert.x, max.x) ||
               Math::eq(vert.y, min.y) || Math::eq(vert.y, max.y);
    }

    bool shareBoundry(IndexArrayIndex indexA, IndexArrayIndex indexB) {
        Vector3 vertA = _verts[_indices[indexA]];
        Vector3 vertB = _verts[_indices[indexB]];
        Vector3 min = _aabb.getMin();
        Vector3 max = _aabb.getMax();

        return (Math::eq(vertA.x, min.x) && Math::eq(vertB.x, min.x)) ||
               (Math::eq(vertA.x, max.x) && Math::eq(vertB.x, max.x)) ||
               (Math::eq(vertA.y, min.y) && Math::eq(vertB.y, min.y)) ||
               (Math::eq(vertA.y, max.y) && Math::eq(vertB.y, max.y));
    }

    bool canMerge(IndexArrayIndex indexA, IndexArrayIndex indexB) {
        bool a = !isOnBoundry(indexA);
        bool b = shareBoundry(indexA, indexB);
        Info("    Checking " << _verts[_indices[indexA]] << " into " << _verts[_indices[indexB]] << ": " << a << " || " << b);
        return a || b;
    }

    bool isOnCorner(IndexArrayIndex index) {
        Vector3 vert = _verts[_indices[index]];
        Vector3 min = _aabb.getMin();
        Vector3 max = _aabb.getMax();
        return (Math::eq(vert.x, min.x) || Math::eq(vert.x, max.x)) &&
               (Math::eq(vert.y, min.y) || Math::eq(vert.y, max.y));
    }

    bool reduce() {
        bool updated = false;
        Info("Reducing poly count! Starting at " << _indexCount / 3);
        for (int indexA = 0; indexA < _indexCount; indexA++) {
            int indexB = findIndexToMergeWith(indexA);
            if (indexB >= 0) {
                Info("Merging " << indexA << " -> " << _indices[indexA] << " " << _verts[_indices[indexA]] << " "
                     "into "    << indexB << " -> " << _indices[indexB] << " " << _verts[_indices[indexB]]);

                VertexArrayIndex toMerge = _indices[indexA];
                for (int i = 0; i < _indexCount; i++) {
                    if (_indices[i] == toMerge) {
                        _indices[i] = _indices[indexB];
                    }
                }

                updated = true;
            } else {
//                Info("Got [" << indexB << "] Could not find anything to merge with " << indexA << " -> " << _indices[indexA] << " "
//                     "[" << _verts[_indices[indexA]].x << ", " << _verts[_indices[indexA]].y << ", " << _verts[_indices[indexA]].z << "]");
            }
        }

        calculateStuff();
        return updated;
    }

public:
    LODIndexArray(VertexArrayIndex *indices, int indexCount, Vector3 *verts, AABB3 aabb):
    _indexCount(indexCount), _indices(indices), _verts(verts), _aabb(aabb) {
        calculateStuff();
        while(reduce()) {}
        Info("Final poly count: " << _indexCount / 3 << " (" << 100.0 * _indexCount / indexCount << "%)");
    }

    virtual ~LODIndexArray() {
        clear_list(_allFaces);
        delete[] _indices;
        _indices = NULL;
    }

    VertexArrayIndex* getPtr(int lod = 0) {
        return _indices;
    }

    int getCount(int lod = 0) {
        return _indexCount;
    }
};

MHReducedWorldModel::MHReducedWorldModel(unsigned int *indices, int indexCount, Vector3 *verts, Vector3 *norms, Vector2 *texCoords, int vertexCount):
MHWorldModel(verts, norms, texCoords, vertexCount), _indices(NULL) {
    _indices = new LODIndexArray(indices, indexCount, verts, _boundingBox);

    /*Info("WORLD MODEL:");
    LogStream::IncrementIndent();
    for (int i = 0; i < vertexCount; i++) {
        Info("[" << i << "] "
             "(" << verts[i].x << ", " << verts[i].y << ", " << verts[i].z << ") "
             "(" << norms[i].x << ", " << norms[i].y << ", " << norms[i].z << ") "
             "(" << texCoords[i].x << ", " << texCoords[i].y << ") ");
    }
    LogStream::DecrementIndent();*/
}

MHReducedWorldModel::~MHReducedWorldModel() {
    delete _indices;
    _indices = NULL;
}

void MHReducedWorldModel::render(RenderContext *context) {
    context->addToPrimitiveCount(_indices->getCount() / 3 * 2);
    context->addToVertexCount(_count * 2);
    context->addToModelCount(1);

//    context->setWireFrame();

    if (_verts) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, _verts);
    }

    if (_norms) {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, _norms);
    }

    if (_texCoords) {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, _texCoords);
    }

    glDrawElements(GL_TRIANGLES, _indices->getCount(), GL_UNSIGNED_INT, _indices->getPtr());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//    context->setFilled();

    if (0 && _norms) {
        glDisable(GL_LIGHTING);

        glUseProgramObjectARB(0);

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_2D, NULL);
        glDisable(GL_TEXTURE_2D);

        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture(GL_TEXTURE_2D, NULL);
        glDisable(GL_TEXTURE_2D);

        glBegin(GL_LINES);
        for (int i = 0; i < _count; i++) {
            float color = pow(_norms[i].z, 5.0);
            glColor4f(1.0, color, color, 1.0);
            glVertex3fv(_verts[i].array);
            glVertex3fv((_verts[i] + _norms[i]).array);
        }
        glEnd();

        glEnable(GL_LIGHTING);
    }
}
