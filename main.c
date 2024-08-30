#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#define HEIGHT 50
#define WIDTH (HEIGHT*2)
#define FPS 60.0

long st = 0;

struct vec3 {
	float x;
	float y;
	float z;
};


struct vec3 vec_mult(struct vec3 a, float s) {
	struct vec3 out = {a.x*s, a.y*s, a.z*s};
	return out;
}
float vec_mag(struct vec3 a) {
	return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}
struct vec3 vec_norm(struct vec3 a) {
	float len = vec_mag(a);
	struct vec3 out = {
		a.x/len,
		a.y/len,
		a.z/len
	};
	return out;
}
float vec_dot(struct vec3 a, struct vec3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}
struct vec3 vec_sub(struct vec3 a, struct vec3 b) {
	struct vec3 out = {a.x-b.x,a.y-b.y,a.z-b.z};
	return out;
}
struct vec3 vec_add(struct vec3 a, struct vec3 b) {
	struct vec3 out = {a.x+b.x,a.y+b.y,a.z+b.z};
	return out;
}

long time_ms() {
	struct timeval tp;
	gettimeofday(&tp, NULL);

	long t = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	return t - st;
}

void time_init() {
	st = time_ms();
}

float time_s() {
	return ((float) time_ms())/1000.0;
}


void clear() {
	system("clear");
}

// Main function of interest
// Acts like a fragment shader; given an x and y, return a pixel value
float fragment(int x, int y) {	
	// If time between fragments is significant, this might become a problem lmao
	float t = time_s(); 
	
	// Transform to screenspace 
	struct vec3 uv = { 2.0*((float)x/(float)WIDTH)-1.0, 2.0*((float)y/(float)HEIGHT)-1.0, 0.0};
	struct vec3 or = {0.0, 0.0, -1.0}; // Origin/source of rays from camera
	struct vec3 v = vec_norm(vec_sub(uv,or)); // Direction of camera ray
	//struct vec3 v = {0.0,0.0,1.0}; // Direction of camera ray
	struct vec3 c = {0.0, 0.0, 7.0}; // Position of centre of sphere
	float r = 4.0; // Radius of sphere
	
	struct vec3 vl = {cos(t), sin(t), -0.3*sin(0.3*t)};  //Light source direction
	vl = vec_norm(vl);
 
	
	// Find lambda for a projection
	float A = vec_dot(v,v);
	float B = 2.0*vec_dot(vec_sub(uv, c), v);
	float C = vec_dot(vec_sub(uv, c), vec_sub(uv, c)) - r*r;

	float val = B*B-4*A*C;
	float lambda = (-B-sqrt(val))/(2.0*A);

	if (val < 0.0 || lambda < 0.0) {
		return 0.0;
	}
	

	/*
	float val = r*r - (uv.x-c.x)*(uv.x-c.x) - (uv.y-c.y)*(uv.y-c.y);
	printf("%f\n", val);
	float lambda = sqrt(val) + c.z;
	*/

	struct vec3 n = vec_sub(vec_add(or, vec_mult(v,lambda)),c);
	//struct vec3 n = vec_norm(vec_sub(vec_add(uv, vec_mult(v, lambda)), c));


	//float out = 0.5+0.5*sin(uv.x*10.0 + sin(t)*5.0)*sin(uv.y*10.0 + cos(t)*5.0);
	float out = vec_dot(n, vl);
	out = 1-exp(-out);
	return out;
}

// Maps value between 0 and 1 to a character. e.g. the "." character
// takes up little space and should be somewhere near 0.05, while the "E"
// character takes up more space and should be closer to 0.5.
// It might be interesting to extend this to unicode for a wider variety
// of brightnesses displayable
char brightness_encode(float brightness) {
	// Define array of ascii characters sorted by "brightness"
	// Alternative character set " `.'^,*-=;$@"
	//static const char vals[] = " `.-':_,^=;><+!rc*/z?sLTv)J7(|Fi{C}fI31tlu[neogxjya]2pmwqkP6h9d45YSVO0GbZUAKXHE8RD#$BMNWQ%&@";
     	static const char vals[] = "$@B%8&WM#oahkbdpqwmZO0QLCJUYXzcvunxrjft*/\\|()1{}[]?-_+~i!lI;:,\"^` ";	

	// Clamp brightness to 0.0 and 1.0
	if (0.0 > brightness) {
		brightness = 0.0;
	} else if (brightness > 1.0) {
		brightness = 1.0;
	}

	brightness = 1.0 - brightness;

	// Compute the length of the array so we can map brightness to it
	// through float to int conversion.
	float vals_length = (float) (sizeof(vals)/sizeof(vals[0]));
	int index = (int) ((1-brightness)*vals_length);
	index = (int)(vals_length) - index;

	// If vals[] has length N, we'll be indexing vals[N] when brightness==0
	// so we handle the special case by printing a space instead
	
	if (index >= ((int) vals_length)-1) {
		index = ((int) vals_length) - 2;
	}
	
	return vals[index];
}

// Loop through buffer and print out characters depending on brightness
void render(float buffer[WIDTH][HEIGHT]) {
	char output_buffer[(WIDTH+1)*HEIGHT+1];
	int i = 0;
	for (int y=0; y < HEIGHT; y++) {
		for (int x=0; x < WIDTH; x++) {
			char output = brightness_encode(buffer[x][y]);
			//printf("%c", output, x, y);
			
			output_buffer[i] = output;
			i++;
		}
		//printf("\n");
		output_buffer[i] = '\n';
		i++;
	}
	//output_buffer[i] = '\0';

	clear();
	printf("%s", output_buffer);
}

// Update framebuffer with values from fragment shader
void update_framebuffer(float (*buffer)[HEIGHT]) {
	for (int x=0; x < WIDTH; x++) {
		for (int y=0; y < HEIGHT; y++) {
			buffer[x][y] = fragment(x,y);
		}
	}

}

int main() {
	float buffer[WIDTH][HEIGHT];

	// Set start time variable (long st) to current time
	// st is subtracted from current time to avoid errors
	// when converting large longs to floats
	time_init();
	


	float target_time = time_s() + 1/FPS;
	while(1) {
		if (time_s() > target_time) {
			update_framebuffer(buffer);
			//printf("seg");
			render(buffer);
			// Avoid expensive call time_s() which causes flickering
			printf("\nTime (s): %f\n", target_time); 
			target_time += 1/FPS;
		}
	}
	
	
	return 0;
}
