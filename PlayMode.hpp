#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>




struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	Sound::Sample const * generate_audio();
	glm::vec2 object_to_window_coordinate(Scene::Transform *object);
	glm::vec2 object_position_to_window_position(glm::vec3 object_position);
	bool check_if_inside(glm::uvec2 check, glm::uvec2 point1, 
		glm::uvec2 point2, glm::uvec2 point3, glm::uvec2 point4);
	bool is_left_side_of_knob(glm::uvec2 knob_coords, glm::uvec2 mouse_position);

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;


	// bool play_sound_again = true;
	

	int wf_middle = 30;
	int wf_amplitude = 30;
	int wf_width = 200;

	Scene::Transform *knob = nullptr;

	Scene::Transform *top_left = nullptr;
	Scene::Transform *top_right = nullptr;
	Scene::Transform *bottom_left = nullptr;
	Scene::Transform *bottom_right = nullptr;


	// Scene::Drawable *top_left_drawable = nullptr;
	// Scene::Drawable *top_right_drawable = nullptr;
	// Scene::Drawable *bottom_left_drawable = nullptr;
	// Scene::Drawable *bottom_right_drawable = nullptr;


	glm::quat knob_base_rotation;
	glm::vec3 knob_position;

	glm::vec3 tl_wpos;
	glm::vec3 tr_wpos;
	glm::vec3 bl_wpos;
	glm::vec3 br_wpos;

	glm::vec2 tl_pos;
	glm::vec2 tr_pos;
	glm::vec2 bl_pos;
	glm::vec2 br_pos;

	int goal_angle = 100;
	int current_angle;
	int points = 0;

	bool clicked_inside_knob = false;




	
	//hexapod leg to wobble:
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;
	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;
	float wobble = 0.6f;
	float ramptime = 0.01f;
	float eps = 0.00001;


	// float ax = 0;
	// float ay = 0;
	// float az = 0;
	// float bx = 0;
	// float by = 1;
	// float bz = 0;
	glm::vec3 get_leg_tip_position();

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
