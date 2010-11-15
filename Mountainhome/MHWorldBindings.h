/*
 *  MHWorldBindings.h
 *  Mountainhome
 *
 *  Created by loch on 10/23/10.
 *  Copyright 2010 Mountainhome Project. All rights reserved.
 *
 */

#ifndef _MHWORLDBINDINGS_H_
#define _MHWORLDBINDINGS_H_
#include "RubyBindings.h"
#include "MHWorld.h"

#include "OctreeSceneManager.h"

#include <Render/MaterialManager.h>
#include <Render/ModelManager.h>

class MHWorldBindings : public RubyBindings<MHWorld, true> {
public:
    /*! Creates the MHWorld bindings. */
    static void SetupBindings();

    /*! Marks the camera and world objects to keep the from being deleted. */
    static void Mark(MHWorld* world);

    /*! Creates a new instance of the MHWorld object and registers it as being associated
     *  with the given ruby object VALUE.
     * \param self The ruby space World object.
     * \param width The width of the world in tiles.
     * \param height The height of the world in tiles.
     * \param depth The depth of the world in tiles. */
    static VALUE Initialize(VALUE self, VALUE rCore);

    /*! Calls on scenemanager to create a new camera object */
    static VALUE CreateCamera(VALUE self, VALUE cameraName);

    /*! Terrain getter. */
    static VALUE GetTerrain(VALUE self);

    /*! LiquidManager getter. */
    static VALUE GetLiquidManager(VALUE self);

    /*! Tells the world to generate geometry in the scene.
     * \param self The ruby space World object. */
    static VALUE Populate(VALUE self);

    /*! Defers to Create<T1, T2>. Need to do this because of issues with pointers to
     *  function templates. */
    static VALUE CreateEntity(VALUE rSelf, VALUE klass, VALUE name, VALUE model, VALUE material);

    /*! Defers to Create<T1, T2>. Need to do this because of issues with pointers to
     *  function templates. */
    static VALUE CreateActor(VALUE rSelf, VALUE klass, VALUE name, VALUE model, VALUE material);

    /*! Scene object creation. Can be used to create derivatives of T. */
    template <typename T, typename TBindings>
    static VALUE Create(VALUE rSelf, VALUE klass, VALUE name, VALUE model, VALUE material);

	/*! Get the selection in this world. */
	static VALUE GetSelection(VALUE self);
    
    /*! Gets the world's width. */
    static VALUE GetWidth(VALUE self);

    /*! Gets the world's height. */
    static VALUE GetHeight(VALUE self);

    /*! Gets the world's depth. */
    static VALUE GetDepth(VALUE self);

    /*! Import/Export the world. */
    static VALUE Save(VALUE self, VALUE world);
    static VALUE Load(VALUE self, VALUE world);
    static VALUE LoadEmpty(VALUE rSelf, VALUE width, VALUE height, VALUE depth, VALUE rCore);

    /*! Do picking. */
    static VALUE PickObjects(VALUE rSelf, VALUE rCam, VALUE rLeft, VALUE rBottom, VALUE rRight, VALUE rTop);

public:
    MHWorldBindings();
    virtual ~MHWorldBindings() {}

private:
    ID _terrain;
    ID _liquidManager;

};

template <typename T, typename TBindings>
VALUE MHWorldBindings::Create(VALUE rSelf, VALUE klass, VALUE name, VALUE model, VALUE material) {
    MHWorld *cSelf = MHWorldBindings::Get()->getPointer(rSelf);

    // Setup the object.
    std::string cName  = rb_string_value_cstr(&name);
    T* cEntity = cSelf->getScene()->create<T>(cName);

    // Handle the model, if there is one.
    if (model != Qnil) {
        std::string cModelName = rb_string_value_cstr(&model);
        Model *cModel = cSelf->getModelManager()->getOrLoadResource(cModelName);
        cEntity->setModel(cModel);

        // Get the material.
        std::string cMaterialName = rb_string_value_cstr(&material);
        Material *cMaterial = cSelf->getMaterialManager()->getOrLoadResource(cMaterialName);
        cModel->setDefaultMaterial(cMaterial);
    }

    // define and return new Ruby-side MHEntity class object
    return NEW_RUBY_OBJECT_FULL(TBindings, cEntity, klass);
}

#endif