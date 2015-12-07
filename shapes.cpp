#include<iostream>
#define INTERSECT_EPSILON 0.000001


//Will be moved to header file
class vector3 {
	public:
	double x, y, z;
	
	vector3(): x(0), y(0), z(0) {}

	vector3(double nx, double ny, double nz): x(nx), y(ny), z(nz) {}
	
	/*
	vector3(const vector3 &v) {
		x = v.x, y = v.y, z = v.z;
	}
	*/

	vector3& operator+(const vector3 &v) const {
		return *(new vector3(x+v.x, y+v.y, z+v.z));
	}

	vector3& operator-(const vector3 &v) const {
		return *(new vector3(x-v.x, y-v.y, z-v.z));
	}

	vector3& operator+=(const vector3 &v) {
		x += v.x, y += v.y, z += v.z;
		return *this;
	}

	double dot(const vector3 &v) const {
		return x*v.x + y*v.y + z*v.z;
	}

	vector3& cross(const vector3 &v) const {
		double sx = y*v.z - z*v.y;
		double sy = z*v.x - x*v.z;
		double sz = x*v.y - y*v.x;
		return *(new vector3(sx, sy, sz));
	}
};
vector3& operator*(double s, const vector3 &v) {
	return *(new vector3(s*v.x, s*v.y, s*v.z));
}
std::ostream& operator<<(std::ostream &out, vector3 &v) {
	out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return out;
}
const static vector3 VECZERO = vector3(0, 0, 0);

struct ray {
	vector3 dir;
	vector3 origin;
};

class shape {
	public:
	virtual vector3* intersect(ray r)=0;
};

//Triangle with verticies pos, pos + p and pos + q.
class triangle: public shape {
	vector3 p, q, normal;
	public:
	vector3 pos;
	triangle(vector3 np, vector3 nq, vector3 npos=vector3(0, 0, 0)): p(np), q(nq), pos(npos) {
		normal = p.cross(q);
	}
	
	//Moller-Trumbore intersection algorithm
	vector3* intersect(ray r) {
		vector3 vec, T;
		double det, u, v, t;
		vec = r.dir.cross(q);
		det = p.dot(vec);
		
		//If det is zero then ray in plane of triangle
		if(det < INTERSECT_EPSILON && det > -INTERSECT_EPSILON) return NULL;
		//Get ray from pos to ray origin
		T = r.origin - pos;
		u = T.dot(vec)/det;
		//Test u
		if(u < 0.0 || u > 1.0) return NULL;
		//and v (+u)
		vec = T.cross(p);
		v = r.dir.dot(vec)/det;
		if(v < 0.0 || v + u > 1.0) return NULL;
		//Intersect at origin + t*dir
		t = q.dot(vec)/det;
		//Check we're not behind the ray origin
		if(t < INTERSECT_EPSILON) return NULL;
		//Return intersect point
		return &(r.origin + t*r.dir);
	}
};

int main() {
	triangle t = triangle(vector3(1, 0, 0), vector3(0, 1, 0));
	shape &s = t;
	ray r = {vector3(0, 0, 1), vector3(0.5, 0.4, -1.0)};
	vector3 *v = s.intersect(r);

	if(v != NULL) std::cout << *v << std::endl;
	else std::cout << "NULL" << std::endl;
	return 0;
}
