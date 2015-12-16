#ifndef HEADER_RAY
#define HEADER_RAY
#include "config.h"
#define INTERSECT_EPSILON 0.0000000001
#define SCREEN_ALIGN_EPSILON 0.0000001
#define ANGLE_CULL_EPSILON 0.0000001 
#include<vector>
#include<cmath>//For sqrt

struct rgb_colour {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};


class colour {
	public:
	double red;
	double green;
	double blue;
	double specular;
	colour(): red(0), green(0), blue(0) , specular(0.0) {}
	colour(double nr, double ng, double nb):
		red(nr), green(ng), blue(nb), specular(0.0) {}
	colour(double nr, double ng, double nb, double ns):
		red(nr), green(ng), blue(nb) {
		specular = ns > 1.0 ? 1.0 : ns;
	}
	void to_rbg(rgb_colour *c) {
		c->red   = red   > 255.0 ? 255 : (unsigned char) red;
		c->green = green > 255.0 ? 255 : (unsigned char) green;
		c->blue  = blue  > 255.0 ? 255 : (unsigned char) blue;
	}

	colour operator+(const colour &c) const {
		return colour(red + c.red, green + c.green, blue + c.blue);
	}
	colour operator*(const colour &c) const {
		return colour((red * c.red)/255.0,\
			(green * c.green)/255.0,\
			(blue * c.blue)/255.0);
	}
	colour operator/(const double d) const {
		return colour(red/d, green/d, blue/d);
	}
	colour operator*(const double d) const {
		return colour(red*d, green*d, blue*d);
	}
	void operator+=(const colour &c) {
		red += c.red;
		green += c.green;
		blue += c.blue;
	}
};


class vector3 {
	public:
	double x, y, z;
	
	vector3(): x(0), y(0), z(0) {}

	vector3(double nx, double ny, double nz): x(nx), y(ny), z(nz) {}

	vector3 operator-() const{
		return vector3(-x, -y, -z);
	}

	void Minv() {
		x = -x, y = -y, z = -z;
	}
	
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
