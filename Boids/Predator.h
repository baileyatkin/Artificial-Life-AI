#pragma once

#include "DrawableGameObject.h"

#define PREDATOR_SPEED_DEFAULT 150.0f

class Boid;

class Predator : public DrawableGameObject
{
public:
	Predator();
	~Predator();

	XMFLOAT3* GetDirection() { return &m_direction; }
	void								checkIsOnScreenAndFix(const XMMATRIX& view, const XMMATRIX& proj);
	void								update(float t, vecBoid* boidList);

protected:
	void								setDirection(XMFLOAT3 direction);

	XMFLOAT3							vecToNearbyBoids(vecBoid* boidList);
	void								createRandomDirection();

	XMFLOAT3							addFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							subtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							normaliseFloat3(XMFLOAT3& f1);
	float								magnitudeFloat3(XMFLOAT3& f1);
	XMFLOAT3							multiplyFloat3(XMFLOAT3& f1, const float scalar);
	XMFLOAT3							divideFloat3(XMFLOAT3& f1, const float scalar);

	XMFLOAT3							m_direction;

	Boid* targetedBoid = nullptr;

	float								speed = PREDATOR_SPEED_DEFAULT;
};