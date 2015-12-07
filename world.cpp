#include<iostream>
#include<vector>
#include "ray.h"

vector3 operator*(double s, const vector3 &v) {
	return vector3(s*v.x, s*v.y, s*v.z);
}

std::ostream& operator<<(std::ostream &out, const vector3 &v) {
	out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return out;
}


//Triangle with verticies pos, pos + p and pos + q.
class triangle: public shape {
	vector3 p, q, normal;	
	public:
	vector3 pos;
	triangle(vector3 np, vector3 nq, vector3 npos, colour &ncol):
		p(np), q(nq), pos(npos) {
		col = ncol;
		normal = p.cross(q);
	}
	
	vector3 get_normal(const vector3 ignore) const {
		return normal;
	}
	
	//Moller-Trumbore intersection algorithm
	double intersect(ray r) {
		vector3 vec, T;
		double det, u, v, t;
		vec = r.dir.cross(q);
		det = p.dot(vec);
		
		//If det is zero then ray in plane of triangle
		if(det < INTERSECT_EPSILON && det > -INTERSECT_EPSILON) return 0.0;
		//Get ray from pos to ray origin
		T = r.origin - pos;
		u = T.dot(vec)/det;
		//Test u
		if(u < 0.0 || u > 1.0) return 0.0;
		//and v (+u)
		vec = T.cross(p);
		v = r.dir.dot(vec)/det;
		if(v < 0.0 || v + u > 1.0) return 0.0;
		//Intersect at origin + t*dir
		t = q.dot(vec)/det;
		//Check we're not behind the ray origin
		if(t < INTERSECT_EPSILON) return 0.0;
		//Return intersect point
		//return &(r.origin + t*r.dir);
		//return ray intersect parameter: point = Origin + t*dir
		return t;
	}
};

std::vector<shape*>* getWorld() {
	std::vector<shape*> *w = new std::vector<shape*>();
	colour red  = colour((char)255, (char)255, (char)0);
	colour blue = colour((char)0, (char)0, (char)255);
	w->push_back((shape*) (new triangle(vector3(5, 0, 0), vector3(0, 5, 0), vector3(1, 1, 81), red)));
	//w->push_back((shape*) (new triangle(vector3(20, 0, 200), vector3(0, 20, 200), vector3(-10, -10, 200), blue)));
	w->push_back((shape*) (new triangle(vector3(10, -10, 20), vector3(10, 10, 20), vector3(0, 0, 100), blue)));
	w->push_back((shape*) (new triangle(vector3(10, 10, 20), vector3(-10, 10, 20), vector3(0, 0, 100), blue)));
	w->push_back((shape*) (new triangle(vector3(-10, 10, 20), vector3(-10, -10, 20), vector3(0, 0, 100), blue)));
	w->push_back((shape*) (new triangle(vector3(-10, -10, 20), vector3(10, -10, 20), vector3(0, 0, 100), blue)));
	return w;
}

std::vector<light*>* getLights() {
	std::vector<light*> *l = new std::vector<light*>();
	l->push_back(new light(vector3(3, 3, 50), colour(127, 255, 255)));
	return l;
}

/*
int main() {
	triangle t = triangle(vector3(1, 0, 0), vector3(0, 1, 0));
	shape &s = t;
	ray r = {vector3(0, 0, 1), vector3(0.5, 0.4, -1.0)};
	vector3 *v = s.intersect(r);

	if(v != NULL) std::cout << *v << std::endl;
	else std::cout << "NULL" << std::endl;
	return 0;
}
*/
