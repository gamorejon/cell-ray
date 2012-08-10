#ifndef SCENE_H
#define SCENE_H
#include <vector>
#include <cmath>
using namespace std;
#include <altivec.h>

#include "SceneObject.h"
#include "ColorGroup.h"
#include "Intersection.h"
#include "Light.h"
#include "AltivecUtils.h"
#include "VectorSInt.h"

class Scene
{
private:
    SceneObject** objects;
    Light**  lights;
	ColorGroup* ambientColor;
	int objectsSize;
	int lightsSize;
	VertexGroup* lightV;
	VertexGroup* reflectionStart;
	VertexGroup* centers;
	VectorSInt* minIndexVec;
	ColorGroup* surfaceColorGroup;

	vector float getSpec(int index)
	{
		if (index == -1)
		{
			return negativeZero();
		}
		else
		{
			return getSceneObject(index)->getSpecularReflection();
		}
	}

	vector float getPt(int index, int number)
	{
		if (index == -1)
		{
			return negativeZero();
		}
		else
		{
			VertexGroup* center = getSceneObject(index)->getCenter();

			switch (number)
			{
				case 1:
					return center->x;
				case 2:
					return center->y;
				case 3:
					return center->z;
				default:
					return negativeZero();
			}
		}
	}

	vector float getDiff(int index, int number)
	{
		if (index == -1)
		{
			return negativeZero();
		}
		else
		{
			ColorGroup* diff = getSceneObject(index)->getDiffusionColor();

			switch (number)
			{
				case 1:
					return diff->r;
				case 2:
					return diff->g;
				case 3:
					return diff->b;
				default:
					return negativeZero();
			}
		}
	}

public:
    Scene(Light** _lights, int _lightsSize, SceneObject** _objects, 
		int _objectsSize, Color* _ambientColor)
    {
		lights = _lights;
		lightsSize = _lightsSize;
		objects = _objects;
		objectsSize = _objectsSize;
		ambientColor = new ColorGroup(*_ambientColor);

		Color zero = Color();
		surfaceColorGroup = new ColorGroup(zero);
		lightV = new VertexGroup(Vertex());
		reflectionStart = new VertexGroup(Vertex());
		centers = new VertexGroup(Vertex());
		minIndexVec = new VectorSInt();

		delete _ambientColor;
    }

    ~Scene()
    {
/*		cout << "Beginning scene delete" << endl;

		for (int i = 0; i < objectsSize; i++)
		{
			cout << "Deleting object " << i << endl;
		//	delete objects[i];
			cout << "Deleted object " << i << endl;
		}
		
		for (int i = 0; i < lightsSize; i++)
		{
			cout << "Deleting light " << i << endl;
		//	delete lights[i];
		}

		cout << "Deleting[] objects" << endl; 

		delete[] objects;
		delete[] lights;
		delete ambientColor;

		cout << "After ambientColor" << endl;
		delete lightV;
		delete reflectionStart;
		delete centers;
		delete minIndexVec;
		delete surfaceColorGroup;

		cout << "End scene delete" << endl;*/
    }

    SceneObject* getSceneObject(int index)
    {
		return objects[index];
    }

	Light* getLight(int index)
	{
		return lights[index];
	}

    int getSceneObjectsSize()
    {
		return objectsSize;
    }

	int getLightsSize()
	{
		return lightsSize;
	}

    void getIntersection(Ray* ray, Intersection& intersection)
	{
    	vector float distance = negativeZero();

	    vector int minIndex = vec_splat_s32(-1);

	    vector float minDistance = splatScalar(1000.0f);

		vector unsigned int boolZero = vec_splat_u32(0);

		vector bool int anyHit = (vector bool int)boolZero;

		SceneObject* object;

		// TODO: keeping the SceneObject is Sphere specific, and not good.

	    for (int i = 0; i < getSceneObjectsSize(); i++)
	    {
	        object = getSceneObject(i);
	
	        vector bool int hit = object->isIntersected(ray, 
				reflectionStart, intersection.hit);

            if (vec_all_eq(hit, boolZero))
            {
                continue;
			}

			anyHit = vec_or(hit, anyHit);

			distance = vec_and(ray->start->distance2(*reflectionStart), hit);

			vector bool int distCmp = vec_and(vec_cmplt(distance, 
				minDistance), hit);

			vector int iSplat = splatScalar(i);

			minIndex = vec_sel(minIndex, iSplat, distCmp);

			minDistance = vec_sel(minDistance, distance, distCmp);

			VertexGroup* refStart = intersection.reflection->start;

			refStart->x = vec_sel(refStart->x, reflectionStart->x, distCmp);
            refStart->y = vec_sel(refStart->y, reflectionStart->y, distCmp);
            refStart->z = vec_sel(refStart->z, reflectionStart->z, distCmp);
	    }

		intersection.hit = vec_and(anyHit, intersection.hit);

    	if (vec_any_gt(intersection.hit, boolZero))
	    {
			minIndexVec->vec = minIndex;

			intersection.minIndex = minIndexVec;

	        int m1 = minIndexVec->points[0];
    	    int m2 = minIndexVec->points[1];
        	int m3 = minIndexVec->points[2];
	        int m4 = minIndexVec->points[3];

			vector float _13x = vec_mergeh(getPt(m1, 1), getPt(m3, 1));
			vector float _13y = vec_mergeh(getPt(m1, 2), getPt(m3, 2));
			vector float _13z = vec_mergeh(getPt(m1, 3), getPt(m3, 3));

			vector float _24x = vec_mergeh(getPt(m2, 1), getPt(m4, 1));
			vector float _24y = vec_mergeh(getPt(m2, 2), getPt(m4, 2));
			vector float _24z = vec_mergeh(getPt(m2, 3), getPt(m4, 3));

			centers->x = vec_mergeh(_13x, _24x);
			centers->y = vec_mergel(_13y, _24y);
			centers->z = vec_mergeh(_13z, _24z);

	        object->getReflection(ray, intersection.reflection, 
				intersection.normal, centers);
	    }
	}

