#ifndef LIGHT_H
#define LIGHT_H

struct Light
{
    VertexGroup* pos;
	Ray* ray;
	Intersection* intersection;
    float shininess;
	ColorGroup* rgb;

    Light(Vertex* _pos, float _shininess, float _r, float _g, float _b)
    {
		pos = new VertexGroup(*_pos);
		shininess = _shininess;

		Color color __attribute__ ((aligned (16))) = Color(_r, _g, _b);

		rgb = new ColorGroup(color);

		ray = new Ray();

		intersection = new Intersection();

		delete _pos;
    }

    ~Light()
    {
//		delete rgb;
//		delete pos;
//		delete ray;
//		delete intersection;
    }
};

#endif

