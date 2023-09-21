#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>

#include <random>

extern SDL_Window *window;
extern int test;



GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("knob.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("knob.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});





// glm::vec4 PlayMode::window_space_to_world_space(glm::uvec2 window_position){

// }

// how to calculate window space to world space transform?


Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	std::vector< float > editable_data(50000);
	for (float& value : editable_data) {
        value = 0;
    }
	return new Sound::Sample(editable_data);

});


Sound::Sample const * PlayMode::generate_audio(){
	std::vector< float > editable_data(50000);

	// random number generation code taken from here:
	// https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(-1.0,1.0);

	for (float& value : editable_data) {
        value = distribution(generator);
    }
	return new Sound::Sample(editable_data);
}

// Load< Sound::Sample > created_sample(LoadTagDefault, []() -> Sound::Sample const * {
// 	// return new Sound::Sample(data_path("dusty-floor.opus"));
// 	std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_real_distribution<float> dis(0.0, 1.0);
// 	for (float& value : editable_data) {
//         value = dis(gen);
//     }

// 	return new Sound::Sample(editable_data);
// });


// this method taken from my base 2 code:
// https://github.com/arililoia-cmu/15-466-f23-base2/blob/8697e4fed38995ac9b5949fd30c0f75dabe02444/PlayMode.cpp
glm::vec2 PlayMode::object_to_window_coordinate(Scene::Transform *object){


	glm::vec4 object_position = glm::vec4(object->position.x, object->position.y, object->position.z, 1.0f);
	glm::mat4x3 object_to_world = object->make_local_to_world();
	// object to world 3x4 x 4x1 coordinates = 3x1 world coordinates
	glm::vec3 op_world = object_to_world * object_position;

	// make a 4 vector out of op_world
	glm::vec4 op_world_vec4 = glm::vec4(op_world.x, op_world.y, op_world.z, 1.0f);

	assert(camera->transform);
	glm::mat4 world_to_clip = camera->make_projection() * glm::mat4(camera->transform->make_world_to_local());
	// world to clip 4x4 x 4x1 world coords = 4x1 clip coords
	glm::vec4 op_clip = world_to_clip * op_world_vec4;

	float ws_x = op_clip.x / op_clip.w;
	float ws_y = op_clip.y / op_clip.w;


	// The next step is to transform from this [-1, 1] space to window-relative coordinate
	// taken from this stackexchange post
	// https://stackoverflow.com/questions/8491247/c-opengl-convert-world-coords-to-screen2d-coords
	float ws_x_real = ((ws_x+1.0f)/2.0f)*1280;
	float ws_y_real = 720 - (((ws_y+1.0f)/2.0f)*720);

	return glm::vec2(ws_x_real, ws_y_real);

}

bool PlayMode::check_if_inside(glm::uvec2 check, glm::uvec2 point1, 
	glm::uvec2 point2, glm::uvec2 point3, glm::uvec2 point4){
		// min/max usage taken from:
		// https://en.cppreference.com/w/cpp/algorithm/max
		float max_x = std::max(std::max(point1.x, point2.x), std::max(point3.x, point4.x));
		float min_x = std::min(std::min(point1.x, point2.x), std::min(point3.x, point4.x));

		float max_y = std::max(std::max(point1.y, point2.y), std::max(point3.y, point4.y));
		float min_y = std::min(std::min(point1.y, point2.y), std::min(point3.y, point4.y));
		
		if (check.x >= min_x && check.x <= max_x && check.y >= min_y && check.y <= max_y){
			return true;
		}
		return false;
}


// parts of this method taken from my base 2 code:
// https://github.com/arililoia-cmu/15-466-f23-base2/blob/8697e4fed38995ac9b5949fd30c0f75dabe02444/PlayMode.cpp
PlayMode::PlayMode() : scene(*hexapod_scene) {


	for (auto &transform : scene.transforms) {
		std::cout << "transform.name: " << transform.name << std::endl;
		// if (transform.name == "Hip.FL") hip = &transform;
		if (transform.name == "Rotator") knob = &transform;
		else if (transform.name == "TopLeft") top_left = &transform;
		else if (transform.name == "TopRight") top_right = &transform;
		else if (transform.name == "BottomLeft") bottom_left = &transform;
		else if (transform.name == "BottomRight") bottom_right = &transform;
		

	}

	if (knob == nullptr) throw std::runtime_error("Knob not found.");
	// if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	// if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");
	knob_base_rotation = knob->rotation;
	knob_position = knob->position;

	tl_wpos = top_left->position;
	tr_wpos = top_right->position;
	bl_wpos = bottom_left->position;
	br_wpos = bottom_right->position;

	// std::cout << "knob_position.x,y,z: " << knob_position.x << " " << knob_position.y << " " << knob_position.z << std::endl; 
	// hip_base_rotation = hip->rotation;
	// upper_leg_base_rotation = upper_leg->rotation;
	// lower_leg_base_rotation = lower_leg->rotation;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
	// for (auto &transform : scene.transforms) {
	// 	if (transform.name == "Hip.FL") hip = &transform;
	// 	else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
	// 	else if (transform.name == "LowerLeg.FL") lower_leg = &transform;
	// }
	// if (hip == nullptr) throw std::runtime_error("Hip not found.");
	// if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	// if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");

	// hip_base_rotation = hip->rotation;
	// upper_leg_base_rotation = upper_leg->rotation;
	// lower_leg_base_rotation = lower_leg->rotation;

	// //get pointer to camera for convenience:
	// if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	// camera = &scene.cameras.front();

	
	

}

