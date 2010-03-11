/*
 *  MHSceneManager.h
 *  System
 *
 *  Created by loch on 1/6/10.
 *  Copyright 2010 Brent Wilson. All rights reserved.
 *
 */

#ifndef _MHSCENEMANAGER_H_
#define _MHSCENEMANAGER_H_
#include <Render/SceneManager.h>
#include <Render/Model.h>
#include <Base/Vector.h>

class MHWorld;

/*! The MHSceneManager is specially desiged to render the MH world. It currently takes
 *  an MHWorld and is capable of querying the world for tile information, generating
 *  geometry, grouping it into renderable chunks and placing these chunks into the scene.
 *  The chunk size and tile sizes are all variable. 
 * \breif A SceneManager designed specifically for rendering MHWorld objects. */
class MHSceneManager : public SceneManager {
public:
    /*! Creates a new scene manager that draws the given world.
     * \param world The world the scene manager is responsible for drawing. */
    MHSceneManager(MHWorld *world);

    /*! D'tor */
    virtual ~MHSceneManager();

    /*! Causes the scene manager to examine the world's tile matrix and generates the
     *  geometry and entities needed to render everything properly. */
    virtual void populate() = 0;

protected:
    MHWorld *_world;       /*!< A reference to the world we're rednering. */
    int  _octreeMaxDepth;  /*!< How deep to allow the octree to grow. */
    Real _tileWidth;       /*!< The width of the tile in world units. */
    Real _tileHeight;      /*!< The height of the tile in world units. */
    Real _tileDepth;       /*!< The depth of the tile in world units. */

};

class WorldEntity : public Model {
public:
    WorldEntity(Vector3 *verts, Vector3 *norms, Vector2 *texCoords, int vertexCount);
    virtual ~WorldEntity();

    void render(RenderContext *context);

private:
    Vector2 *_texCoords;
    Vector3 *_verts;
    Vector3 *_norms;
    int _count;
};

class IndexedWorldEntity : public Model {
public:
    IndexedWorldEntity(unsigned int *indices, int indexCount, Vector3 *verts, Vector3 *norms, int vertexCount);
    virtual ~IndexedWorldEntity();

    void render(RenderContext *context);

private:
    unsigned int *_indices;
    int _indexCount;

    Vector3 *_verts;
    Vector3 *_norms;
    int _vertexCount;
};

#endif
