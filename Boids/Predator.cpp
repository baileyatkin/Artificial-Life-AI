#include "Predator.h"
#include "Boid.h"

Predator::Predator()
{
	m_scale = 3.0f;
	createRandomDirection();
}

Predator::~Predator()
{
	if (targetedBoid != nullptr) targetedBoid = nullptr; delete targetedBoid;
}

void Predator::createRandomDirection()
{
	float x = (float)(rand() % 10);
	x -= 5;
	float y = (float)(rand() % 10);
	y -= 5;
	float z = 0;
	setDirection(XMFLOAT3(x, y, z));
}

void Predator::setDirection(XMFLOAT3 direction)
{
	XMVECTOR v = XMLoadFloat3(&direction);
	v = XMVector3Normalize(v);
	XMStoreFloat3(&m_direction, v);
}

void Predator::update(float t, vecBoid* boidList)
{
	XMFLOAT3 nearbyBoidsVec = vecToNearbyBoids(boidList);
	m_direction = addFloat3(m_direction, nearbyBoidsVec);
	//m_direction = VecToNearbyBoids(boidList);

	if (magnitudeFloat3(m_direction) != 0)
	{
		m_direction = normaliseFloat3(m_direction);
	}
	else
	{
		createRandomDirection(); // if no direction, make one
	}

	XMFLOAT3 dir = multiplyFloat3(m_direction, t * speed);
	m_position = addFloat3(m_position, dir);

	m_position.z = 0;
	m_direction.z = 0;

	DrawableGameObject::update(t);
}

XMFLOAT3 Predator::vecToNearbyBoids(vecBoid* boidList)
{
	XMFLOAT3 nearby = XMFLOAT3(0, 0, 0);

	for (Boid* b : *boidList)
	{
		XMFLOAT3 vB = *(b->getPosition());
		XMFLOAT3 vDiff = subtractFloat3(vB, m_position);
		float l = magnitudeFloat3(vDiff);

		vDiff = normaliseFloat3(vDiff);
		vDiff = divideFloat3(vDiff, l); // closer boids will have a greater weight
		nearby = addFloat3(nearby, vDiff);
	}

	if (magnitudeFloat3(nearby) > 0)
	{
		nearby = divideFloat3(nearby, boidList->size());
		nearby = normaliseFloat3(nearby);
	}

	return nearby;
}

XMFLOAT3 Predator::addFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x + f2.x;
	out.y = f1.y + f2.y;
	out.z = f1.z + f2.z;

	return out;
}

XMFLOAT3 Predator::subtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x - f2.x;
	out.y = f1.y - f2.y;
	out.z = f1.z - f2.z;

	return out;
}

XMFLOAT3 Predator::multiplyFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x * scalar;
	out.y = f1.y * scalar;
	out.z = f1.z * scalar;

	return out;
}

XMFLOAT3 Predator::divideFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x / scalar;
	out.y = f1.y / scalar;
	out.z = f1.z / scalar;

	return out;
}

float Predator::magnitudeFloat3(XMFLOAT3& f1)
{
	return sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));
}

XMFLOAT3 Predator::normaliseFloat3(XMFLOAT3& f1)
{
	float length = sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));

	f1.x /= length;
	f1.y /= length;
	f1.z /= length;

	return f1;
}

void Predator::checkIsOnScreenAndFix(const XMMATRIX& view, const XMMATRIX& proj)
{
	XMFLOAT4 v4;
	v4.x = m_position.x;
	v4.y = m_position.y;
	v4.z = m_position.z;
	v4.w = 1.0f;

	XMVECTOR vScreenSpace = XMLoadFloat4(&v4);
	XMVECTOR vScreenSpace2 = XMVector4Transform(vScreenSpace, view);
	XMVECTOR vScreenSpace3 = XMVector4Transform(vScreenSpace2, proj);

	XMFLOAT4 v;
	XMStoreFloat4(&v, vScreenSpace3);
	v.x /= v.w;
	v.y /= v.w;
	v.z /= v.w;
	v.w /= v.w;

	float fOffset = 10; // a suitable distance to rectify position within clip space
	if (v.x < -1 || v.x > 1 || v.y < -1 || v.y > 1)
	{
		if (v.x < -1 || v.x > 1) {
			v4.x = -v4.x + (fOffset * v.x);
		}
		else if (v.y < -1 || v.y > 1) {
			v4.y = -v4.y + (fOffset * v.y);
		}
	}

	m_position.x = v4.x;
	m_position.y = v4.y;
	m_position.z = v4.z;

	return;
}