/*
 * TilePalette.h
 * Mountainhome
 *
 * Created by Andrew Jean on 12/4/10
 * Copyright 2010 Mountainhome Project. All rights reserved.
 */

#ifndef _TILEPALETTE_H_
#define _TILEPALETTE_H_

#include <Boost/any.hpp>
#include <vector>
#include <string>
#include "Assertion.h"

// Include this for use of VALUE, for some reason including ruby.h results in a redefinition of struct timespec
#include "RubyBindings.h"

class Material;

typedef char PaletteIndex;

enum {
    TILE_EMPTY = -1
};

typedef std::string ParameterID;
typedef boost::any ParameterData;
typedef std::map<ParameterID, ParameterData> ParameterMap;
typedef ParameterMap::iterator ParameterIterator;
typedef ParameterMap::const_iterator ConstParameterIterator;

class Tile {
public:
    Tile() { }
    Tile(const Tile &otherTile) {
        duplicate(otherTile);
    }
    ~Tile() {
        _parameters.clear();
    }

    VALUE getType() const { return _rubyType; }
    void setType(VALUE type) { _rubyType = type; }

    void duplicate(const Tile &otherTile) {
        otherTile.copyParameters(_parameters);
        _rubyType = otherTile.getType();
    }

    void copyParameters(ParameterMap &map) const {
        ConstParameterIterator itr = _parameters.begin();
        for(; itr != _parameters.end(); itr++) {
            map[(*itr).first] = (*itr).second;
        }
    }

    int numParameters() const {
        return _parameters.size();
    }

    bool hasParameter(ParameterID id) const {
        return (_parameters.find(id) != _parameters.end());
    }

    const ParameterData &getParameter(ParameterID id) const {
#if DEBUG
        ASSERT(hasParameter(id));
#endif
        return _parameters.find(id)->second;
    }

    void addParameter(ParameterID id, const ParameterData &value) {
        _parameters[id] = value;
    }

    void setParameter(ParameterID id, const ParameterData &value) {
#if DEBUG
        ASSERT(hasParameter(id));
#endif
        _parameters[id] = value;
    }

    bool isParameterEqual(const ParameterData &thisParameter, const ParameterData &otherParameter) const {
        if(thisParameter.type() != otherParameter.type()) { return false; }
        else if(thisParameter.type() == typeid(bool)) {
            return (boost::any_cast<bool>(thisParameter) ==
                    boost::any_cast<bool>(otherParameter));
        }
        else if(thisParameter.type() == typeid(char)) {
            return (boost::any_cast<char>(thisParameter) ==
                    boost::any_cast<char>(otherParameter));
        }
        else if(thisParameter.type() == typeid(std::string)) {
            return (boost::any_cast<std::string>(thisParameter) ==
                    boost::any_cast<std::string>(otherParameter));
        }
        else if(thisParameter.type() == typeid(int)) {
            return (boost::any_cast<int>(thisParameter) ==
                    boost::any_cast<int>(otherParameter));
        }
        else if(thisParameter.type() == typeid(double)) {
            return (boost::any_cast<double>(thisParameter) ==
                    boost::any_cast<double>(otherParameter));
        }
        else {
            Error("Can't compare properties");
        }
        return false;
    }

    void operator=(const Tile &other) {
        duplicate(other);
    }

    bool operator==(const Tile &other) const {
        if(_rubyType != other.getType()) { return false; }
        if(_parameters.size() != other.numParameters()) { return false; }

        ConstParameterIterator itr = _parameters.begin();
        for(; itr != _parameters.end(); itr++) {
            if(!other.hasParameter((*itr).first)) { return false; }

            const ParameterData &otherData = other.getParameter((*itr).first);
            if(!isParameterEqual((*itr).second, otherData)) { return false; }
        }
        return true;
    }

private:
    ParameterMap _parameters;
    VALUE _rubyType;

};

class TilePalette {
public:
    static const int IndexNotFound = -1;

public:
    TilePalette();
    ~TilePalette();

    PaletteIndex getPaletteIndex(const Tile &tile);

    Material * getMaterialForIndex(PaletteIndex index);
    const Tile &getTileForIndex(PaletteIndex index);

    int registerTile(const std::string &name, Tile &tile, Material *mat);

private:
    std::vector <Tile> _registeredTypes;
    std::vector <Material*> _registeredMaterials;
};

#endif
