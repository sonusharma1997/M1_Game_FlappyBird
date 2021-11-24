#include "unity.h"
#include <play.h>

#define PROJECT_NAME    "Flappy Bird"

void test_update(void);
void test_reset(void);

void setUp(){}

void tearDown(){}

int main()
{
  UNITY_BEGIN();


  RUN_TEST(test_update);
  RUN_TEST(test_reset);

  return UNITY_END();
}

void test_update(void) {
	
	
	if (!glfwInit()) {
		const char* error = NULL;
		glfwGetError(&error);
		fprintf(stderr, "failed to init GLFW3: %s\n", error);
		assert(0==1);
	}
	
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	
	// ask for an OpenGL 3.3 Core profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	
	// set flags for monitor details
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	
	GLFWwindow* window = NULL;
	
		window = glfwCreateWindow(1280, 720, "Flappy Bird", NULL, NULL);
		if (window == NULL) {
			const char* error = NULL;
			glfwGetError(&error);
			fprintf(stderr, "failed to create GLFW3 window: %s\n", error);
			
			glfwTerminate();
			assert(0==1);
		}
	
	struct FlappyBoard game;
	game.f_s = 3;
	game.f_s_uniform_layer = 0;
	game.f_s_uniform_model = 1; 
	game.f_s_uniform_projection = 2;
	game.s_s = 6;
	game.s_s_uniform_model = 0;
	game.s_s_uniform_projection = 1;
	game.s_b = 1;
	game.s_m = 1;
	game.s_m_vertex_count = 6;
	game.t_bg = 1;
	game.t_bird = 2;
	game.t_pipebottom = 3;
	game.t_pipetop = 4;
	game.l_sec = 0;
	game.l_frme = 0, 
	game.num_frame = 0;
	game.playing = false;
	game.game_over = false;
	game.space = false;
	game.score = 0;
	game.camera = -3;
	game.bird_pos_x = -6; 
	game.bird_pos_y = 0; 
	game.bird_vel_x = 6; game.bird_vel_y = 0;
	for (long i = 0; i < NUMPIPE; i++) {
		float gap = (float)rand() / (float)RAND_MAX;  // [0.0, 1.0]
		gap -= 0.5f;  // [-0.5, 0.5]
		game.pipes[i] = gap * 4.0f;  // [-2.0, 2.0]
	}
	struct FlappyBoard lgame=game;
	// timing vars
	long num_frame = 0;
change_gme(&game, window, 0.63);
  TEST_ASSERT_EQUAL(lgame.game_over, game.game_over);
}

void test_reset(void) {
		
		
	struct FlappyBoard game;
	game.f_s = 3;
	game.f_s_uniform_layer = 0;
	game.f_s_uniform_model = 1; 
	game.f_s_uniform_projection = 2;
	game.s_s = 6;
	game.s_s_uniform_model = 0;
	game.s_s_uniform_projection = 1;
	game.s_b = 1;
	game.s_m = 1;
	game.s_m_vertex_count = 6;
	game.t_bg = 1;
	game.t_bird = 2;
	game.t_pipebottom = 3;
	game.t_pipetop = 4;
	game.l_sec = 0;
	game.l_frme = 0, 
	game.num_frame = 0;
	game.playing = false;
	game.game_over = false;
	game.space = false;
	game.score = 0;
	game.camera = -3;
	game.bird_pos_x = -6; 
	game.bird_pos_y = 0; 
	game.bird_vel_x = 6; game.bird_vel_y = 0;
	for (long i = 0; i < NUMPIPE; i++) {
		float gap = (float)rand() / (float)RAND_MAX;  // [0.0, 1.0]
		gap -= 0.5f;  // [-0.5, 0.5]
		game.pipes[i] = gap * 4.0f;  // [-2.0, 2.0]
	}
		struct FlappyBoard lgame=game;
		// timing vars
		long num_frame = 0;
		game.game_over=true;
		rst_gme(&game);
		TEST_ASSERT_EQUAL(false,game.game_over);
		
		
		
		

 
}
