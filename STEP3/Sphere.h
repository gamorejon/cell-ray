#ifndef SPHERE_H
#define SPHERE_H

#include "SceneObject.h"
#include "AltivecUtils.h"

class Sphere : public SceneObject
{
private:
    vector float radius2;
	vector float four;
	vector float oneHalf;

public:
    Sphere(float _radius, Vertex* _center, float _reflective,
    	Color* _diffusionColor, float _specularReflection) :
    		SceneObject(_center, _reflective, _diffusionColor,
    			_specularReflection)
    {
		radius2 = splatScalar(_radius * _radius);

		four = splatScalar(4.0f);
		oneHalf = splatScalar(0.5f);
    }

	~Sphere()
	{
	}

	/** 
     * These equations come from plugging in the parametric representation
     * of a line into the equation of a sphere and solving for the parameter.
     * After doing some sanity checking, the parameter is plugged back into
     * the equation of a line to find the intersection point.
     *
     * See http://local.wasp.uwa.edu.au/~pbourke/geometry/sphereline/ for more
     * info.
     */
    vec_bint4 isIntersected(Ray *ray, VertexGroup* reflectionStart, 
		Intersection& intersection)
    {
        vector float epsilon = vectorEpsilon();
        vec_bint4 zero = boolZero();

		vec_bint4 hit = intersection.hit;

		// use this for direction to the center
		reflectionStart->x = vec_sub(center.x, ray->start.x);
		reflectionStart->y = vec_sub(center.y, ray->start.y);
		reflectionStart->z = vec_sub(center.z, ray->start.z);

		// b in the quadratic equation (a is 1 since ray->start is normalized)
		// this also represents the parameter of the line for the closest point
		// to the sphere.  If the distance from ray(b) to the center is greater
		// than the radius of the sphere, the point does not hit.  Note that
		// the b used in the quadratic equation is actually twice as large as
		// this, but the factor of two ends up cancelling out completely and
		// is not needed.
		vector float b = ray->direction.dot(*reflectionStart);
		
		// if b is negative, there is no hope for the param in the quad eq
		// for the intersection point to be positive (in front of the ray). 
		// 	Therefore, we can exit out now.
		hit = vec_and(hit, vec_cmpgt(b, epsilon));

		// the norm of the relative direction squared...
		vector float dirDotDir = reflectionStart->dot(*reflectionStart);

		// minus r^2 is c in the quadratic equation.  As with b, the term of
		// four used in the quadratic equation ends up cancelling out.
		vector float c = vec_sub(radius2, dirDotDir);

		c = vec_madd(splatScalar(1.0f), c, negativeZero());

		// (2b)^2 + 4ac = 4(b^2 + c), sqrt(4(b^2 + c)) = 2*sqrt(...)
		vector float discriminant = vec_and(vec_madd(b, b, c), hit);

		// if the discriminant is less than zero, there is no intersection
		hit = vec_and(hit, vec_cmpgt(discriminant, epsilon));

		// take the branch hit now to avoid the costly sqrt
		if (vec_all_eq(hit, zero))
		{
			return hit;
		}

		// we now have the distance parameter
        intersection.distance = vec_sub(b, sqrt(discriminant));

		// and the intersection
		reflectionStart->x = vec_madd(ray->direction.x, intersection.distance, 
			ray->start.x);
		reflectionStart->y = vec_madd(ray->direction.y, intersection.distance,
			ray->start.y);
		reflectionStart->z = vec_madd(ray->direction.z, intersection.distance,
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

