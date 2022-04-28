
#include "GameStateLevel1.h"
#include "CDT.h"


// -------------------------------------------
// Defines
// -------------------------------------------

#define MESH_MAX					32				// The total number of Mesh (Shape)
#define TEXTURE_MAX					32				// The total number of texture
#define GAME_OBJ_INST_MAX			1024			// The total number of different game object instances
#define PLAYER_INITIAL_NUM			3				// initial number of ship lives
#define NUM_ASTEROID				30
#define SHIP_ACC_FWD				150.0f			// ship forward acceleration (in m/s^2)
#define SHIP_ACC_BWD				-180.0f			// ship backward acceleration (in m/s^2)
#define SHIP_ROT_SPEED				(2.0f * PI)		// ship rotation speed (degree/second)
#define HOMING_MISSILE_ROT_SPEED	(PI / 4.0f)		// homing missile rotation speed (degree/second)
#define BULLET_SPEED				300.0f			
#define ASTEROID_SPEED				100.0f	
#define MAX_SHIP_VELOCITY			200.0f

#define SCREEN_HEIGHT 768
#define SCREEN_WIDTH 1024

enum GAMEOBJ_TYPE
{
	// list of game object types
	TYPE_SHIP = 0,
	TYPE_BULLET,
	TYPE_ASTEROID,
	TYPE_BACKGROUND,
	TYPE_MISSILE
};

#define FLAG_INACTIVE		0
#define FLAG_ACTIVE			1


// -------------------------------------------
// Structure definitions
// -------------------------------------------

struct GameObj
{
	CDTMesh* mesh;
	CDTTex* tex;
	int				type;				// enum GAMEOBJ_TYPE
	int				flag;				// 0 - inactive, 1 - active
	glm::vec3		position;			// usually we will use only x and y
	glm::vec3		velocity;			// usually we will use only x and y
	glm::vec3		scale;				// usually we will use only x and y
	float			orientation;		// 0 radians is 3 o'clock, PI/2 radian is 12 o'clock
	glm::mat4		modelMatrix;
};


// -------------------------------------------
// Level variable, static - visible only in this file
// -------------------------------------------

static CDTMesh		sMeshArray[MESH_MAX];							// Store all unique shape/mesh in your game
static int			sNumMesh;
static CDTTex		sTexArray[TEXTURE_MAX];							// Corresponding texture of the mesh
static int			sNumTex;
static GameObj		sGameObjInstArray[GAME_OBJ_INST_MAX];			// Store all game object instance
static int			sNumGameObj;

static GameObj* sPlayer;										// Pointer to the Player game object instance

static int			sPlayerLives;									// The number of lives left
static int			sScore;


// functions to create/destroy a game object instance
static GameObj* gameObjInstCreate(int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient);
static void			gameObjInstDestroy(GameObj& pInst);


// -------------------------------------------
// Game object instant functions
// -------------------------------------------

GameObj* gameObjInstCreate(int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient)
{
	// loop through all object instance array to find the free slot
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObj* pInst = sGameObjInstArray + i;

		if (pInst->flag == FLAG_INACTIVE) {

			pInst->mesh = sMeshArray + type;
			pInst->tex = sTexArray + type;
			pInst->type = type;
			pInst->flag = FLAG_ACTIVE;
			pInst->position = pos;
			pInst->velocity = vel;
			pInst->scale = scale;
			pInst->orientation = orient;
			pInst->modelMatrix = glm::mat4(1.0f);

			sNumGameObj++;
			return pInst;
		}
	}

	// Cannot find empty slot => return 0
	return NULL;
}

void gameObjInstDestroy(GameObj& pInst)
{
	// Lazy deletion, not really delete the object, just set it as inactive
	if (pInst.flag == FLAG_INACTIVE)
		return;

	sNumGameObj--;
	pInst.flag = FLAG_INACTIVE;
}


// -------------------------------------------
// Game states function
// -------------------------------------------

