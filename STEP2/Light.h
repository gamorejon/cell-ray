#ifndef LIGHT_H
#define LIGHT_H

struct Light
{
    VertexGroup pos;
	Ray ray;
	Intersection intersection;
    float shininess;
	ColorGroup rgb;

	Light()
	{
		shininess = 0.0f;
	}

    Light(Vertex* _pos, float _shininess, float _r, float _g, float _b)
    {
		pos = VertexGroup(const_cast<const Vertex&>(*_pos));
		shininess = _shininess;

		Color color __attribute__ ((aligned (16))) = Color(_r, _g, _b);

		rgb = ColorGroup(color);

		ray = Ray();

		intersection = Intersection();

		delete _pos;
    }

    ~Light()
    {
//		delete rgb;
//		delete pos;
//		delete ray;
//		delete intersection;
    }

	Light& operator=(const Light& o)
	{
		pos = o.pos;
		ray = o.ray;
		intersection = o.intersection;
		shininess = o.shininess;
		rgb = o.rgb;

		return *this;
	}
};

#endif

