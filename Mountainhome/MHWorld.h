/*
 *  MHWorld.h
 *  System
 *
 *  Created by loch on 2/16/10.
 *  Copyright 2010 Brent Wilson. All rights reserved.
 *
 */

#ifndef _MHWORLD_H_
#define _MHWORLD_H_
#include <Engine/Entity.h>
#include <Base/Vector.h>

class MaterialManager;
class ModelManager;

class OctreeSceneManager;
class Camera;
class MHCamera;
class MHObject;
class MHCore;
class MHSelection;
class EntityBindings;
class MHTerrain;
class MHPathFinder;

/*! Represents the world itself in game. This contains all of the hooks into the engine
 *  and handles any and all direct engine interaction that may need to be done, such as
 *  setting up the initial scene: clear color, camera properties, lights, etc... It also
 *  provides the ruby bindings necessary to interact with the C++ object from within ruby.
 * \note This class should remain generally barebones, leaving much of the higher level
 *  logic to the ruby class. */
class MHWorld {
public:
    /*! Creates a new MHWorld */
    MHWorld();

    /*! Destroys the world and releases everything associated with it. */
    virtual ~MHWorld();

    /* Sets up the given scene to render everything correctly.
     * \param width The width of the new world.
     * \param height The height of the new world.
     * \param depth The depth of the new world. */
    void initialize(MHCore *core);

    /* Calls on the scene manager to create a camera, marking it and initializing its Ruby bindings */
    Camera* createCamera(std::string cameraName);

    /*! Gets the scene manager that was created by the world. */
    OctreeSceneManager *getScene() const;
    
    /*! Gets the terrain object. */
    MHTerrain *getTerrain() const;

    /*! Gets the path finder object. */
    MHPathFinder *getPathFinder() const;

    /*! Tells the terrain object to pass world geometry to the scene. */
    void populate();

    /*! Gets the width of the world */
    int getWidth();

    /*! Gets the height of the world */
    int getHeight();

    /*! Gets the depth of the world */
    int getDepth();

    MHSelection* getSelection();

    MaterialManager *getMaterialManager();

    ModelManager *getModelManager();

    /*! Saves the world data */
    void save(std::string worldName);

    /*! Loads world data from a group of files */
    bool load(std::string worldName);

    void loadEmpty(int width, int height, int depth, MHCore *core);

    /*! Returns a list of objects within a selection area */
    void pickObjects(Camera *activeCam, Real startX, Real startY, Real endX, Real endY);

    /*! Projects a ray into the world and returns the first tile it hits,
     *  returning false if a tile is not hit. */
    bool projectRay(const Vector3 &start, const Vector3 &dir, Vector3 &nearestTile);

protected:
    /*! Creates and initializes the scene, setting up cameras, lights, etc... */
    void initializeScene();

    /*! Updates the viewports based on the current state. */
    void updateViewports();

protected:
    MaterialManager *_materialManager;
    ModelManager *_modelManager;
    OctreeSceneManager *_scene;

    MHTerrain *_terrain;
    MHSelection *_selection;
    MHPathFinder *_pathFinder;

    bool  _split;  /*!< Whether or not split screen is active. */
    int   _width;  /*!< The width of the world. */
    int   _height; /*!< The height of the world. */
    int   _depth;  /*!< The depth of the world. */
};

#endif
