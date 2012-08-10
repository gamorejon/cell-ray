#ifndef RAYTRACER_H
#define RAYTRACER_H

class Raytracer
{
private:
	inline float getRef(Scene* scene, Intersection& intersection, int _index)
	{
	    // int index = vec_extract(intersection.minIndex->vec, _index);
	    int index = intersection.minIndex.points[_index];

    	if (index == -1)
	    {
	        return 0.0f;
	    }
	    else
	    {
    	    return scene->getSceneObject(index)->getReflectiveScalar();
	    }
	}
public:
	
	Raytracer()
	{

	}
	
	void raytrace(Ray* ray, volatile ColorGroup* color, Scene* scene,
	    RayTraceParameters* params, Intersection& intersection)
	{
	    vector float depth = splatScalar(-0.0f);
	    vector float increment = splatScalar(1.0f);
	
	    color->zero();
	
	    vector float maxReflect = 
			splatScalar((float)params->maxReflectionDepth);

		vector float currentReflection = increment;
	
    	ColorGroup* ambient = scene->getAmbientColor();
	
    	while (vec_all_lt(depth, maxReflect) && 
//			vec_any_gt(currentReflection, vectorEpsilon()) &&
	        !(vec_all_gt(color->r, increment) && 
			  vec_all_gt(color->g, increment) &&
              vec_all_gt(color->b, increment)))
	    {
    	    scene->getIntersection(ray, intersection);

    	    if (vec_any_ne(intersection.hit, boolZero()))
        	{
	            ColorGroup* surface = scene->surfaceColor(intersection);

    	        vector float reflectiveVector = (vector float)
        	        {getRef(scene, intersection, 0), 
				     getRef(scene, intersection, 1),
            	     getRef(scene, intersection, 2), 
					 getRef(scene, intersection, 3)};


    	        color->addColor(*surface, currentReflection);
        	    color->addColor(*ambient, vec_and(intersection.hit, 
					currentReflection));

                currentReflection = vec_madd(currentReflection,
                    reflectiveVector, negativeZero());
	        }	
    	    else
        	{
	            break;
    	    }

        	ray->start     = intersection.reflection.start;
    	    ray->direction = intersection.reflection.direction;
	
        	depth = vec_add(depth, vec_and(intersection.hit, increment));
	    }
	}
};

#endif

