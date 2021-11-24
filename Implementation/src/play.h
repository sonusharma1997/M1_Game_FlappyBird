#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <GLFW/glfw3.h>
#include <linmath/linmath.h>

#include "config.h"
#include "font.h"
#include "model.h"
#include "opengl.h"
#include "physics.h"
#include "shader.h"
#include "texture.h"

// game resources
#include "models/sprite.h"
#include "shaders/font_frag.h"
#include "shaders/font_vert.h"
#include "shaders/sprite_frag.h"
#include "shaders/sprite_vert.h"
#include "textures/bg.h"
#include "textures/bird.h"
#include "textures/pipe_bot.h"
#include "textures/pipe_top.h"
#ifndef M_PI
#define M_PI 3.141592653589793
#endif


enum {
	NUMPIPE = 512,
};

struct FlappyBoard {
	// shader for font rendering
	unsigned int f_s;
	int f_s_uniform_layer;
	int f_s_uniform_model;
	int f_s_uniform_projection;
	
	// shader for sprite rendering
	unsigned int s_s;
	int s_s_uniform_model;
	int s_s_uniform_projection;
	unsigned int s_b;
	unsigned int s_m;
	unsigned int s_m_vertex_count;
	
	// texture handles
	unsigned int t_bg;
	unsigned int t_bird;
	unsigned int t_pipebottom;
	unsigned int t_pipetop;
	
	// timing vars
	double l_sec;
	double l_frme;
	long num_frame;
	
	// game state
	bool playing;
	bool game_over;
	bool space;
	long score;
	
	// game objects
	float camera;
	float bird_pos_x;
	float bird_pos_y;
	float bird_vel_x;
	float bird_vel_y;
	float pipes[NUMPIPE];
};

bool start_game(struct FlappyBoard* game);
void end_game(struct FlappyBoard* game);
void rst_gme(struct FlappyBoard* game);
void change_gme(struct FlappyBoard* game, GLFWwindow* window, double delta);
void load_game(struct FlappyBoard* game, long width, long height);
