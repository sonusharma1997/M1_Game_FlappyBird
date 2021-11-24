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

// boardstate resources
#include "models/sprite.h"
#include "shaders/font_frag.h"
#include "shaders/font_vert.h"
#include "shaders/sprite_frag.h"
#include "shaders/sprite_vert.h"
#include "textures/bg.h"
#include "textures/bird.h"
#include "textures/pipe_bot.h"
#include "textures/pipe_top.h"
#include "play.h"
static void
draw_sprite(struct FlappyBoard* boardstate, unsigned t, float x, float y, float z, float r, float sx, float sy)
{
	// bind the shader
	glUseProgram(boardstate->s_s);
	
	// setup model matrix
	mat4x4 m = {{ 0 }};
	mat4x4_translate(m, x, y, z);
	mat4x4_rotate_Z(m, m, r * (M_PI / 180.0));  // convert deg to rad
	mat4x4_scale_aniso(m, m, sx, sy, 1.0f);
	glUniformMatrix4fv(boardstate->s_s_uniform_model, 1, GL_FALSE, (const float*)m);
	
	// setup projection matrix
	mat4x4 p = {{ 0 }};
	mat4x4_identity(p);
	mat4x4_ortho(p, -(WIDTH / 2.0f), (WIDTH / 2.0f), -(HEIGHT / 2.0f), (HEIGHT / 2.0f), -1.0f, 1.0f);
	glUniformMatrix4fv(boardstate->s_s_uniform_projection, 1, GL_FALSE, (const float*)p);
	
	// bind the texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, t);
	
	// bind the model
	glBindVertexArray(boardstate->s_m);
	
	// draw the sprite!
	glDrawArrays(GL_TRIANGLES, 0, boardstate->s_m_vertex_count);
}

static void
draw_text(struct FlappyBoard* boardstate, const char* str, float x, float y, float z, float sx, float sy)
{
	// bind the shader
	glUseProgram(boardstate->f_s);
	
	// setup layer
	glUniform1f(boardstate->f_s_uniform_layer, z);
	
	// setup model matrix
	mat4x4 m = {{ 0 }};
	mat4x4_translate(m, x, y, 0.0f);
	mat4x4_scale_aniso(m, m, sx, sy, 1.0f);
	glUniformMatrix4fv(boardstate->f_s_uniform_model, 1, GL_FALSE, (const float*)m);
	
	// setup projection matrix
	mat4x4 p = {{ 0 }};
	mat4x4_identity(p);
	mat4x4_ortho(p, -(WIDTH / 2.0f), (WIDTH / 2.0f), -(HEIGHT / 2.0f), (HEIGHT / 2.0f), -1.0f, 1.0f);
	glUniformMatrix4fv(boardstate->f_s_uniform_projection, 1, GL_FALSE, (const float*)p);
	
	long vertices = font_vertices(str);
	
	long size = font_size(str);
	float* buf = malloc(size);
	assert(buf != NULL);
	
	// amazing 4x4 bitmap font clarity
	font_print(str, buf, size);
	
	unsigned vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	unsigned vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glBufferData(GL_ARRAY_BUFFER, size, buf, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);
	glEnableVertexAttribArray(0);
	
	free(buf);
	
	glUseProgram(boardstate->f_s);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertices);
	
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