void GameStateLevel1Load(void) {

	// clear the Mesh array
	memset(sMeshArray, 0, sizeof(CDTMesh) * MESH_MAX);
	sNumMesh = 0;

	//+ clear the Texture array
	memset(sTexArray, 0, sizeof(CDTTex) * TEXTURE_MAX);
	sNumTex = 0;

	//+ clear the game object instance array
	for (int idx = 0; idx < GAME_OBJ_INST_MAX; idx++)
	{
		gameObjInstDestroy(sGameObjInstArray[idx]);
	}

	// Set the ship object instance to NULL
	sPlayer = NULL;


	// --------------------------------------------------------------------------
	// Create all of the unique meshes/textures and put them in MeshArray/TexArray
	//		- The order of mesh should follow enum GAMEOBJ_TYPE 
	/// --------------------------------------------------------------------------

	// Temporary variable for creating mesh
	CDTMesh* pMesh;
	CDTTex* pTex;
	std::vector<CDTVertex> vertices;
	CDTVertex v1, v2, v3, v4;

	// Create Ship mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("ship1.png");

	//+ Create Bullet mesh/texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("bullet.png");


	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	//+ Create Asteroid mesh/texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("asteroid.png");

	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 1.0f; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 1.0f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	//+ Create Background mesh/texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("space_bg1.png");

	//+ Create Missile mesh/texture
	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("missile.png");


	printf("Level1: Load\n");
}


void GameStateLevel1Init(void) {

	//+ Create the background instance
	//	- Creation order is important when rendering, so we should create the background first
	gameObjInstCreate(TYPE_BACKGROUND, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1024.0f, 768.0f, 1.0f), 0.0f);


	// Create player game object instance
	//	- the position.z should be set to 0
	//	- the scale.z should be set to 1
	//	- the velocity.z should be set to 0
	sPlayer = gameObjInstCreate(TYPE_SHIP, glm::vec3(0.0f, -GetWindowHeight() / 4, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(50.0f, 50.0f, 1.0f), 0.0f);

	//+ Create all asteroid instance, NUM_ASTEROID, with random pos and velocity
	//	- int a = rand() % 30 + 20;							// a is in the range 20-50
	//	- float b = (float)rand()/(float)(RAND_MAX);		// b is the range 0..1
	for (int idx = 0; idx < NUM_ASTEROID; idx++) // for (1,NUM_ASTEROID] index 1 for BG
	{
		float rand_x = ((SCREEN_WIDTH * (float)rand() / (float)RAND_MAX)) - SCREEN_WIDTH / 2;
		float rand_y = ((SCREEN_HEIGHT * (float)rand() / (float)RAND_MAX)) - SCREEN_HEIGHT / 2;
		float velo_x = ((2 * ASTEROID_SPEED * (float)rand() / (float)RAND_MAX)) - ASTEROID_SPEED;
		float velo_y = ((2 * ASTEROID_SPEED * (float)rand() / (float)RAND_MAX)) - ASTEROID_SPEED;
		int rand_scale = (rand() % 30) + 20;
		gameObjInstCreate(TYPE_ASTEROID, glm::vec3(rand_x, rand_y, 0.0f), glm::vec3(velo_x, velo_y, 0.0f), glm::vec3(rand_scale, rand_scale, 1.0f), 0.0f);
	}



	//+ reset the score and player life
	sPlayerLives = 3;
	sScore = PLAYER_INITIAL_NUM;

	printf("Level1: Init\n");
}


