#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#define WIDTH 100
#define HEIGHT 50
#define FPS 60.0

long st = 0;


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

// Acts like a fragment shader; given an x and y, return a pixel value
float fragment(int x, int y) {
	// Transform to screenspace 
	float u = 2.0*((float)x/(float)WIDTH)-1.0;
	float v = 2.0*((float)y/(float)HEIGHT)-1.0;

	float t = time_s();

	float out = 0.5+0.5*sin(u*10.0 + sin(t)*5.0)*sin(v*10.0 + cos(t)*5.0);
	return out;
}

// Maps value between 0 and 1 to a character. e.g. the "." character
// takes up little space and should be somewhere near 0.05, while the "E"
// character takes up more space and should be closer to 0.5.
// It might be interesting to extend this to unicode for a wider variety
// of brightnesses displayable
char brightness_encode(float brightness) {
	// Define array of ascii characters sorted by "brightness"
	static const char vals[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

	// Clamp brightness to 0.0 and 1.0
	if (0.0 > brightness) {
		brightness = 0.0;
	} else if (brightness > 1.0) {
		brightness = 1.0;
	}

	// Compute the length of the array so we can map brightness to it
	// through float to int conversion.
	float vals_length = (float) (sizeof(vals)/sizeof(vals[0]));
	int index = (int) ((1.0-brightness)*vals_length);

	// If vals[] has length N, we'll be indexing vals[N] when brightness==0
	// so we handle the special case by printing a space instead
	
	if (index >= ((int) vals_length)-1) {
		index = ((int) vals_length) - 2;
	}
	
	return vals[index];
}

// Loop through buffer and print out characters depending on brightness
void render(float buffer[WIDTH][HEIGHT]) {
	for (int y=0; y < HEIGHT; y++) {
		for (int x=0; x < WIDTH; x++) {
			char output = brightness_encode(buffer[x][y]);
			printf("%c", output, x, y);
		}
		printf("\n");
	}
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
			clear();
			render(buffer);
			// Avoid expensive call time_s() which causes flickering
			printf("\nTime (s): %f\n", target_time); 
			target_time += 1/FPS;
		}
	}
	
	
	return 0;
}
