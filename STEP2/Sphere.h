#ifndef SPHERE_H
#define SPHERE_H

#include "SceneObject.h"
#include "AltivecUtils.h"

class Sphere : public SceneObject
{
private:
    vector float radius2;

public:
    Sphere(float _radius, Vertex* _center, float _reflective,
    	Color* _diffusionColor, float _specularReflection) :
    		SceneObject(_center, _reflective, _diffusionColor,
    			_specularReflection)
    {
		radius2 = splatScalar(_radius * _radius);
    }

	~Sphere()
	{
	}

    vec_bint4 isIntersected(Ray *ray, VertexGroup* reflectionStart, vec_bint4
		hit)
    {
		reflectionStart->x = vec_sub(center.x, ray->start.x);
		reflectionStart->y = vec_sub(center.y, ray->start.y);
		reflectionStart->z = vec_sub(center.z, ray->start.z);

		vector float rayDotCenter = ray->direction.dot(*reflectionStart);

		vector float epsilon = vectorEpsilon();
		vec_bint4 zero = boolZero();

		hit = vec_and(hit, vec_cmpgt(rayDotCenter, epsilon));

/*		if (vec_all_eq(hit, zero))
		{
			return hit;
		}*/

		vector float rcDotRc = reflectionStart->dot(*reflectionStart);

		vector float disc = vec_sub(radius2, rcDotRc);

		disc = vec_madd(rayDotCenter, rayDotCenter, disc);

		hit = vec_and(hit, vec_cmpgt(disc, epsilon));

		if (vec_all_eq(hit, zero))
		{
			return hit;
		}

        vector float distance = vec_and(vec_sub(rayDotCenter, sqrt(disc)), hit);

		reflectionStart->x = vec_madd(ray->direction.x, distance, 
			ray->start.x);
		reflectionStart->y = vec_madd(ray->direction.y, distance,
			ray->start.y);
		reflectionStart->z = vec_madd(ray->direction.z, distance,
			ray->start.z);

		return hit;
	}

	void getReflection(Ray* ray, Ray* reflection, VertexGroup* normal,
		VertexGroup* centers)
	{
		normal->x = vec_sub(reflection->start.x, centers->x);
		normal->y = vec_sub(reflection->start.y, centers->y);
		normal->z = vec_sub(reflection->start.z, centers->z); 

		normal->normalize();

		vector float negZero = negativeZero();

		vector float two = splatScalar(2.0f);

		vector float rayDotNormal = ray->direction.dot(*normal);

		rayDotNormal = vec_madd(two, rayDotNormal, negZero);

		vector float tempX = vec_madd(normal->x, rayDotNormal, negZero);
		vector float tempY = vec_madd(normal->y, rayDotNormal, negZero);
		vector float tempZ = vec_madd(normal->z, rayDotNormal, negZero);

		reflection->direction.x = vec_sub(ray->direction.x, tempX);
		reflection->direction.y = vec_sub(ray->direction.y, tempY);
		reflection->direction.z = vec_sub(ray->direction.z, tempZ);

		reflection->direction.normalize();
    }
};

#endif

