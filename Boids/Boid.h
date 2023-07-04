#pragma once
#include "DrawableGameObject.h"

#define SEPARATIONSCALE_DEFAULT	1.5f
#define ALIGNMENTSCALE_DEFAULT	1.0f
#define COHESIONSCALE_DEFAULT	1.0f
#define FLEESCALE_DEFAULT		10.0f

#define SPEED_DEFAULT			100.0f

class Predator;

class Boid : public DrawableGameObject
{
public:
	Boid();
	~Boid();

	XMFLOAT3*							getDirection() { return &m_direction; }
	void								checkIsOnScreenAndFix(const XMMATRIX&  view, const XMMATRIX&  proj);
	void								update(float t, vecBoid* drawList, vector<Predator*> predatorList);

	bool								getAlive() { return isAlive; }

	bool								getTargeted() { return targeted; }
	void								setTargeted(bool target) { targeted = target; }

protected:
	void								setDirection(XMFLOAT3 direction);

	vecBoid								nearbyBoids(vecBoid* boidList);
	XMFLOAT3							calculateSeparationVector(vecBoid* drawList);
	XMFLOAT3							calculateAlignmentVector(vecBoid* drawList);
	XMFLOAT3							calculateCohesionVector(vecBoid* drawList);
	XMFLOAT3							vecToNearbyBoids(vecBoid* drawList);
	XMFLOAT3							calculateFleeVector(vector<Predator*> predatorList);
	void								createRandomDirection();

	bool								compareAngle(XMFLOAT3 pos1, XMFLOAT3 pos2, float range);

	XMFLOAT3							addFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							subtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							normaliseFloat3(XMFLOAT3& f1);
	float								magnitudeFloat3(XMFLOAT3& f1);
	XMFLOAT3							multiplyFloat3(XMFLOAT3& f1, const float scalar);
	XMFLOAT3							divideFloat3(XMFLOAT3& f1, const float scalar);

	XMFLOAT3							m_direction;
	//unsigned int*						m_nearbyDrawables;

	float								separationScale = SEPARATIONSCALE_DEFAULT;
	float								alignmentScale = ALIGNMENTSCALE_DEFAULT;
	float								cohesionScale = COHESIONSCALE_DEFAULT;
	float								fleeScale = FLEESCALE_DEFAULT;
	float								fleeDistance = 10.0f;
	float								killDistance = 2.0f;
	bool								isAlive = true;

	float								speed = SPEED_DEFAULT;
	float								FOV = 45.0f;
	bool								spotPredator = false;
	const bool							canDie = true;

	//Timer*								_timer;
private:
	bool								targeted = false; // used by predators to avoid multiple predators targeting the same boid
};

