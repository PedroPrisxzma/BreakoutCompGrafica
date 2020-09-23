#include "game_object.h"
#include "ball_object.h"

enum Direction {
	UP,
	RIGHT,
	DOWN,
	LEFT
}; 
typedef std::tuple<bool, Direction, glm::vec2> Collision;    


float clamp(float value, float min, float max);
bool CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 target);