#ifndef HEADER_RAY
#define HEADER_RAY
#define INTERSECT_EPSILON 0.0000000001
#define TRACE_DEPTH 9000
#define LIGHT_FALLOFF 600
#define SCREEN_ALIGN_EPSILON 0.0000001
#define SCREEN_RESOLUTION_WIDTH 1920
#define SCREEN_RESOLUTION_HEIGHT 1080
#define SCREEN_DISTANCE 5
#define SCREEN_SIZE_SCALE (1.0/1000.0)
#define EYE_POSITION (vector3(0, 0, 0)) 
#define EYE_DIRECTION (vector3(0, 0, 1)) 
#define ANGLE_CULL_EPSILON 100000 
#include<vector>
#include<iostream>
#include<stdio.h>
#include<cmath>

struct rgb_colour {
	char red;
	char green;
	char blue;
};


class colour {
	public:
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	colour(): red(0), green(0), blue(0) {}
	colour(unsigned char nr, unsigned char ng, unsigned char nb): red(nr), green(ng), blue(nb) {}
	void to_rbg(rgb_colour *c) {
		c->red = red;
		c->green = green;
		c->blue = blue;
	}

	colour operator+(const colour &c) const {
		int nr = (int)red + (int)c.red;
		int ng = (int)green + (int)c.green;
		int nb = (int)blue + (int)c.blue;
		return colour(nr>255 ? 255 : nr, ng>255 ? 255 : ng, nb>255 ? 255 : nb);
	}
	colour operator*(const colour &c) const {
		return colour(((int)red * (int)c.red)/255,\
			((int)green * (int)c.green)/255,\
			((int)blue * (int)c.blue)/255);
	}
	colour operator/(const double d) const {
		return colour(red/d, green/d, blue/d);
	}
	colour operator*(const double d) const {
		if(d > 1.0)
			fprintf(stderr, "Warning: colour::operator*: d > 1 (Overflow?)");

		return colour(red*d, green*d, blue*d);
	}
};


class vector3 {
	public:
	double x, y, z;
	
	vector3(): x(0), y(0), z(0) {}

	vector3(double nx, double ny, double nz): x(nx), y(ny), z(nz) {}
	
	vector3 operator+(const vector3 &v) const {
		return vector3(x+v.x, y+v.y, z+v.z);
	}

	vector3 operator-(const vector3 &v) const {
		return vector3(x-v.x, y-v.y, z-v.z);
	}

	vector3 operator+=(const vector3 &v) {
		x += v.x, y += v.y, z += v.z;
		return *this;
	}

	vector3 operator/(const double s) const {
		return vector3(x/s, y/s, z/s);
	}

	double dot(const vector3 &v) const {
		return x*v.x + y*v.y + z*v.z;
	}

	vector3 cross(const vector3 &v) const {
		double sx = y*v.z - z*v.y;
		double sy = z*v.x - x*v.z;
		double sz = x*v.y - y*v.x;
		return vector3(sx, sy, sz);
	}

	double lengthsq() const {
		return x*x + y*y + z*z;
	}

	double length() const {
		return std::sqrt(lengthsq());
	}

	vector3 normalise() const {
		double len = length();
		return vector3(x/len, y/len, z/len);
	}
};



struct ray {
	vector3 dir;
	vector3 origin;
};

class shape {
	public:
	virtual double intersect(ray r)=0;
	virtual vector3 get_normal(const vector3 pos) const=0;
	colour col;
	virtual ~shape() {}
};

class light {
	public:
	vector3 pos;
	colour col;
	light(vector3 npos, colour nc=colour(255, 255, 255)): pos(npos), col(nc) {}
};


//In world.cpp
std::vector<shape*>* getWorld();
std::vector<light*>* getLights();
void freeWorld(std::vector<shape*> *w);
void freeLights(std::vector<light*> *l);
vector3 operator*(double s, const vector3 &v);

std::ostream& operator<<(std::ostream &out, const vector3 &v);

//In pnmout.cpp
void write_pnm(char*, int, int, FILE*);
#endif
