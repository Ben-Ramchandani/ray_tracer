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
	triangle(vector3 np, vector3 nq, vector3 npos, colour &ncol, surface &s):
		p(np), q(nq), pos(npos) {
		col = ncol;
		surf = s;
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
		//return ray intersect parameter: point = Origin + t*dir
		return t;
	}
};

class shpere: public shape {
	public:
	vector3 pos;
	double radius;
	shpere(double nr, vector3 np, colour &ncol, surface &s):
		pos(np), radius(nr){
		col = ncol;
		surf = s;
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

class plane: public shape {
	private:
	vector3 normal;
	public:
	vector3 pos;
	
	plane(vector3 np, vector3 nn, colour &ncol, surface &ns):
		pos(np) {
		normal = nn.normalise();
		col = ncol;
		surf = ns;
	}

	vector3 get_normal(const vector3 ignore) const {
		return normal;
	}

	double intersect(ray r) {
		vector3 n;
		double dot_norm = r.dir.dot(normal);
		if(dot_norm < 0.0) {
			n = -normal;
			dot_norm = -dot_norm;
		} else {
			n = normal;
		}

		if(dot_norm < INTERSECT_EPSILON) {
			//Line is parallel to plane
			return 0.0;
		}

		/* Ray: p = t*r + r_0
		 * Plane (p-p_0).n = 0
		 * => t = ((p_0 - r_0).n)/(r.n)
		 */
		double t = ((pos - r.origin).dot(n))/(dot_norm);
		if(t < INTERSECT_EPSILON) {
			return 0.0;
		} else {
			return t;
		}
	}
};




std::vector<shape*>* getWorld() {
	std::vector<shape*> *w = new std::vector<shape*>();
	
	colour red  = colour(255, 0, 0, 1.0);
	colour blue = colour(0, 0, 255, 1.0);
	//colour green = colour(0, 255.0, 0, 1.0);
	//colour silver = colour(211, 211, 211);
	//surface matt;
	surface shiny;
	//matt.reflection = 0.0;
	//matt.specular = 0.0;
	//matt.diffusion = 1.0;
	shiny.reflection = 0.8;
	shiny.specular = 1.0;
	shiny.diffusion = 0.9;

	//Pyramid and ball
	
	w->push_back((shape*) (new triangle(vector3(10, -10, 30), vector3(10, 10, 30), vector3(0, 0, 20), blue, shiny)));
	w->push_back((shape*) (new triangle(vector3(10, 10, 30), vector3(-10, 10, 30), vector3(0, 0, 20), blue, shiny)));
	w->push_back((shape*) (new triangle(vector3(-10, 10, 30), vector3(-10, -10, 30), vector3(0, 0, 20), blue, shiny)));
	w->push_back((shape*) (new triangle(vector3(-10, -10, 30), vector3(10, -10, 30), vector3(0, 0, 20), blue, shiny)));
	w->push_back((shape*) (new shpere(1, vector3(0, 3, 25), red, shiny)));
	

	//A room
	/*
	w->push_back((shape*) (new plane(vector3(0, -10, 0), vector3(0, 1, 0), red, matt)));
	w->push_back((shape*) (new plane(vector3(0, 10, 0), vector3(0, -1, 0), red, matt)));
	w->push_back((shape*) (new plane(vector3(0, 0, 30), vector3(0, 0, 1), blue, matt)));
	w->push_back((shape*) (new plane(vector3(10, 0, 0), vector3(1, 0, 0), green, matt)));
	w->push_back((shape*) (new plane(vector3(-10, 0, 0), vector3(-1, 0, 0), green, matt)));
	w->push_back((shape*) (new shpere(3, vector3(0, 0, 20), silver, shiny)));
	*/
	return w;

}

std::vector<shape*>* sphere_world(int n_spheres) {
	surface shiny;
	shiny.reflection = 0.8;
	shiny.specular = 1.0;
	shiny.diffusion = 0.9;
	std::vector<shape*> *w = new std::vector<shape*>();
	int i, j, k;
	for(i=0; i<3; i++) {
	for(j=0; j<3; j++) {
	for(k=0; k<3; k++) {
		if(n_spheres > 0) {
			colour c = colour(i*127.5, 255-j*127.5, ((int) (127.5+(k*127.5)))%382);
			w->push_back((shape*) (new shpere(1, vector3(i*4-5.5, j*4-6, k*4+14), c, shiny)));
			n_spheres -= 1;
		}
	}
	}
	}
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
	//l->push_back(new light(vector3(-5, 3, 2), colour(255, 255, 255)));
	l->push_back(new light(vector3(15, 15, 5), colour(255, 255, 255)));
	return l;
}

void freeLights(std::vector<light*> *l) {
	while(l->size() > 0) {
		delete (l->back());
		l->pop_back();
	}
	delete l;
}

