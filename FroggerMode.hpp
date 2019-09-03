#include "ColorTextureProgram.hpp"

#include "Mode.hpp"
#include "GL.hpp"

#include <vector>
#include <deque>

/*
 * FroggerMode is a game mode that implements TODO.
 */

struct FroggerMode : Mode {
	FroggerMode();
	virtual ~FroggerMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	glm::vec2 court_radius = glm::vec2(7.0f, 5.0f);
	// glm::vec2 paddle_radius = glm::vec2(0.2f, 1.0f);
	// glm::vec2 ball_radius = glm::vec2(0.2f, 0.2f);

	// glm::vec2 left_paddle = glm::vec2(-court_radius.x + 0.5f, 0.0f);
	// glm::vec2 right_paddle = glm::vec2( court_radius.x - 0.5f, 0.0f);

	// glm::vec2 ball = glm::vec2(0.0f, 0.0f);
	// glm::vec2 ball_velocity = glm::vec2(-1.0f, 0.0f);

	// uint32_t left_score = 0;
	// uint32_t right_score = 0;
  glm::vec2 score_radius = glm::vec2(0.04f, 0.04f);

	// float ai_offset = 0.0f;
	// float ai_offset_update = 0.0f;

  glm::vec2 roadmark_radius = glm::vec2(0.5f, 0.1f);
  unsigned int road_lane_count = 8; // includes grass

  unsigned int level_max = 7;
	glm::vec2 size_min = glm::vec2(0.1f, 0.1f);
	glm::vec2 size_growth = glm::vec2(0.06f, 0.06f);
  struct {
    glm::vec2 radius = glm::vec2(0.1f, 0.1f);
    glm::vec2 position = glm::vec2(0.0f, -4.5f);
    unsigned int points = 0;
    unsigned int level = 0;
    bool move_up = false, move_down = false, move_left = false, move_right = false;
    // progress[i] is total # of points needed to progress to level i+1
    std::vector< unsigned int > progress = {3, 10, 20, 40, 80, 150};
    float speed = 0.025f; // each key pressed changes position.x,y by this amt
  } frog;

	typedef struct Vehicle {
		glm::vec2 position;
		unsigned int level; // Generate random number between 0 and 6 incl.
    glm::vec2 radius; // Add level times size_growth to size_min
    float speed;
    int direction;
    glm::u8vec4 color;
	} Vehicle;
	#define HEX_TO_U8VEC4( HX ) (glm::u8vec4( (HX >> 24) & 0xff, (HX >> 16) & 0xff, (HX >> 8) & 0xff, (HX) & 0xff ))
  const std::vector< glm::u8vec4 > vehicle_colors = {
		HEX_TO_U8VEC4(0xe2ff70ff), HEX_TO_U8VEC4(0xcbff70ff), HEX_TO_U8VEC4(0xaeff5dff),
		HEX_TO_U8VEC4(0x88ff52ff), HEX_TO_U8VEC4(0x6cff47ff), HEX_TO_U8VEC4(0x3aff37ff),
		HEX_TO_U8VEC4(0x2eff94ff), HEX_TO_U8VEC4(0x2effa5ff), HEX_TO_U8VEC4(0x17ffc1ff),
		HEX_TO_U8VEC4(0x00f4e7ff), HEX_TO_U8VEC4(0x00cbe4ff), HEX_TO_U8VEC4(0x00b0d8ff),
		HEX_TO_U8VEC4(0x00a5d1ff), HEX_TO_U8VEC4(0x0098cfd8), HEX_TO_U8VEC4(0x0098cf54),
		HEX_TO_U8VEC4(0x0098cf54), HEX_TO_U8VEC4(0x0098cf54), HEX_TO_U8VEC4(0x0098cf54),
		HEX_TO_U8VEC4(0x0098cf54), HEX_TO_U8VEC4(0x0098cf54), HEX_TO_U8VEC4(0x0098cf54),
		HEX_TO_U8VEC4(0x0098cf54)
  };
	#undef HEX_TO_U8VEC4
	std::vector< Vehicle > vehicles;
	unsigned int vehicle_count_max = 20;
  float vehicle_speed = 0.05f;
  float vehicle_start_x = 7.5f;

	bool gameover = false;

	//other useful drawing constants:
	const float wall_radius = 0.05f;
	const float shadow_offset = 0.07f;
	const float padding = 0.14f; //padding between outside of walls and edge of window

	//----- pretty rainbow trails -----

	// float trail_length = 1.3f;
	// std::deque< glm::vec3 > ball_trail; //stores (x,y,age), oldest elements first

	//----- opengl assets / helpers ------

	//draw functions will work on vectors of vertices, defined as follows:
	struct Vertex {
		Vertex(glm::vec3 const &Position_, glm::u8vec4 const &Color_, glm::vec2 const &TexCoord_) :
			Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
		glm::vec3 Position;
		glm::u8vec4 Color;
		glm::vec2 TexCoord;
	};
	static_assert(sizeof(Vertex) == 4*3 + 1*4 + 4*2, "FroggerMode::Vertex should be packed");

	//Shader program that draws transformed, vertices tinted with vertex colors:
	ColorTextureProgram color_texture_program;

	//Buffer used to hold vertex data during drawing:
	GLuint vertex_buffer = 0;

	//Vertex Array Object that maps buffer locations to color_texture_program attribute locations:
	GLuint vertex_buffer_for_color_texture_program = 0;

	//Solid white texture:
	GLuint white_tex = 0;

	//matrix that maps from clip coordinates to court-space coordinates:
	glm::mat3x2 clip_to_court = glm::mat3x2(1.0f);
	// computed in draw() as the inverse of OBJECT_TO_CLIP
	// (stored here so that the mouse handling code can use it to position the paddle)

};
