/*
 *  ModelMeshPart.h
 *  Render
 *
 *  Created by Andrew Jean on on 8/05/10.
 *  Copyright 2010 Mountainhome Project
 *  All rights reserved.
 *
 */

#ifndef _MODELMESHPART_H_
#define _MODELMESHPART_H_

class ModelMeshPart {
public:
    ModelMeshPart();
    ~ModelMeshPart();

private:
    Material *_mat;         // The material associated with this mesh

    std::string _tag;       // This mesh's identifier

    int *_indexBuffer;      // The index buffer for this mesh
    int _startIndex;        // The location in the index buffer to begin reading verts

    Vertex3 *_vertexBuffer; // The vertex buffer for this mesh
    int _vertexOffset;      // The offset in vertices from the top of the index buffer

    int _numVerts;          // The number of vertices used during a draw call
    int _primCount;         // The number of primivites rendered
};

#endif