PlayMode::~PlayMode() {
}

// mode gets passed logical size bc sdl event dstructure has things related to 
bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			// std::cout << "SDL_MOUSEBUTTONDOWN - SDL_GetRelativeMouseMode = false" << std::endl;

			int mouse_x, mouse_y;
			// glm::uvec2 mouse_position;
			SDL_GetMouseState(&mouse_x, &mouse_y);
			// std::cout << "mouse_x " << mouse_x << " mouse_y " << mouse_y << std::endl;
			glm::uvec2 mouse_position(mouse_x, mouse_y);

			glm::vec2 tl_coords = object_to_window_coordinate(top_left);
			glm::vec2 tr_coords = object_to_window_coordinate(top_right);
			glm::vec2 bl_coords = object_to_window_coordinate(bottom_left);
			glm::vec2 br_coords = object_to_window_coordinate(bottom_right);
			if(check_if_inside(mouse_position, tl_coords, tr_coords, bl_coords, br_coords)){
				// std::cout << "T" << std::endl;
				clicked_inside_knob = true;
			}
			
			// else{
			// 	// std::cout << "F" << std::endl;
			// }


			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {

		if (clicked_inside_knob){
			std::cout << "aklsjdfh" << std::endl;

		}else{
			if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
				// std::cout << "SDL_GetRelativeMouseMode() == SDL_true" << std::endl;
				glm::vec2 motion = glm::vec2(
					evt.motion.xrel / float(window_size.y),
					-evt.motion.yrel / float(window_size.y)
				);
				camera->transform->rotation = glm::normalize(
					camera->transform->rotation
					* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
					* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
				);
				return true;
			}
		}
		
	} else if (evt.type == SDL_MOUSEBUTTONUP) {
		clicked_inside_knob = false;
	}

	return false;
}

void PlayMode::update(float elapsed) {

	if (clicked_inside_knob){
		std::cout << "T" << std::endl;
	}else{
		std::cout << "F" << std::endl;
	}
	// void set_volume(float new_volume, float ramp = 1.0f / 60.0f);
	// std::cout << wobble << std::endl;

	//slowly rotates through [0,1):
	// wobble += elapsed / 10.0f;
	// if (wobble >= 0.5){

	// 	leg_tip_loop = Sound::play(*generate_audio(), 1.0f, 0.0f);
	// 	wobble = 0;
	// }
	// std::cout << wobble << std::endl;
	// // wobble -= std::floor(wobble);

	// hip->rotation = hip_base_rotation * glm::angleAxis(
	// 	glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 1.0f, 0.0f)
	// );
	// upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
	// 	glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 0.0f, 1.0f)
	// );
	// lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
	// 	glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 0.0f, 1.0f)
	// );

	//move sound to follow leg tip position:
	// leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);

	// if (left.pressed && !right.pressed){
	// 	if ((ramptime - eps) > 0){
	// 		ramptime -= eps;
	// 	}
	// }
	// if (!left.pressed && right.pressed){
	// 	// std::cout << "etest" << std::endl;
	// 	ramptime += eps;
	// }
	
	// leg_tip_loop->set_volume(1.0f, 0.0f);
	// leg_tip_loop->set_volume(0.0f, ramptime);
	


	
	// std::cout << "ramptime: " << ramptime << std::endl;


	// from my game 2 code:
	// https://github.com/arililoia-cmu/15-466-f23-base2/blob/8697e4fed38995ac9b5949fd30c0f75dabe02444/PlayMode.cpp
	// int mouse_x, mouse_y;
	// // glm::uvec2 mouse_position;
	// SDL_GetMouseState(&mouse_x, &mouse_y);
	// std::cout << "mouse_x " << mouse_x << " mouse_y " << mouse_y << std::endl;
	// glm::uvec2 mouse_position(mouse_x, mouse_y);

	// glm::vec2 tl_coords = object_to_window_coordinate(top_left);
	// glm::vec2 tr_coords = object_to_window_coordinate(top_right);
	// glm::vec2 bl_coords = object_to_window_coordinate(bottom_left);
	// glm::vec2 br_coords = object_to_window_coordinate(bottom_right);
	// if(check_if_inside(mouse_position, tl_coords, tr_coords, bl_coords, br_coords)){
	// 	std::cout << " " << std::endl;
	// }else{
	// 	std::cout << "F" << std::endl;
	// }
	


	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 frame_forward = -frame[2];

		camera->transform->position += move.x * frame_right + move.y * frame_forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		// object to clip space matrix
		DrawLines wf(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));


		// need to do 
		// window space to clip space to world space


		// wf.draw(glm::uvec3(ax,ay,az), glm::uvec3(bx,by,bz));

		wf.draw(glm::vec3(-0.5f,0,0), glm::vec3(-0.5f,0.8,0));
		// repeatedly convert window space coordinates to world space
		// so that i can give the 

		// wf.draw(glm::uvec3());
		// std::cout << "(" << ax << ", " << ay << ", " << az << ") -> (" << bx << ", " << by << ", " << bz << ")" << std::endl;

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}

glm::vec3 PlayMode::get_leg_tip_position() {
	//the vertex position here was read from the model in blender:
	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}
