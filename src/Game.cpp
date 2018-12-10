//
//  Game.cpp
//
//  Copyright � 2018 Alun Evans. All rights reserved.
//

#include "Game.h"
#include "Shader.h"
#include "extern.h"
#include "Parsers.h"

Game::Game() {

}

int createFree(float aspect, ControlSystem& sys) {
	int ent_player = ECS.createEntity("PlayerFree");
	Camera& player_cam = ECS.createComponentForEntity<Camera>(ent_player);
	lm::vec3 the_position(10.0f, 6.0f, -5.0f);
	ECS.getComponentFromEntity<Transform>(ent_player).translate(the_position);
	player_cam.position = the_position;
	player_cam.forward = lm::vec3(-1.0f, -0.5f, 0.0f);
	player_cam.setPerspective(60.0f*DEG2RAD, aspect, 0.1f, 10000.0f);

	ECS.main_camera = ECS.getComponentID<Camera>(ent_player);

	sys.control_type = ControlTypeFree;

	return ent_player;
}

int createPlayer(float aspect, ControlSystem& sys) {
    int ent_player = ECS.createEntity("PlayerFPS");
    Camera& player_cam = ECS.createComponentForEntity<Camera>(ent_player);
	lm::vec3 the_position(0.0f, 3.0f, 5.0f);
    ECS.getComponentFromEntity<Transform>(ent_player).translate(the_position);
    player_cam.position = the_position;
    player_cam.forward = lm::vec3(0.0f, 0.0f, -1.0f);
    player_cam.setPerspective(60.0f*DEG2RAD, aspect, 0.01f, 10000.0f);
    
    //FPS colliders 
	//each collider ray entity is parented to the playerFPS entity
    int ent_down_ray = ECS.createEntity("Down Ray");
    Transform& down_ray_trans = ECS.createComponentForEntity<Transform>(ent_down_ray);
    down_ray_trans.parent = ECS.getComponentID<Transform>(ent_player); //set parent as player entity *transform*!
    Collider& down_ray_collider = ECS.createComponentForEntity<Collider>(ent_down_ray);
    down_ray_collider.collider_type = ColliderTypeRay;
    down_ray_collider.direction = lm::vec3(0.0,-1.0,0.0);
    down_ray_collider.max_distance = 100.0f;
    
    int ent_left_ray = ECS.createEntity("Left Ray");
    Transform& left_ray_trans = ECS.createComponentForEntity<Transform>(ent_left_ray);
    left_ray_trans.parent = ECS.getComponentID<Transform>(ent_player); //set parent as player entity *transform*!
    Collider& left_ray_collider = ECS.createComponentForEntity<Collider>(ent_left_ray);
    left_ray_collider.collider_type = ColliderTypeRay;
    left_ray_collider.direction = lm::vec3(-1.0,0.0,0.0);
    left_ray_collider.max_distance = 1.0f;
    
    int ent_right_ray = ECS.createEntity("Right Ray");
    Transform& right_ray_trans = ECS.createComponentForEntity<Transform>(ent_right_ray);
    right_ray_trans.parent = ECS.getComponentID<Transform>(ent_player); //set parent as player entity *transform*!
    Collider& right_ray_collider = ECS.createComponentForEntity<Collider>(ent_right_ray);
    right_ray_collider.collider_type = ColliderTypeRay;
    right_ray_collider.direction = lm::vec3(1.0,0.0,0.0);
    right_ray_collider.max_distance = 1.0f;
    
    int ent_forward_ray = ECS.createEntity("Forward Ray");
    Transform& forward_ray_trans = ECS.createComponentForEntity<Transform>(ent_forward_ray);
    forward_ray_trans.parent = ECS.getComponentID<Transform>(ent_player); //set parent as player entity *transform*!
    Collider& forward_ray_collider = ECS.createComponentForEntity<Collider>(ent_forward_ray);
    forward_ray_collider.collider_type = ColliderTypeRay;
    forward_ray_collider.direction = lm::vec3(0.0,0.0,-1.0);
    forward_ray_collider.max_distance = 1.0f;
    
    int ent_back_ray = ECS.createEntity("Back Ray");
    Transform& back_ray_trans = ECS.createComponentForEntity<Transform>(ent_back_ray);
    back_ray_trans.parent = ECS.getComponentID<Transform>(ent_player); //set parent as player entity *transform*!
    Collider& back_ray_collider = ECS.createComponentForEntity<Collider>(ent_back_ray);
    back_ray_collider.collider_type = ColliderTypeRay;
    back_ray_collider.direction = lm::vec3(0.0,0.0,1.0);
    back_ray_collider.max_distance = 1.0f;
    
    //the control system stores the FPS colliders 
	sys.FPS_collider_down = ECS.getComponentID<Collider>(ent_down_ray);
	sys.FPS_collider_left = ECS.getComponentID<Collider>(ent_left_ray);
	sys.FPS_collider_right = ECS.getComponentID<Collider>(ent_right_ray);
	sys.FPS_collider_forward = ECS.getComponentID<Collider>(ent_forward_ray);
	sys.FPS_collider_back = ECS.getComponentID<Collider>(ent_back_ray);
    
	ECS.main_camera = ECS.getComponentID<Camera>(ent_player);

	sys.control_type = ControlTypeFPS;

    return ent_player;
}

