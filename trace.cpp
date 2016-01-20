#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<vector>
#include<cmath>
#include<ctime>
#include "ray.h"
#define TIME_TRACE
#define SPECULAR_CUTOFF 0.7
#define REFLECTION_CUTOFF 0.01
#define DBL_INFINITY (1.0/0.0)
#define SCREEN_WIDTH ((double) (SCREEN_SIZE))
#define SCREEN_HEIGHT ((((double) (SCREEN_RESOLUTION_HEIGHT))/((double) (SCREEN_RESOLUTION_WIDTH))) * SCREEN_SIZE)
//#define RUN_BENCHMARK

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



//Find the nearest intersecting shpae and return a pointer to it or NULL on no intersection.
shape* trace_nearest(ray &r, std::vector<shape*> &world, double &distance) {
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



bool trace_light(vector3 pos, std::vector<shape*> &w, light *l, colour *c) {
	ray r;
	double t, distsq, strength;
	r.origin = pos;
	r.dir = l->pos - pos;
	shape* s;
	#ifdef SHADOWS_ENABLE
	//Check for collisions before the light source
	s = trace_nearest(r, w, t);
	#else
	s = NULL;
	#endif
	
	if(s != NULL && t < (1.0+INTERSECT_EPSILON)) {
		//Light is blocked (shadow)
		return false;
	} else {
		distsq = r.dir.lengthsq();
		strength = (distsq <= LIGHT_FALLOFF ? 1.0 : LIGHT_FALLOFF/(distsq));
		*c = l->col * strength;
		return true;
	}
}



bool trace(ray &r, std::vector<shape*> &world, std::vector<light*> &lights, int depth, double strength, colour *c) {
	double t, light_cosine;
	vector3 hit_position, normal, inv_ray_dir, dir_to_light;
	colour tmpcol, diff_light_col = AMBIENT_COLOUR, spec_light_col = colour(0, 0, 0);
	std::vector<light*>::iterator liter;
	shape *intersect_shape;
	intersect_shape = trace_nearest(r, world, t);
	ray reflection;
	double reflection_cosine;

	if(intersect_shape != NULL) {
		inv_ray_dir = -r.dir;
		liter = lights.begin();
		hit_position = r.origin + t*r.dir;
		normal = intersect_shape->get_normal(hit_position).normalise();
		if(inv_ray_dir.dot(normal) < 0.0) {
			normal = -normal;
		}
		reflection.origin = hit_position;
		reflection.dir    = r.dir - 2*(r.dir.dot(normal))*normal;

		//Go through each light and find its contribution
		while(liter != lights.end()) {
			dir_to_light = ((**liter).pos - hit_position).normalise();
			//Don't look for light begind the object
			light_cosine = dir_to_light.dot(normal);
			if(light_cosine > 0.0) {
				if(trace_light(hit_position, world, *liter, &tmpcol)) {
					diff_light_col += tmpcol*light_cosine;
					reflection_cosine = reflection.dir.normalise().dot(dir_to_light);
					if(reflection_cosine > SPECULAR_CUTOFF) {
						spec_light_col += tmpcol*std::pow(reflection_cosine, SPECULAR_POWER);
					}
				}
			}
			liter++;
		}
		
		colour diffuse_component  = diff_light_col * intersect_shape->col * intersect_shape->surf.diffusion;
		colour specular_component = spec_light_col * intersect_shape->surf.specular;
		*c = diffuse_component + specular_component;

		#ifdef REFLECTION_ENABLE
		//Deal with reflection

		colour reflection_component;
		double reflection_strength = intersect_shape->surf.reflection * strength; 
		if(depth > 0 && (reflection_strength > REFLECTION_CUTOFF)) {
			colour reflection_colour;
			if(trace(reflection, world, lights, depth-1, reflection_strength, &reflection_colour)) {
				reflection_component = reflection_colour * intersect_shape->surf.reflection;// * intersect_shape->col;
				*c = *c + reflection_component;
			}
		}
		#endif

		return true;
	} else {
		*c = BACKGROUND_COLOUR;
		return false;
	}
}



char* ray_trace(s_screen &s, std::vector<shape*> &world, std::vector<light*> &lights) {
	vector3 h_step, v_step, pos;
	char *data = (char*) malloc(s.width*s.height*3);
	char *d;
	if(data == NULL) {
		std::cerr << "Fatal: ray_trace: Memory allocation failed";
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
	
	#ifdef TIME_TRACE
	clock_t t_start, t_end;
	double t_elapsed;
	std::cerr << "Begin timed ray trace" << std::endl;
	t_start = std::clock();
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
					trace(r, world, lights, TRACE_DEPTH, 1.0, &col);
					cuml += col/(SUPER_SAMPLE*SUPER_SAMPLE);
					sample_pos += h_sample_step;
				}
			}
			cuml.to_rbg(&c);
			#else
			r.dir = pos-s.eye;
			trace(r, world, lights, TRACE_DEPTH, 1.0, &col);
			col.to_rbg(&c);
			#endif
			d = data + (j*s.width + i)*3;
			d[0] = c.red;
			d[1] = c.green;
			d[2] = c.blue;
			pos += h_step;
		}
	}

	#ifdef TIME_TRACE
	t_end = std::clock();
	t_elapsed = (double) (t_end-t_start)/CLOCKS_PER_SEC;
	std::cerr << "Run in " << t_elapsed << " seconds." << std::endl;
	#endif
	return data;
}


void ray_tracer(int n_spheres, FILE* out_file) {
	char* data;
	s_screen screen;
	std::vector<shape*> *w;
	std::vector<light*> *l;
	
	std::cerr << "### Ray tracer ###" << std::endl;

	ray eye = {EYE_DIRECTION, EYE_POSITION};
	gen_screen(eye, &screen);
	#ifdef RUN_BENCHMARK
	w = sphere_world(n_spheres);
	#else
	//w = getWorld();
	w = sphere_world(27);
	#endif
	l = getLights();

	std::cerr << "Rendering at " << SCREEN_RESOLUTION_WIDTH << " by " << SCREEN_RESOLUTION_HEIGHT;
	#ifdef SUPER_SAMPLE
	std::cerr << " with " << (SUPER_SAMPLE*SUPER_SAMPLE) << " samples per pixel." << std::endl;
	#else
	std::cerr << " (supersampling disabled)." << std::endl;
	#endif
	std::cerr << w->size() << " shapes and " << l->size() << " lights." << std::endl;
	#ifdef REFLECTION_ENABLE
	std::cerr << "Reflection: enabled" << std::endl;
	#else
	std::cerr << "Reflection: disabled" << std::endl;
	#endif
	#ifdef SHADOWS_ENABLE
	std::cerr << "Shadows:    enabled" << std::endl;
	#else
	std::cerr << "Shadows:    disabled" << std::endl;
	#endif
	data = ray_trace(screen, *w, *l);
	#ifndef RUN_BENCHMARK
	write_pnm(data, screen.width, screen.height, out_file);
	#endif
	free(data);
	freeWorld(w);
	freeLights(l);
}

void ray_tracer(FILE* out_file) {
	ray_tracer(27, out_file);
}

int main() {
	#ifdef RUN_BENCHMARK
	for(int i=0; i<28; i++) {
		ray_tracer(i, NULL);
	}
	#else
	ray_tracer(stdout);
	#endif
	return 0;
}
