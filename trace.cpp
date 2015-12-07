#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<vector>
#include "ray.h"
#define TRACE_DEPTH 9000
#define DBL_INFINITY (1.0/0.0)
#define LIGHT_FALLOFF 2000
#define SCREEN_ALIGN_EPSILON 0.0000001
#define SCREEN_RESOLUTION_WIDTH 1920
#define SCREEN_RESOLUTION_HEIGHT 1080
#define SCREEN_DISTANCE 10
#define SCREEN_SIZE_SCALE (1.0/1000.0)
#define SCREEN_WIDTH (((double) (SCREEN_RESOLUTION_WIDTH)) * SCREEN_SIZE_SCALE)
#define SCREEN_HEIGHT (((double) (SCREEN_RESOLUTION_HEIGHT)) * SCREEN_SIZE_SCALE)
#define EYE_POSITION (vector3(0, 0, 0))
#define EYE_DIRECTION (vector3(0, 0, 1))

/* topl ------- 
 * |		|
 * |		|
 * |		|
 * |		|
 * base ----- botr
 */
struct s_screen {
	vector3 eye;
	vector3 base;
	vector3 topl;
	vector3 botr;
	int height;
	int width;
};

void gen_screen(ray eye, s_screen *s) {
	vector3 planevec, zx_comp, zy_comp;
	vector3 viewdir = eye.dir.normalise();
	vector3 centre = eye.origin + SCREEN_DISTANCE*viewdir;
	//Form vector in eye.dir-y plane, then find z-x screen component
	//if eye.dir ~= y then z-x component is x
	if(viewdir.x < SCREEN_ALIGN_EPSILON && viewdir.x > -SCREEN_ALIGN_EPSILON &&\
			 viewdir.z < SCREEN_ALIGN_EPSILON && viewdir.z > -SCREEN_ALIGN_EPSILON) {
		zx_comp = SCREEN_WIDTH * vector3(1, 0, 0);
	} else {
		planevec = vector3(viewdir.x, viewdir.y + 1, viewdir.z);
		zx_comp = SCREEN_WIDTH * planevec.cross(viewdir).normalise();
	}
	//Then the z-y component
	if(viewdir.y < SCREEN_ALIGN_EPSILON && viewdir.y > -SCREEN_ALIGN_EPSILON &&\
			 viewdir.z < SCREEN_ALIGN_EPSILON && viewdir.z > -SCREEN_ALIGN_EPSILON) {
		zy_comp = SCREEN_HEIGHT * vector3(0, 1, 0);
	} else {
		planevec = vector3(viewdir.x + 1, viewdir.y, viewdir.z);
		zy_comp = -SCREEN_HEIGHT * planevec.cross(viewdir).normalise();
	}

	s->base   = centre - (zx_comp + zy_comp);
	s->topl   = centre + (zy_comp - zx_comp);
	s->botr   = centre + (zx_comp - zy_comp);
	s->eye    = eye.origin;
	s->height = SCREEN_RESOLUTION_HEIGHT;
	s->width  = SCREEN_RESOLUTION_WIDTH;
}

/*
struct s_screen screen = {
	vector3( 0,  0, 0),
	vector3(-1, -1, 5),
	vector3(-1,  1, 5),
	vector3( 1, -1, 5),
	SCREEN_RESOLUTION,
	SCREEN_RESOLUTION
};
*/

shape* trace_nearest(ray r, std::vector<shape*> &world, double &distance) {
	double closest_distance = DBL_INFINITY;
	double t;
	std::vector<shape*>::iterator v = world.begin();
	shape* closest_shape = NULL;
	while(v != world.end()) {
		t = (**v).intersect(r);
		if(t != 0.0 && t < closest_distance) {
			closest_distance = t;
			closest_shape = *v;
		}
		v++;
	}
	distance = closest_distance;
	return closest_shape;
}

void trace_light(vector3 pos, std::vector<shape*> &w, light *l, colour *c) {
	ray r;
	double t, distsq, strength;
	r.origin = pos;
	r.dir = l->pos - pos;
	//Check for collisions before the light source
	//std::cout << "Tracing ray " << r.origin << ", " << r.dir << std::endl;
	shape* s = trace_nearest(r, w, t);
	if(s != NULL && t < (1.0+INTERSECT_EPSILON)) {
		//std::cout << "Light hit shape with t = " << t << std::endl;
		*c = colour();
	} else {
		distsq = r.dir.lengthsq();
		//std::cout << "Light unimpeded at distance " << distsq << std::endl;
		strength = (distsq <= LIGHT_FALLOFF ? 1.0 : LIGHT_FALLOFF/(distsq));
		*c = l->col * strength;
		//std::cout << "Strength is " << strength << std::endl;
	}
}


void trace(ray r, std::vector<shape*> &world, std::vector<light*> &lights, int depth, rgb_colour *c) {
	double t;
	bool hit_side, light_side;
	vector3 hit_position;
	colour tmpcol, lightcol = colour(32, 32, 32);
	std::vector<light*>::iterator liter;
	shape *intersect_shape;
	intersect_shape = trace_nearest(r, world, t);

	if(intersect_shape != NULL) {
		liter = lights.begin();
		hit_position = r.origin + t*r.dir;
		hit_side = (r.dir.dot(intersect_shape->get_normal(r.origin + t*r.dir)) > 0.0);
		while(liter != lights.end()) {
			//Don't look for light begind the object
			light_side = ((hit_position-(**liter).pos).dot(intersect_shape->get_normal(r.origin + t*r.dir))) > 0.0;
			if(light_side == hit_side) {
				trace_light(hit_position, world, *liter, &tmpcol);
			}
			liter++;
			lightcol = lightcol + tmpcol;
		}
		(lightcol * intersect_shape->col).to_rbg(c);
	} else {
		c->red = c->green = c->blue = (char)150;
	}
}

char* ray_trace(s_screen s, std::vector<shape*> &world, std::vector<light*> &lights) {
	vector3 h_step, v_step, pos;
	char *data = (char*) malloc(s.width*s.height*3);
	char *d;
	if(data == NULL) {
		std::cout << "Fatal: ray_trace: Memory allocation failed";
		exit(2);
	}
	ray r;
	r.origin = s.eye;
	rgb_colour c;
	int i, j;
	
	h_step = (s.botr - s.base)/(s.width-1);
	v_step = (s.topl - s.base)/(-s.height+1);
	pos = s.topl;
	
	for(j=0; j<s.height; j++) {
		for(i=0; i<s.width; i++) {
			r.dir = pos-s.eye;
			trace(r, world, lights, TRACE_DEPTH, &c);
			d = data + (j*s.width + i)*3;
			d[0] = c.red;
			d[1] = c.green;
			d[2] = c.blue;
			pos += h_step;
		}
		pos = s.topl + j*v_step;
	}
	return data;
}

void print_char_arr(char *arr, int length) {
	int i;
	for(i=0; i<length; i++) {
		printf("%d, ", arr[i]);
	}
	printf("\n");
}

int main() {
	char* data;
	ray eye;
	std::vector<shape*> *w = getWorld();
	std::vector<light*> *l = getLights();
	s_screen screen;
	eye.origin = EYE_POSITION;
	eye.dir = EYE_DIRECTION;
	gen_screen(eye, &screen);
	data = ray_trace(screen, *w, *l);
	//print_char_arr(data, 16*3);
	write_pnm(data, screen.width, screen.height, stdout);
	free(data);
	delete w;
	delete l;
	return 0;
}
