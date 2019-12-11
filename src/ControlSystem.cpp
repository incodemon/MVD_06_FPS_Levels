#include "ControlSystem.h"
#include "extern.h"

//set initial state of input system
void ControlSystem::init() {
	//set all keys and buttons to 0
	for (int i = 0; i < GLFW_KEY_LAST; i++) input[i] = 0;
}

//called from hardware input (via game)
void ControlSystem::key_mouse_callback(int key_button, int action, int mods) {

	if (action == GLFW_PRESS) input[key_button] = true;
	if (action == GLFW_RELEASE) input[key_button] = false;

}

//called from hardware input (via game)
void ControlSystem::updateMousePosition(int new_x, int new_y) {
	mouse.delta_x = new_x - mouse.x;
	mouse.delta_y = new_y - mouse.y;
	mouse.x = new_x;
	mouse.y = new_y;
}

//called once per frame
void ControlSystem::update(float dt) {
	if (control_type == ControlTypeFPS) {
		updateFPS(dt);
	}
	else {
		updateFree(dt);
	}

	//check if switch to Debug cam
	if (input[GLFW_KEY_O] == true) {
		ECS.main_camera = 0; //debug cam is 0
		control_type = ControlTypeFree;
	}
	if (input[GLFW_KEY_P] == true) {
		ECS.main_camera = 1;
		control_type = ControlTypeFPS;
	}
}

//update an entity with a free movement control component 
void ControlSystem::updateFree(float dt) {

	Camera& camera = ECS.getComponentInArray<Camera>(ECS.main_camera);
	Transform& transform = ECS.getComponentFromEntity<Transform>(camera.owner);

	//multiply speeds by delta time 
	float move_speed_dt = move_speed_ * dt;
	float turn_speed_dt = turn_speed_ * dt;

	//rotate camera if clicking the mouse - update camera.forward
	if (input[GLFW_MOUSE_BUTTON_LEFT]) {
		lm::mat4 R_yaw, R_pitch;

		//yaw - axis is up vector of world
		R_yaw.makeRotationMatrix(mouse.delta_x * turn_speed_dt, lm::vec3(0, 1, 0));
		camera.forward = R_yaw * camera.forward;

		//pitch - axis is strafe vector of camera i.e cross product of cam_forward and up
		lm::vec3 pitch_axis = camera.forward.normalize().cross(lm::vec3(0, 1, 0));
		R_pitch.makeRotationMatrix(mouse.delta_y * turn_speed_dt, pitch_axis);
		camera.forward = R_pitch * camera.forward;
	}

	lm::vec3 forward_dir = camera.forward.normalize() * move_speed_dt;
	lm::vec3 strafe_dir = camera.forward.cross(lm::vec3(0, 1, 0)) * move_speed_dt;

	if (input[GLFW_KEY_W] == true)	transform.translate(forward_dir);
	if (input[GLFW_KEY_S] == true)	transform.translate(forward_dir * -1);
	if (input[GLFW_KEY_A] == true) 	transform.translate(strafe_dir*-1);
	if (input[GLFW_KEY_D] == true) 	transform.translate(strafe_dir);

	//update camera position
	camera.position = transform.position();

}



void ControlSystem::updateFPS(float dt) {

	Camera& camera = ECS.getComponentInArray<Camera>(ECS.main_camera);
	Transform& transform = ECS.getComponentFromEntity<Transform>(camera.owner);

	//multiply speeds by delta time 
	float move_speed_dt = move_speed_ * dt;
	float turn_speed_dt = turn_speed_ * dt;

	//if (input[GLFW_MOUSE_BUTTON_LEFT]) {
		//rotate camera just like Free movement
	lm::mat4 R_yaw, R_pitch;
	//yaw - axis is up vector of world
	R_yaw.makeRotationMatrix(mouse.delta_x * turn_speed_dt, lm::vec3(0, 1, 0));
	camera.forward = R_yaw * camera.forward;
	//pitch - axis is strafe vector of camera i.e cross product of cam_forward and up
	lm::vec3 pitch_axis = camera.forward.normalize().cross(lm::vec3(0, 1, 0));
	R_pitch.makeRotationMatrix(mouse.delta_y * turn_speed_dt, pitch_axis);
	camera.forward = R_pitch * camera.forward;

	//rotate camera transform (for FPS colliders
	transform.rotateLocal(mouse.delta_x * turn_speed_dt, lm::vec3(0, 1, 0));
	//}

	//fps control should have five ray colliders assigned
	auto& colliders = ECS.getAllComponents<Collider>();
	Collider& collider_down = colliders[FPS_collider_down];
	Collider& collider_forward = colliders[FPS_collider_forward];
	Collider& collider_left = colliders[FPS_collider_left];
	Collider& collider_right = colliders[FPS_collider_right];
	Collider& collider_back = colliders[FPS_collider_back];

	lm::vec3 cp = collider_down.collision_point;
	if (collider_down.collision_distance < FPS_height + 0.01f) {

		//move up
		transform.position(cp.x, cp.y + FPS_height, cp.z);

		//we're on the ground, can start jump
		if (input[GLFW_KEY_SPACE] == true) {

			FPS_jump_force = 12.0f;
			transform.translate(0.0f, FPS_jump_force*dt, 0.0f);
		}

	}
	else {
		//in the air
		if (FPS_jump_force > 0.0)
			FPS_jump_force -= 7.0f*dt;

		transform.translate(0.0f, (FPS_jump_force - FPS_gravity)*dt, 0.0f);
	}

	// start jump by translating by initial jump force * dt
	// if already jumping, translate by jump force every frame - gravity
	// slow-down jump force every frame until 0 (then gravity takes over)

	lm::vec3 forward_dir = camera.forward.normalize() * move_speed_dt;

	forward_dir.y = 0.0f;

	lm::vec3 strafe_dir = camera.forward.cross(lm::vec3(0, 1, 0)) * move_speed_dt;

	strafe_dir.y = 0.0f;

	if (input[GLFW_KEY_W] == true && !collider_forward.colliding)
		transform.translate(forward_dir);
	if (input[GLFW_KEY_S] == true && !collider_back.colliding)
		transform.translate(forward_dir * -1);
	if (input[GLFW_KEY_A] == true && !collider_left.colliding) 	transform.translate(strafe_dir*-1);
	if (input[GLFW_KEY_D] == true && !collider_right.colliding) 	transform.translate(strafe_dir);

	//update camera position
	camera.position = transform.position();
}