bool
start_game(struct FlappyBoard* boardstate)
{
	assert(boardstate != NULL);
	
	// create shader for rendering text
	boardstate->f_s = shader_compile_and_link(SHADER_FONT_VERT_SOURCE, SHADER_FONT_FRAG_SOURCE);
	boardstate->f_s_uniform_layer = glGetUniformLocation(boardstate->f_s, "u_layer");
	boardstate->f_s_uniform_model = glGetUniformLocation(boardstate->f_s, "u_model");
	boardstate->f_s_uniform_projection = glGetUniformLocation(boardstate->f_s, "u_projection");
	
	// create shader for rendering sprites
	boardstate->s_s = shader_compile_and_link(SHADER_SPRITE_VERT_SOURCE, SHADER_SPRITE_FRAG_SOURCE);
	boardstate->s_s_uniform_model = glGetUniformLocation(boardstate->s_s, "u_model");
	boardstate->s_s_uniform_projection = glGetUniformLocation(boardstate->s_s, "u_projection");
	
	// set texture uniform location
	glUseProgram(boardstate->s_s);
	glUniform1i(glGetUniformLocation(boardstate->s_s, "u_texture"), 0);
	glUseProgram(0);
	
	// create model for rendering sprites
	boardstate->s_b = model_buffer_create(MODEL_SPRITE_FORMAT, MODEL_SPRITE_VERTEX_COUNT, MODEL_SPRITE_VERTICES);
	boardstate->s_m = model_buffer_config(MODEL_SPRITE_FORMAT, boardstate->s_b);
	boardstate->s_m_vertex_count = MODEL_SPRITE_VERTEX_COUNT;
	
	// create textures
	boardstate->t_bg = texture_create(TEXTURE_BG_FORMAT, TEXTURE_BG_WIDTH, TEXTURE_BG_HEIGHT, TEXTURE_BG_PIXELS);
	boardstate->t_bird = texture_create(TEXTURE_BIRD_FORMAT,TEXTURE_BIRD_WIDTH, TEXTURE_BIRD_HEIGHT, TEXTURE_BIRD_PIXELS);
	boardstate->t_pipebottom = texture_create(TEXTURE_PIPE_BOT_FORMAT, TEXTURE_PIPE_BOT_WIDTH,TEXTURE_PIPE_BOT_HEIGHT, TEXTURE_PIPE_BOT_PIXELS);
	boardstate->t_pipetop = texture_create(TEXTURE_PIPE_TOP_FORMAT,TEXTURE_PIPE_TOP_WIDTH, TEXTURE_PIPE_TOP_HEIGHT, TEXTURE_PIPE_TOP_PIXELS);
	
	// reset
	rst_gme(boardstate);
	return true;
}

void
end_game(struct FlappyBoard* boardstate)
{
	assert(boardstate != NULL);
	
	glDeleteProgram(boardstate->f_s);
	glDeleteProgram(boardstate->s_s);
	glDeleteBuffers(1, &boardstate->s_b);
	glDeleteVertexArrays(1, &boardstate->s_m);
	glDeleteTextures(1, &boardstate->t_bg);
	glDeleteTextures(1, &boardstate->t_bird);
	glDeleteTextures(1, &boardstate->t_pipebottom);
	glDeleteTextures(1, &boardstate->t_pipetop);
}

void
rst_gme(struct FlappyBoard* boardstate)
{
	assert(boardstate != NULL);
	
	// boardstate state
	boardstate->playing = false;
	boardstate->game_over = false;
	boardstate->space = false;
	boardstate->score = 0;
	
	// boardstate objects
	boardstate->camera = -3.0f;
	boardstate->bird_pos_x = -6.0f;
	boardstate->bird_pos_y = 0.0f;
	boardstate->bird_vel_x = SPEED;
	boardstate->bird_vel_y = 0.0f;
	for (long i = 0; i < NUMPIPE; i++) {
		float gap = (float)rand() / (float)RAND_MAX;  // [0.0, 1.0]
		gap -= 0.5f;  // [-0.5, 0.5]
		boardstate->pipes[i] = gap * 4.0f;  // [-2.0, 2.0]
	}
}

