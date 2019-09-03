// Sections adapted from base code: https://github.com/15-466/15-466-f19-base0
#include "FroggerMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

FroggerMode::FroggerMode() {

	//----- allocate OpenGL resources -----
	{ //vertex buffer:
		glGenBuffers(1, &vertex_buffer);
		//for now, buffer will be un-filled.

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //vertex array mapping buffer for color_texture_program:
		//ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unused vertex array object:
		glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

		//set vertex_buffer_for_color_texture_program as the current vertex array object:
		glBindVertexArray(vertex_buffer_for_color_texture_program);

		//set vertex_buffer as the source of glVertexAttribPointer() commands:
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		//set up the vertex array object to describe arrays of FroggerMode::Vertex:
		glVertexAttribPointer(
			color_texture_program.Position_vec4, //attribute
			3, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 0 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Position_vec4);
		//[Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]

		glVertexAttribPointer(
			color_texture_program.Color_vec4, //attribute
			4, //size
			GL_UNSIGNED_BYTE, //type
			GL_TRUE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Color_vec4);

		glVertexAttribPointer(
			color_texture_program.TexCoord_vec2, //attribute
			2, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 + 4*1 //offset
		);
		glEnableVertexAttribArray(color_texture_program.TexCoord_vec2);

		//done referring to vertex_buffer, so unbind it:
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//done setting up vertex array object, so unbind it:
		glBindVertexArray(0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //solid white texture:
		//ask OpenGL to fill white_tex with the name of an unused texture object:
		glGenTextures(1, &white_tex);

		//bind that texture object as a GL_TEXTURE_2D-type texture:
		glBindTexture(GL_TEXTURE_2D, white_tex);

		//upload a 1x1 image of solid white to the texture:
		glm::uvec2 size = glm::uvec2(1,1);
		std::vector< glm::u8vec4 > data(size.x*size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		//set filtering and wrapping parameters:
		//(it's a bit silly to mipmap a 1x1 texture, but I'm doing it because you may want to use this code to load different sizes of texture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
		glGenerateMipmap(GL_TEXTURE_2D);

		//Okay, texture uploaded, can unbind it:
		glBindTexture(GL_TEXTURE_2D, 0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}
}

FroggerMode::~FroggerMode() {

	//----- free OpenGL resources -----
	glDeleteBuffers(1, &vertex_buffer);
	vertex_buffer = 0;

	glDeleteVertexArrays(1, &vertex_buffer_for_color_texture_program);
	vertex_buffer_for_color_texture_program = 0;

	glDeleteTextures(1, &white_tex);
	white_tex = 0;
}

bool FroggerMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	// Up/down/left/right for continuous motion of frog (update frog position.x,y)

	if (evt.type == SDL_KEYDOWN){
		if (evt.key.keysym.scancode == SDL_SCANCODE_W) {
      frog.move_up = true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_A) {
      frog.move_left = true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_S) {
      frog.move_down = true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_D) {
      frog.move_right = true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_R) {
      // reset game
      frog.radius = glm::vec2(0.1f, 0.1f);
      frog.position = glm::vec2(0.0f, -4.5f);
      frog.points = 0; frog.level = 0;
      frog.move_up   = false; frog.move_down  = false;
      frog.move_left = false; frog.move_right = false;
      vehicles.clear();
      gameover = false;
    }
	} else if (evt.type == SDL_KEYUP){
		if (evt.key.keysym.scancode == SDL_SCANCODE_W) {
      frog.move_up = false;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_A) {
      frog.move_left = false;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_S) {
      frog.move_down = false;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_D) {
      frog.move_right = false;
		}
	}

	return false; // QUESTION: why return false?
}

void FroggerMode::update(float elapsed) {

	static std::mt19937 mt; //mersenne twister pseudo-random number generator

  if (!gameover) {
    // move frog
    if (frog.move_up)    {frog.position.y += frog.speed; }
    if (frog.move_left)  {frog.position.x -= frog.speed; }
    if (frog.move_down)  {frog.position.y -= frog.speed; }
    if (frog.move_right) {frog.position.x += frog.speed; }

  	// clamp frog to road
    frog.position.x = std::max(-court_radius.x+wall_radius*2, frog.position.x);
    frog.position.x = std::min( court_radius.x-wall_radius*2, frog.position.x);
    frog.position.y = std::max(-court_radius.y+wall_radius*2, frog.position.y);
    frog.position.y = std::min( court_radius.y-wall_radius*2, frog.position.y);
  }

	// update positions of vehicles, removing if necessary
  for (std::vector<Vehicle>::iterator it = vehicles.begin(); it != vehicles.end(); it++) {
    it->position += glm::vec2(it->speed * it->direction, 0.0f);
  }
  auto isOutOfBounds = [&](const Vehicle &v){
    return (v.position.x < -vehicle_start_x || v.position.x > vehicle_start_x);
  };
  // based on https://en.cppreference.com/w/cpp/algorithm/remove
  std::vector<Vehicle>::iterator end = std::remove_if(vehicles.begin(), vehicles.end(), isOutOfBounds);
  vehicles.erase(end, vehicles.end());

	// generate new vehicles if needed, spawn with 3.33% probability per frame
  if (vehicles.size() < vehicle_count_max){
    if (mt() % 30 == 0) {
      Vehicle new_vehicle;
      unsigned int lane = (mt()%(road_lane_count-2))+1;
      float half_lane_width = court_radius.y/road_lane_count;
      float lane_y = (2*lane+1)*half_lane_width - court_radius.y;
      new_vehicle.direction = (lane%2)?-1:1;
      new_vehicle.position = glm::vec2(vehicle_start_x*(-new_vehicle.direction), lane_y);
      new_vehicle.level = mt() % level_max;
      new_vehicle.radius = size_growth*float(new_vehicle.level) + size_min;
      new_vehicle.speed = vehicle_speed;
      new_vehicle.color = vehicle_colors[mt()%vehicle_colors.size()];
      vehicles.push_back(new_vehicle);
    }
  }

	// check for collision between vehicles and frog
  auto frog_vs_vehicle = [this](Vehicle &v) {
    glm::vec2 min = glm::max(v.position-v.radius, frog.position-frog.radius);
    glm::vec2 max = glm::min(v.position+v.radius, frog.position+frog.radius);
		if (min.x > max.x || min.y > max.y) return;
    if (v.level > frog.level) { gameover = true; return; }
    // frog collided with smaller/equally sized vehicle: consume.
    frog.points += v.level + 1;
    v.position.x = 10.0f;
    if (frog.level < level_max && frog.points >= frog.progress[frog.level]) {
      frog.level++;
      frog.radius += size_growth;
    }
  };
  if (!gameover) {
    for (std::vector<Vehicle>::iterator it = vehicles.begin(); it != vehicles.end(); it++) {
      frog_vs_vehicle(*it);
    }
  }
}

void FroggerMode::draw(glm::uvec2 const &drawable_size) {

	//some nice colors from the course web page:
	#define HEX_TO_U8VEC4( HX ) (glm::u8vec4( (HX >> 24) & 0xff, (HX >> 16) & 0xff, (HX >> 8) & 0xff, (HX) & 0xff ))
	const glm::u8vec4 bg_color = HEX_TO_U8VEC4(0xf3ffc6ff);
	const glm::u8vec4 fg_color = HEX_TO_U8VEC4(0x000000ff);
  const glm::u8vec4 road_color = HEX_TO_U8VEC4(0x444444ff);
  const glm::u8vec4 roadmark_color = HEX_TO_U8VEC4(0xfffdf7ff);
  const glm::u8vec4 grass_color = HEX_TO_U8VEC4(0x6bec50ff);
  const glm::u8vec4 frog_color = HEX_TO_U8VEC4(0x007c2aff);
  const glm::u8vec4 dead_color = HEX_TO_U8VEC4(0xc72929ff);
	#undef HEX_TO_U8VEC4

	//---- compute vertices to draw ----

	//vertices will be accumulated into this list and then uploaded+drawn at the end of this function:
	std::vector< Vertex > vertices;

	//inline helper function for rectangle drawing:
	auto draw_rectangle = [&vertices](glm::vec2 const &center, glm::vec2 const &radius, glm::u8vec4 const &color) {
		//split rectangle into two CCW-oriented triangles:
		vertices.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));

		vertices.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x-radius.x, center.y+radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
	};

	//Road:
  float half_lane_width = court_radius.y/road_lane_count;
  float lane_width = half_lane_width * 2;
  draw_rectangle(glm::vec2(0.0f, 0.0f), glm::vec2(court_radius.x, court_radius.y), road_color);
  draw_rectangle(glm::vec2(0.0f, court_radius.y-half_lane_width), glm::vec2(court_radius.x, half_lane_width), grass_color);
  draw_rectangle(glm::vec2(0.0f,-court_radius.y+half_lane_width), glm::vec2(court_radius.x, half_lane_width), grass_color);
  for(unsigned int i=2; i<road_lane_count-1; i++){
    float roadmark_y = court_radius.y - (lane_width * i);
    // Exactly 5 roadmarks between lanes, roadmark radius is fixed
    for(unsigned int j=1; j<=9; j+=2){
      draw_rectangle(glm::vec2(j*(court_radius.x/5)-court_radius.x, roadmark_y), roadmark_radius, roadmark_color);
    }
  }

	//walls:
	draw_rectangle(glm::vec2(-court_radius.x-wall_radius, 0.0f), glm::vec2(wall_radius, court_radius.y + 2.0f * wall_radius), fg_color);
	draw_rectangle(glm::vec2( court_radius.x+wall_radius, 0.0f), glm::vec2(wall_radius, court_radius.y + 2.0f * wall_radius), fg_color);
	draw_rectangle(glm::vec2( 0.0f,-court_radius.y-wall_radius), glm::vec2(court_radius.x, wall_radius), fg_color);
	draw_rectangle(glm::vec2( 0.0f, court_radius.y+wall_radius), glm::vec2(court_radius.x, wall_radius), fg_color);

  //frog:
  if (gameover) draw_rectangle(frog.position, frog.radius, dead_color);
  else draw_rectangle(frog.position, frog.radius, frog_color);

  //vehicles:
	static std::mt19937 mt; //mersenne twister pseudo-random number generator
  for (std::vector<Vehicle>::iterator it = vehicles.begin(); it != vehicles.end(); it++) {
    draw_rectangle(it->position, it->radius, it->color);
  }

	//scores:
  unsigned int horizontal_limit = 117;
	for (unsigned int i = 0; i < std::min(frog.points, horizontal_limit); ++i) {
		draw_rectangle(glm::vec2( court_radius.x - (2.0f + 3.0f * i) * score_radius.x, court_radius.y + 2.0f * wall_radius + 2.0f * score_radius.y), score_radius, fg_color);
	}
  if (frog.points > horizontal_limit) {
  	for (unsigned int i = 0; i < frog.points - horizontal_limit; ++i) {
  		draw_rectangle(glm::vec2( court_radius.x - (2.0f + 3.0f * i) * score_radius.x, court_radius.y + 2.0f * wall_radius + 5.0f * score_radius.y), score_radius, fg_color);
  	}
  }

	//------ compute court-to-window transform ------

	//compute area that should be visible:
	glm::vec2 scene_min = glm::vec2(
		-court_radius.x - 2.0f * wall_radius - padding,
		-court_radius.y - 2.0f * wall_radius - padding
	);
	glm::vec2 scene_max = glm::vec2(
		court_radius.x + 2.0f * wall_radius + padding,
		court_radius.y + 2.0f * wall_radius + 3.0f * score_radius.y + padding
	);

	//compute window aspect ratio:
	float aspect = drawable_size.x / float(drawable_size.y);
	//we'll scale the x coordinate by 1.0 / aspect to make sure things stay square.

	//compute scale factor for court given that...
	float scale = std::min(
		(2.0f * aspect) / (scene_max.x - scene_min.x), //... x must fit in [-aspect,aspect] ...
		(2.0f) / (scene_max.y - scene_min.y) //... y must fit in [-1,1].
	);

	glm::vec2 center = 0.5f * (scene_max + scene_min);

	//build matrix that scales and translates appropriately:
	glm::mat4 court_to_clip = glm::mat4(
		glm::vec4(scale / aspect, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, scale, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(-center.x * (scale / aspect), -center.y * scale, 0.0f, 1.0f)
	);
	//NOTE: glm matrices are specified in *Column-Major* order,
	// so this matrix is actually transposed from how it appears.

	//also build the matrix that takes clip coordinates to court coordinates (used for mouse handling):
	clip_to_court = glm::mat3x2(
		glm::vec2(aspect / scale, 0.0f),
		glm::vec2(0.0f, 1.0f / scale),
		glm::vec2(center.x, center.y)
	);

	//---- actual drawing ----

	//clear the color buffer:
	glClearColor(bg_color.r / 255.0f, bg_color.g / 255.0f, bg_color.b / 255.0f, bg_color.a / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	//upload vertices to vertex_buffer:
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); //set vertex_buffer as current
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STREAM_DRAW); //upload vertices array
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//set color_texture_program as current program:
	glUseProgram(color_texture_program.program);

	//upload OBJECT_TO_CLIP to the proper uniform location:
	glUniformMatrix4fv(color_texture_program.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(court_to_clip));

	//use the mapping vertex_buffer_for_color_texture_program to fetch vertex data:
	glBindVertexArray(vertex_buffer_for_color_texture_program);

	//bind the solid white texture to location zero:
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, white_tex);

	//run the OpenGL pipeline:
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

	//unbind the solid white texture:
	glBindTexture(GL_TEXTURE_2D, 0);

	//reset vertex array to none:
	glBindVertexArray(0);

	//reset current program to none:
	glUseProgram(0);

	GL_ERRORS(); //PARANOIA: print errors just in case we did something wrong.

}
