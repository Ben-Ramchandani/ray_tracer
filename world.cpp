#include<iostream>
#include<stdio.h>
#include<vector>
#include<cmath>
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
		//std::cout << "det = " << det << std::endl;
		//If det is zero then ray in plane of triangle
		if(det < INTERSECT_EPSILON && det > -INTERSECT_EPSILON) return 0.0;
		//Get ray from pos to ray origin
		T = r.origin - pos;
		u = T.dot(vec)/det;
		//Test u
		if(u < 0 || u > 1.0) return 0.0;
		//and v (+u)
		vec = T.cross(p);
		v = r.dir.dot(vec)/det;
		if(v < 0 || v + u > 1.0) return 0.0;
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

class shpere: public shape {
	public:
	vector3 pos;
	double radius;
	shpere(double nr, vector3 np, colour &ncol):
		pos(np), radius(nr) {
		col = ncol;
	}
	
	vector3 get_normal(const vector3 p) const {
		return p - pos;
	}

	double intersect(ray r) {
		double a, b, c, d, dsq, s1, s2;
		c = r.origin.dot(r.origin) + pos.dot(pos) - 2*r.origin.dot(pos) - radius*radius;
		b = 2 * r.dir.dot(r.origin - pos);
		a = r.dir.dot(r.dir);
		if(a < INTERSECT_EPSILON) {
			fprintf(stderr, "Warn: sphere::intersect: a < ITERSECT_EPSILON\n");
			return 0.0;
		}
		dsq = b*b - 4*a*c;
		if(dsq <= 0) {
			return 0;
		}
		d = std::sqrt(dsq);
		s1 = (-b+d)/(2*a);
		s2 = (-b-d)/(2*a);
		if(s1 < s2) {
			if(s1 > INTERSECT_EPSILON) {
				return s1;
			}
		}
		if(s2 > INTERSECT_EPSILON) {
			return s2;
		} else {
			return s1 > INTERSECT_EPSILON ? s1 : 0.0;
		}
	}
};

		

std::vector<shape*>* getWorld() {
	std::vector<shape*> *w = new std::vector<shape*>();
	colour red  = colour(255, 0, 0, 1.0);
	colour blue = colour(0, 0, 255, 1.0);
	//w->push_back((shape*) (new triangle(vector3(5, 0, 0), vector3(0, 5, 0), vector3(1, 1, 81), red)));
	//w->push_back((shape*) (new triangle(vector3(20, 0, 200), vector3(0, 20, 200), vector3(-10, -10, 200), blue)));
	
	w->push_back((shape*) (new triangle(vector3(10, -10, 20), vector3(10, 10, 20), vector3(0, 0, 30), blue)));
	w->push_back((shape*) (new triangle(vector3(10, 10, 20), vector3(-10, 10, 20), vector3(0, 0, 30), blue)));
	w->push_back((shape*) (new triangle(vector3(-10, 10, 20), vector3(-10, -10, 20), vector3(0, 0, 30), blue)));
	w->push_back((shape*) (new triangle(vector3(-10, -10, 20), vector3(10, -10, 20), vector3(0, 0, 30), blue)));
	w->push_back((shape*) (new shpere(1, vector3(5, 3, 28), red)));
	
	//w->push_back((shape*) (new shpere(10, vector3(0, 0, 30), red)));
	return w;
}

void freeWorld(std::vector<shape*> *w) {
	while(w->size() > 0) {
		delete (w->back());
		w->pop_back();
	}
	delete w;
}

std::vector<light*>* getLights() {
	std::vector<light*> *l = new std::vector<light*>();
	l->push_back(new light(vector3(10, 3, 0), colour(255, 255, 255)));
	l->push_back(new light(vector3(13, 20, 5), colour(255, 255, 255)));
	return l;
}

void freeLights(std::vector<light*> *l) {
	while(l->size() > 0) {
		delete (l->back());
		l->pop_back();
	}
	delete l;
}