    ColorGroup* surfaceColor(Intersection &intersection)
    {
		surfaceColorGroup->zero();

		for (int i = 0; i < getLightsSize(); i++)
		{
			Light* light = getLight(i);

			updateSurfaceColor(*surfaceColorGroup, light, intersection);
		}

		return surfaceColorGroup;
	}

	void updateSurfaceColor(ColorGroup& surfaceColor, Light* light, 
		Intersection& intersection)
	{
		VectorSInt* minIndex = intersection.minIndex;

		VertexGroup* pos = light->pos;
		VertexGroup* start = intersection.reflection->start;

		lightV->x = vec_sub(pos->x, start->x);
		lightV->y = vec_sub(pos->y, start->y);
		lightV->z = vec_sub(pos->z, start->z);

		lightV->normalize();

/*        Ray lightRay;
        Intersection lightIntersection;

        lightRay.start->copy(*intersection.reflection->start);
        lightRay.direction->copy(*lightV);

		getIntersection(&lightRay, lightIntersection);

		vector bool int lightObstructed = 
			vec_nor(lightIntersection.hit, lightIntersection.hit);

		if (vec_all_ne(lightIntersection.hit, boolZero()))
		{
			return;
		}*/

		vector float negZero = negativeZero();
		vector float epsilon = vectorEpsilon();
        
		vector float intensityDiffusion = lightV->dot(*intersection.normal);

		if (vec_all_lt(intensityDiffusion, epsilon))
		{
			// the light and the normal are perpindicular - or worse (?)
			return;
		}

		vector bool int intenseGtZero = vec_cmpgt(intensityDiffusion, epsilon);

        vector float ldot = lightV->dot(*intersection.reflection->direction);
		vector bool int ldotGtZero = vec_cmpgt(ldot, negativeZero());

		vector float intensityReflection = vec_and(pow(ldot, 
			light->shininess), ldotGtZero);


		int m1 = minIndex->points[0];
		int m2 = minIndex->points[1];
		int m3 = minIndex->points[2];
		int m4 = minIndex->points[3];

		vector float spec = vec_mergeh(vec_mergeh(getSpec(m1), getSpec(m3)), 
			vec_mergeh(getSpec(m2), getSpec(m4)));

        vector float specResult = vec_madd(intensityReflection,
            spec, negZero);

		vector float _13r = vec_mergeh(getDiff(m1, 1), getDiff(m3, 1));
		vector float _13g = vec_mergeh(getDiff(m1, 2), getDiff(m3, 2));
		vector float _13b = vec_mergeh(getDiff(m1, 3), getDiff(m3, 3));
		
		vector float _24r = vec_mergeh(getDiff(m2, 1), getDiff(m4, 1));
		vector float _24g = vec_mergeh(getDiff(m2, 2), getDiff(m4, 2));
		vector float _24b = vec_mergeh(getDiff(m2, 3), getDiff(m4, 3));

		surfaceColor.r = vec_add(vec_and(vec_madd(intensityDiffusion, 
			vec_madd(vec_mergeh(_13r, _24r), light->rgb->r, negZero), 
			specResult), intenseGtZero), surfaceColor.r);
		surfaceColor.g = vec_add(vec_and(vec_madd(intensityDiffusion, 
			vec_madd(vec_mergeh(_13g, _24g), light->rgb->g, negZero), 
			specResult), intenseGtZero), surfaceColor.g);
		surfaceColor.b = vec_add(vec_and(vec_madd(intensityDiffusion, 
			vec_madd(vec_mergeh(_13b, _24b), light->rgb->b, negZero), 
			specResult), intenseGtZero), surfaceColor.b);
    }

    ColorGroup* getAmbientColor()
    {
		return ambientColor;
    }
};

#endif