//Nothing here yet
void Game::init() {


	//******* INIT SYSTEMS *******

	//init systems except debug, which needs info about scene
	control_system_.init();
	graphics_system_.init(window_width_, window_height_);

    //******** AUTOMATIC LOADING **********//
    
	//this is a simple level parser which reads a json file and loads it into the engine
    Parsers::parseJSONLevel("data/assets/level4.json", graphics_system_);
    
	//******** MANUAL LOADING **********//

	// load a shader into the graphics system
	int gun_shader = graphics_system_.loadShader("gun_shader", "data/shaders/phong.vert", "data/shaders/phong.frag");

	// load geometry into GraphicsSystem (stored in std::vector)
	int gun_geom_id = graphics_system_.createGeometryFromFile("data/assets/gun.obj");

	//manually create material in graphics system (also stored in std::vector), get ref to it
	int gun_material_id = graphics_system_.createMaterial();
	Material& gun_material = graphics_system_.getMaterial(gun_material_id);

	//set shader it to the one we loaded above
	gun_material.shader_id = gun_shader;

	//set material diffuse map. parseTexture returns an openGL texture id
	gun_material.diffuse_map = Parsers::parseTexture("data/assets/SubmachineGun_diff_1024.tga");

	//now create entity - it has a transform component by default
	int gun_entity = ECS.createEntity("gun");
	
	//get entity transform and translate a bit as an example
	Transform& gun_transform = ECS.getComponentFromEntity<Transform>(gun_entity);
	gun_transform.translate(3, 2, 0);

	//create mesh component for entity, and plug in geometry and materials
	Mesh& gun_mesh_comp = ECS.createComponentForEntity<Mesh>(gun_entity);
	gun_mesh_comp.geometry = gun_geom_id;
	gun_mesh_comp.material = gun_material_id;

	//create box collider
	Collider& gun_collider = ECS.createComponentForEntity<Collider>(gun_entity);
	gun_collider.collider_type = ColliderTypeBox;
	gun_collider.local_halfwidth = lm::vec3(0.5, 0.5, 0.7);
	gun_collider.local_center = lm::vec3(0.2, 0.0, -0.3);


	//create another light
	//int second_light_entity = ECS.createEntity("2nd light");
	//Light& second_light = ECS.createComponentForEntity<Light>(second_light_entity);
	//second_light.color = lm::vec3(1, 1, 1);
	//ECS.getComponentFromEntity<Transform>(second_light_entity).translate(100, 100, 0);

	//there are two cameras in the scene. If you look at control_system.update
	//pressing the 'o' key switches to the 'free look' camera and control (useful for debugging)
	//while the 'p' key switches to the fps camera/control. 
	//camera 0 is debug camera, ControlTypeFree
	createFree((float)window_width_ / (float)window_height_, control_system_);

	//camera 1 is FPS camera, ControlTypeFPS
	createPlayer((float)window_width_ / (float)window_height_, control_system_);


    //******* INIT DEBUG SYSTEM *******
    debug_system_.init();
    debug_system_.setActive(true);
}

//Entry point for game update code
void Game::update(float dt) {
	//update each system in turn

	//update input
	control_system_.update(dt);

	//collision
	collision_system_.update(dt);

	//render
	graphics_system_.update(dt);
    
    //debug
    debug_system_.update(dt);
   
}
//update game viewports
void Game::update_viewports(int window_width, int window_height) {
	window_width_ = window_width;
	window_height_ = window_height;

	auto& cameras = ECS.getAllComponents<Camera>();
	for (auto& cam : cameras) {
		cam.setPerspective(60.0f*DEG2RAD, (float)window_width_ / (float) window_height_, 0.01f, 10000.0f);
	}

	graphics_system_.updateMainViewport(window_width_, window_height_);
}