void
change_gme(struct FlappyBoard* boardstate, GLFWwindow* rootwin, double delta)
{
	// can change_gme be detached from the GLFWwindow* ?
	if (glfwGetKey(rootwin, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(rootwin, GLFW_TRUE);
	}
	
	// only allow single flaps (not continuous)
	if (glfwGetKey(rootwin, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (boardstate->game_over) rst_gme(boardstate);
		
		boardstate->playing = true;
		if (!boardstate->space) {
			boardstate->bird_vel_y = FLAP;
			boardstate->space = true;
		} else {
			boardstate->bird_vel_y -= delta * GRAVITY;
		}
	} else {
		if (boardstate->playing) {
			boardstate->bird_vel_y -= delta * GRAVITY;
		}
		boardstate->space = false;
	}
	
	// update bird and camera positions
	if (boardstate->playing) {
		boardstate->bird_pos_x += (boardstate->bird_vel_x * delta);
		boardstate->bird_pos_y += (boardstate->bird_vel_y * delta);
		boardstate->camera += (boardstate->bird_vel_x * delta);
	}
	
	// check collision
	if (boardstate->bird_pos_x >= -4.0f) {
		// determine index of the next approaching pipe
		long pipe_index = (boardstate->bird_pos_x + 2.0f) / 4.0f;
		float gap = boardstate->pipes[pipe_index % NUMPIPE];
		float top = gap + GAP;
		float bot = gap - GAP;
		
		bool collision = false;
		if (physics_intersect_circle_rect(boardstate->bird_pos_x, boardstate->bird_pos_y, 0.3f,
			pipe_index * 4.0f, top, PIPE_WIDTH, PIPE_HEIGHT)) {
			collision = true;
			}
			if (physics_intersect_circle_rect(boardstate->bird_pos_x, boardstate->bird_pos_y, 0.3f,
				pipe_index * 4.0f, bot, PIPE_WIDTH, PIPE_HEIGHT)) {
				collision = true;
				}
				if (boardstate->bird_pos_y > 4.5f || boardstate->bird_pos_y < -4.5f) {
					collision = true;
				}
				
				if (collision && !boardstate->game_over) {
					boardstate->game_over = true;
					boardstate->bird_vel_x = 0.0f;
					boardstate->bird_vel_y = 8.0f;
				}
	}
	
	// determine score based on bird's position
	boardstate->score = (boardstate->bird_pos_x + 3.0f) / 4.0f;
}

void
game_render(struct FlappyBoard* boardstate, long width, long height)
{
	// determine boxing and calculate centering offsets
	long x_offset = 0;
	long y_offset = 0;
	float aspect = (float)width / height;
	if (aspect <= ASPECT) {
		// letterbox
		y_offset = (height - (width / ASPECT)) / 2;
		height = width / ASPECT;
	} else {
		// pillarbox
		x_offset = (width - (height * ASPECT)) / 2;
		width = height * ASPECT;
	}
	
	// set viewport every frame (is this bad?)
	glViewport(x_offset, y_offset, width, height);
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// draw background (scrolls independently of boardstate objects)
	double bg_scroll = glfwGetTime() * SCROLL;
	double bg_offset = fmod(bg_scroll, 4.5);
	for (float x = -9.0f; x <= 13.5f; x += 4.5f) {
		draw_sprite(boardstate, boardstate->t_bg,
					x - bg_offset, 0.0f, BG_LAYER,
			  0.0f, BG_WIDTH, BG_HEIGHT);
	}
	
	// draw pipes (every 4.0f units starting at 0.0f)
	for (float x = boardstate->camera - 8.0f; x <= boardstate->camera + 12.0f; x += 4.0f) {
		if (x < 0.0f) continue;
		long pipe_index = x / 4.0f;
		float gap = boardstate->pipes[pipe_index % NUMPIPE];
		float top = gap + GAP;
		float bot = gap - GAP;
		float pipe_x = pipe_index * 4.0f;
		draw_sprite(boardstate, boardstate->t_pipetop,
					pipe_x - boardstate->camera, top, PIPE_LAYER,
			  0.0f, PIPE_WIDTH, PIPE_HEIGHT);
		draw_sprite(boardstate, boardstate->t_pipebottom,
					pipe_x - boardstate->camera, bot, PIPE_LAYER,
			  0.0f, PIPE_WIDTH, PIPE_HEIGHT);
	}
	
	// draw bird
	draw_sprite(boardstate, boardstate->t_bird,
				boardstate->bird_pos_x - boardstate->camera, boardstate->bird_pos_y, BIRD_LAYER,
			 boardstate->bird_vel_y * 5.0f, BIRD_WIDTH, BIRD_HEIGHT);
	
	// draw score
	char score_text[16] = { 0 };
	snprintf(score_text, 16, "%.3ld", boardstate->score);
	draw_text(boardstate, score_text, -WIDTH / 2.0f + 1.0f, HEIGHT / 2.0f - 1.0f, 0.5f, 0.5f, 0.5f);
}
