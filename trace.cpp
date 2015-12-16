#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<vector>
#include<cmath>
#include "ray.h"
#define SPECULAR_CUTOFF 0.7
#define DBL_INFINITY (1.0/0.0)
#define SCREEN_WIDTH ((double) (SCREEN_SIZE))
#define SCREEN_HEIGHT ((((double) (SCREEN_RESOLUTION_HEIGHT))/((double) (SCREEN_RESOLUTION_WIDTH))) * SCREEN_SIZE)

/* topl ------- 
 * |		|
 * |		|
 * |		|
 * |		|
 * botl ----- botr
 */
struct s_screen {
	vector3 eye;
	vector3 botl;
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

	s->botl   = centre - (zx_comp + zy_comp);
	s->topl   = centre + (zy_comp - zx_comp);
	s->botr   = centre + (zx_comp - zy_comp);
	s->eye    = eye.origin;
	s->height = SCREEN_RESOLUTION_HEIGHT;
	s->width  = SCREEN_RESOLUTION_WIDTH;
}


shape* trace_nearest(ray r, std::vector<shape*> &world, double &distance) {
	double closest_distance = DBL_INFINITY;
	double t;
	shape* closest_shape = NULL;
	std::vector<shape*>::iterator v = world.begin();
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
	shape* s = trace_nearest(r, w, t);
	if(s != NULL && t < (1.0+INTERSECT_EPSILON)) {
		*c = colour(0, 0, 0);
	} else {
		distsq = r.dir.lengthsq();
		strength = (distsq <= LIGHT_FALLOFF ? 1.0 : LIGHT_FALLOFF/(distsq));
		*c = l->col * strength;
	}
}



void trace(ray r, std::vector<shape*> &world, std::vector<light*> &lights, int depth, colour *c) {
	double t, light_cosine;
	vector3 hit_position, normal, inv_ray_dir, dir_to_light;
	colour tmpcol, diff_light_col = AMBIENT_COLOUR, spec_light_col = colour(0, 0, 0);
	std::vector<light*>::iterator liter;
	shape *intersect_shape;
	intersect_shape = trace_nearest(r, world, t);

	if(intersect_shape != NULL) {
		inv_ray_dir = -r.dir;
		liter = lights.begin();
		hit_position = r.origin + t*r.dir;
		normal = intersect_shape->get_normal(hit_position).normalise();
		if(inv_ray_dir.dot(normal) < 0.0) {
			normal.Minv();
		}
		

		while(liter != lights.end()) {
			dir_to_light = ((**liter).pos - hit_position).normalise();
			//Don't look for light begind the object
			light_cosine = dir_to_light.dot(normal);
			if(light_cosine > 0.0) {
				trace_light(hit_position, world, *liter, &tmpcol);
				diff_light_col += tmpcol*light_cosine;
				if(light_cosine > SPECULAR_CUTOFF) {
					spec_light_col += tmpcol*std::pow(light_cosine, SPECULAR_POWER);
				}
			}
			liter++;
		}
		*c = ((diff_light_col * intersect_shape->col) + (spec_light_col * intersect_shape->col.specular));
	} else {
		*c = BACKGROUND_COLOUR;
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
	colour col;
	int i, j;
	
	h_step = (s.botr - s.botl)/(s.width-1);
	v_step = (s.topl - s.botl)/(-s.height+1);
	#ifdef SUPER_SAMPLE
	int k, l;
	vector3 h_sample_step = h_step/SUPER_SAMPLE;
	vector3 v_sample_step = v_step/SUPER_SAMPLE;
	vector3 sample_pos;
	colour cuml;
	#endif
	
	for(j=0; j<s.height; j++) {
		pos = s.topl + j*v_step;
		for(i=0; i<s.width; i++) {
			#ifdef SUPER_SAMPLE
			cuml = colour(0, 0, 0);
			for(k=0; k<SUPER_SAMPLE; k++) {
				sample_pos = pos+k*v_sample_step;
				for(l=0; l<SUPER_SAMPLE; l++) {
					r.dir = sample_pos-s.eye;
					trace(r, world, lights, TRACE_DEPTH, &col);
					cuml += col/(SUPER_SAMPLE*SUPER_SAMPLE);
					sample_pos += h_sample_step;
				}
			}
			cuml.to_rbg(&c);
			#else
			r.dir = pos-s.eye;
			trace(r, world, lights, TRACE_DEPTH, &col);
			col.to_rbg(&c);
			#endif
			d = data + (j*s.width + i)*3;
			d[0] = c.red;
			d[1] = c.green;
			d[2] = c.blue;
			pos += h_step;
		}
	}
	return data;
}


int main() {
	char* data;
	ray eye;
	s_screen screen;
	eye.origin = EYE_POSITION;
	eye.dir = EYE_DIRECTION;
	gen_screen(eye, &screen);
	std::vector<shape*> *w = getWorld();
	std::vector<light*> *l = getLights();
	data = ray_trace(screen, *w, *l);
	write_pnm(data, screen.width, screen.height, stdout);
	free(data);
	freeWorld(w);
	freeLights(l);
	return 0;
}