void GameStateLevel1Update(double dt, long frame, int& state) {

	// Find Target Asteroid for Homing Missile
	GameObj *target = NULL;
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++)
	{
		if (sGameObjInstArray[i].type == TYPE_ASTEROID && sGameObjInstArray[i].flag == true)
		{
			target = sGameObjInstArray + i;
			break;
		}
	}

	//-----------------------------------------
	// Get user input
	//-----------------------------------------

	// Moving the Player
	//	- WS accelereate/deaccelerate the ship
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {

		// find acceleration vector
		glm::vec3 acc = glm::vec3(SHIP_ACC_FWD * glm::cos(glm::radians(sPlayer->orientation) + PI / 2.0f),
			SHIP_ACC_FWD * glm::sin(glm::radians(sPlayer->orientation) + PI / 2.0f), 0);

		// use acceleration to change velocity
		sPlayer->velocity += acc * glm::vec3(dt, dt, 0.0f);

		//+ velocity cap to MAX_SHIP_VELOCITY
		sPlayer->velocity.x = (sPlayer->velocity.x > MAX_SHIP_VELOCITY ? MAX_SHIP_VELOCITY : sPlayer->velocity.x);
		sPlayer->velocity.y = (sPlayer->velocity.y > MAX_SHIP_VELOCITY ? MAX_SHIP_VELOCITY : sPlayer->velocity.y);

	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		// find acceleration vector
		glm::vec3 acc = glm::vec3(SHIP_ACC_BWD * glm::cos(glm::radians(sPlayer->orientation) + PI / 2.0f),
			SHIP_ACC_BWD * glm::sin(glm::radians(sPlayer->orientation) + PI / 2.0f), 0);


		// use acceleration to change velocity
		sPlayer->velocity += acc * glm::vec3(dt, dt, 0.0f);

		//+ velocity cap to MAX_SHIP_VELOCITY
		sPlayer->velocity.x = (sPlayer->velocity.x < -MAX_SHIP_VELOCITY ? -MAX_SHIP_VELOCITY : sPlayer->velocity.x);
		sPlayer->velocity.y = (sPlayer->velocity.y < -MAX_SHIP_VELOCITY ? -MAX_SHIP_VELOCITY : sPlayer->velocity.y);
	}

	//+ AD: turn the ship
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		sPlayer->orientation += SHIP_ROT_SPEED / 2;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		sPlayer->orientation -= SHIP_ROT_SPEED / 2;
	}

	// Fire bullet/missile using JK
	//	- create the bullet at the ship's position
	//	- bullet direction is the same as the ship's orientation
	//	- may use if(frame % n == 0) too slow down the bullet creation
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {

		if (frame % 5 == 0)
		{
			//+ find the bullet velocity vector
			glm::vec3 bulletVec = glm::vec3(BULLET_SPEED * glm::cos(glm::radians(sPlayer->orientation) + PI / 2.0f), 
				BULLET_SPEED * glm::sin(glm::radians(sPlayer->orientation) + PI / 2.0f), 0.0f);

			//+ call gameObjInstCreate() to create a bullet
			gameObjInstCreate(TYPE_BULLET, sPlayer->position, bulletVec, glm::vec3(25.0f, 25.0f, 1.0f), sPlayer->orientation);
		}

	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		if (frame % 10 == 0)
		{
			//+ find the bullet velocity vector
			glm::vec3 bulletVec = glm::vec3(BULLET_SPEED * glm::cos(glm::radians(sPlayer->orientation) + PI / 2.0f), BULLET_SPEED * glm::sin(glm::radians(sPlayer->orientation) + PI / 2.0f), 0.0f);

			//+ call gameObjInstCreate() to create a bullet
			gameObjInstCreate(TYPE_MISSILE, sPlayer->position, bulletVec, glm::vec3(25.0f, 25.0f, 1.0f), sPlayer->orientation);
		}
	}

	// Cam zoom UI, for Debugging
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
		ZoomIn(0.1f);
	}
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		ZoomOut(0.1f);
	}


	//---------------------------------------------------------
	// Update all game obj position using velocity 
	//---------------------------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObj* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->flag == FLAG_INACTIVE)
			continue;

		if (pInst->type == TYPE_SHIP) {

			//+ for ship: add some friction to slow it down
			sPlayer->velocity *= 0.995f;

			//+ use velocity to update the position
			sPlayer->position += glm::vec3(sPlayer->velocity.x * dt, sPlayer->velocity.y * dt, 0.0f);

		}
		else if ((pInst->type == TYPE_BULLET) || (pInst->type == TYPE_ASTEROID)) {

			//+ use velocity to update the position
			pInst->position += glm::vec3(pInst->velocity.x * dt, pInst->velocity.y * dt, 0.0f);
		}
		else if ((pInst->type == TYPE_MISSILE))
		{
			if(target != NULL)
			{
				glm::vec3 v1 = pInst->velocity;
				glm::vec3 v2 = target->position - pInst->position;
				float direction = glm::cross(v1, v2).z < 0 ? -1 : 1;
				pInst->orientation += direction * HOMING_MISSILE_ROT_SPEED;

				pInst->velocity = glm::vec3(BULLET_SPEED * glm::cos(glm::radians(pInst->orientation) + PI / 2.0f), BULLET_SPEED * glm::sin(glm::radians(pInst->orientation) + PI / 2.0f), 0.0f);
			}
			pInst->position += glm::vec3(pInst->velocity.x * dt, pInst->velocity.y * dt, 0.0f);
		}
	}


	//-----------------------------------------
	// Update some game obj behavior
	//	- wrap ship around the screen
	//	- destroy bullet that go out of the screen
	//-----------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObj* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->flag == FLAG_INACTIVE)
			continue;

		if ((pInst->type == TYPE_SHIP) || (pInst->type == TYPE_ASTEROID)) {

			//+ wrap the ship and asteroid around the screen 
			if (pInst->position.x < -SCREEN_WIDTH / 2)
			{
				pInst->position.x = SCREEN_WIDTH / 2;
			}
			else if (pInst->position.x > SCREEN_WIDTH / 2)
			{
				pInst->position.x = -SCREEN_WIDTH / 2;
			}

			if (pInst->position.y < -SCREEN_HEIGHT / 2)
			{
				pInst->position.y = SCREEN_HEIGHT / 2;
			}
			else if (pInst->position.y > SCREEN_HEIGHT / 2)
			{
				pInst->position.y = -SCREEN_HEIGHT / 2;
			}
		}
		else if (pInst->type == TYPE_BULLET || pInst->type == TYPE_MISSILE) {

			//+ call gameObjInstDestroy() on bullet that go out of the screen X [-width/2,width/2], Y [-height/2,height/2]
			if (pInst->position.x < -SCREEN_WIDTH / 2 || pInst->position.x > SCREEN_WIDTH / 2 || 
				pInst->position.y < -SCREEN_HEIGHT / 2 || pInst->position.y > SCREEN_HEIGHT / 2)
			{
				gameObjInstDestroy(*pInst);
			}
		}
	}

	//-----------------------------------------
	// Check for collsion, O(n^2)
	//-----------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObj* pInst1 = sGameObjInstArray + i;

		// skip inactive object
		if (pInst1->flag == FLAG_INACTIVE)
			continue;

		// if pInst1 is an asteroid
		if (pInst1->type == TYPE_ASTEROID) {

			// compare pInst1 with all game obj instances 
			for (int j = 0; j < GAME_OBJ_INST_MAX; j++) {
				GameObj* pInst2 = sGameObjInstArray + j;

				// skip inactive object
				if (pInst2->flag == FLAG_INACTIVE)
					continue;

				// skip asteroid object
				if (pInst2->type == TYPE_ASTEROID)
					continue;

				if (pInst2->type == TYPE_SHIP) {

					bool collide = true;

					float rightA = pInst1->position.x + pInst1->scale.x / 2;
					float leftA = pInst1->position.x - pInst1->scale.x / 2;
					float topA = pInst1->position.y + pInst1->scale.y / 2;
					float bottomA = pInst1->position.y - pInst1->scale.y / 2;

					float rightB = pInst2->position.x + pInst2->scale.x / 2;
					float leftB = pInst2->position.x - pInst2->scale.x / 2;
					float topB = pInst2->position.y + pInst2->scale.y / 2;
					float bottomB = pInst2->position.y - pInst2->scale.y / 2;

					//+ Check for collsion
					if (leftA > rightB || leftB > rightA || bottomA > topB || bottomB > topA)
					{
						collide = false;
					}


					if (collide) {

						printf("HIT\n");

						//+ Update game behavior and sGameObjInstArray
						gameObjInstDestroy(*pInst1);
						sPlayerLives--;
						if (sPlayerLives == 0)
						{
							state = 2;
						}

						break;
					}
				}
				else if (pInst2->type == TYPE_BULLET) {

					bool collide = true;

					float rightA = pInst1->position.x + pInst1->scale.x / 2;
					float leftA = pInst1->position.x - pInst1->scale.x / 2;
					float topA = pInst1->position.y + pInst1->scale.y / 2;
					float bottomA = pInst1->position.y - pInst1->scale.y / 2;

					float rightB = pInst2->position.x + pInst2->scale.x / 2;
					float leftB = pInst2->position.x - pInst2->scale.x / 2;
					float topB = pInst2->position.y + pInst2->scale.y / 2;
					float bottomB = pInst2->position.y - pInst2->scale.y / 2;

					//+ Check for collsion
					if (leftA > rightB || leftB > rightA || bottomA > topB || bottomB > topA)
					{
						collide = false;
					}

					//+ Check for collsion

					if (collide) {

						//+ Update game behavior and sGameObjInstArray
						gameObjInstDestroy(*pInst1);
						gameObjInstDestroy(*pInst2);
						break;
					}
				}
				else if (pInst2->type == TYPE_MISSILE) {

					bool collide = true;

					float rightA = pInst1->position.x + pInst1->scale.x / 2;
					float leftA = pInst1->position.x - pInst1->scale.x / 2;
					float topA = pInst1->position.y + pInst1->scale.y / 2;
					float bottomA = pInst1->position.y - pInst1->scale.y / 2;

					float rightB = pInst2->position.x + pInst2->scale.x / 2;
					float leftB = pInst2->position.x - pInst2->scale.x / 2;
					float topB = pInst2->position.y + pInst2->scale.y / 2;
					float bottomB = pInst2->position.y - pInst2->scale.y / 2;

					//+ Check for collsion
					if (leftA > rightB || leftB > rightA || bottomA > topB || bottomB > topA)
					{
						collide = false;
					}

					//+ Check for collsion

					if (collide) {

						//+ Update game behavior and sGameObjInstArray
						gameObjInstDestroy(*pInst1);
						gameObjInstDestroy(*pInst2);
						break;
					}
				}
			}
		}
	}


	//-----------------------------------------
	// Update modelMatrix of all game obj
	//-----------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObj* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->flag == FLAG_INACTIVE)
			continue;

		glm::mat4 rMat = glm::mat4(1.0f);
		glm::mat4 sMat = glm::mat4(1.0f);
		glm::mat4 tMat = glm::mat4(1.0f);

		// Compute the scaling matrix
		sMat = glm::scale(glm::mat4(1.0f), pInst->scale);

		//+ Compute the rotation matrix, we should rotate around z axis 
		rMat = glm::rotate(glm::mat4(1.0f), glm::radians(pInst->orientation), glm::vec3(0.0f, 0.0f, 1.0f));

		//+ Compute the translation matrix
		tMat = glm::translate(glm::mat4(1.0f), pInst->position);

		// Concatenate the 3 matrix to from Model Matrix
		pInst->modelMatrix = tMat * sMat * rMat;
	}

	//printf("Life> %i\n", sPlayerLives);
	//printf("Score> %i\n", sScore);
}

