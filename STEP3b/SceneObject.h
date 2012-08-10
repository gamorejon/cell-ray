#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include "Color.h"
#include "Ray.h"
#include "Vertex.h"
#include "SceneObject.h"
#include "VertexGroup.h"
#include "ColorGroup.h"
#include "AlignedStruct.h"

class SceneObject : public AlignedStruct
{
protected:
    VertexGroup center;
    vector float reflective;
	float reflectiveScalar;
    ColorGroup diffusionColor;
    vector float specularReflection;

public:
    SceneObject(Vertex* _center, float _reflective, Color* _diffusionColor,
    	float _specularReflection)
    {
		center = VertexGroup(const_cast<const Vertex&>(*_center));

        reflective = splatScalar(_reflective);
		reflectiveScalar = _reflective;
		diffusionColor = ColorGroup(*_diffusionColor);
		specularReflection = splatScalar(_specularReflection);

		delete _diffusionColor;
		delete _center;
    }

    virtual ~SceneObject()
    {
//		aligned_free(center);
//		delete center;
//		delete centerVertex;
//		delete diffusionColor;
    }

    VertexGroup* getCenter() { return &center; }

    ColorGroup* getDiffusionColor() { return &diffusionColor; }

    vector float getReflective() { return reflective; }
//    void setReflective(vector float _reflective) { reflective = _reflective; }

	float getReflectiveScalar() { return reflectiveScalar; }

    vector float getSpecularReflection() { return specularReflection; }
/*    void setSpecularReflection(vector float _specularReflection)
    {
		specularReflection = _specularReflection;
    } */

    bool shouldReflect(int depth) { depth = depth; return true; }

    /*virtual vec_bint4 isIntersected(Ray* ray, VertexGroup* start,
		vec_bint4 hit) = 0;

	virtual void getReflection(Ray* ray, Ray* reflection, 
		VertexGroup* normal, VertexGroup* centers) = 0;

	virtual void blah() = 0; */
};

#endif