void GameStateLevel1Draw(void) {

	// Clear the screen
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw all game object instance in the sGameObjInstArray
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObj* pInst = sGameObjInstArray + i;

		// skip inactive object
		if (pInst->flag == FLAG_INACTIVE)
			continue;

		// 4 steps to draw sprites on the screen
		//	1. SetRenderMode()
		//	2. SetTexture()
		//	3. SetTransform()
		//	4. DrawMesh()

		SetRenderMode(CDT_TEXTURE, 1.0f);
		SetTexture(*pInst->tex, 0.0f, 0.0f);
		SetTransform(pInst->modelMatrix);
		DrawMesh(*pInst->mesh);
	}
	// Swap the buffer, to present the drawing
	glfwSwapBuffers(window);
}

void GameStateLevel1Free(void) {

	//+ call gameObjInstDestroy for all object instances in the sGameObjInstArray
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++)
	{
		gameObjInstDestroy(sGameObjInstArray[i]);
	}

	// reset camera
	ResetCam();

	printf("Level1: Free\n");
}

void GameStateLevel1Unload(void) {

	// Unload all meshes in MeshArray
	for (int i = 0; i < sNumMesh; i++) {
		UnloadMesh(sMeshArray[i]);
	}

	//+ Unload all textures in TexArray
	for (int i = 0; i < sNumTex; i++)
	{
		TextureUnload(sTexArray[i]);
	}


	printf("Level1: Unload\n");
}
