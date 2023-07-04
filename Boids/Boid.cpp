#include "Boid.h"
#include "Predator.h"

#define NEARBY_DISTANCE		200.0f	// how far boids can see

Boid::Boid()
{
	m_scale = 1.0f;
	speed = SPEED_DEFAULT + (rand() % 100);
	FOV += (rand() % 225);
	fleeDistance += (rand() % 100);
	createRandomDirection();
}

Boid::~Boid()
{
}

void Boid::createRandomDirection()
{
	float x = (float)(rand() % 10);
	x -= 5;
	float y = (float)(rand() % 10);
	y -= 5;
	float z = 0;
	setDirection(XMFLOAT3(x, y, z));
}

void Boid::setDirection(XMFLOAT3 direction)
{
	XMVECTOR v = XMLoadFloat3(&direction);
	v = XMVector3Normalize(v);
	XMStoreFloat3(&m_direction, v);
}


#define DIRECTION_DELTA 0.1f
void Boid::update(float t, vecBoid* boidList, vector<Predator*> predatorList)
{
	// create a list of nearby boids
	vecBoid nearBoids = nearbyBoids(boidList);

	// NOTE these functions should always return a normalised vector
	XMFLOAT3  vSeparation = calculateSeparationVector(&nearBoids);
	XMFLOAT3  vAlignment = calculateAlignmentVector(&nearBoids);
	XMFLOAT3  vCohesion = calculateCohesionVector(&nearBoids);
	XMFLOAT3 vFlee = calculateFleeVector(predatorList);

	// each vector is multiplied by their to make some more important than others
	vSeparation = multiplyFloat3(vSeparation, separationScale);
	vAlignment = multiplyFloat3(vAlignment, alignmentScale);
	vCohesion = multiplyFloat3(vCohesion, cohesionScale);
	vFlee = multiplyFloat3(vFlee, fleeScale);

	// all four is added together and normalized
	XMFLOAT3 forces = XMFLOAT3(0.0f, 0.0f, 0.0f);
	forces = addFloat3(forces, vSeparation);
	forces = addFloat3(forces, vAlignment);
	forces = addFloat3(forces, vCohesion);
	forces = addFloat3(forces, vFlee);
	m_direction = addFloat3(m_direction, forces);
	if (magnitudeFloat3(m_direction) != 0)
	{
		m_direction = normaliseFloat3(m_direction);
	}
	else
	{
		m_direction = vecToNearbyBoids(boidList); // no direction then go to nearest boid

		if (magnitudeFloat3(m_direction) == 0) // still no direction so create a random direction to travel in
			createRandomDirection();
	}

	XMFLOAT3 dir = multiplyFloat3(m_direction, t * speed);
	m_position = addFloat3(m_position, dir);

	m_position.z = 0;
	m_direction.z = 0;

	DrawableGameObject::update(t);
}

XMFLOAT3 Boid::calculateSeparationVector(vecBoid* boidList)
{
	if (boidList == nullptr)
		return XMFLOAT3(0, 0, 0);

	float desiredSeparation = 12.5f;
	XMFLOAT3 nearby = XMFLOAT3(0, 0, 0);
	int count = 0;

	for (Boid* b : *boidList)
	{
		// find the distance between boids
		XMFLOAT3 vB = *(b->getPosition());
		XMFLOAT3 vDiff = subtractFloat3(m_position, vB);
		float l = magnitudeFloat3(vDiff);

		// ignore self (distance of 0 could only be self)
		if (l < 0)
		{
			continue;
		}

		// only separate from boids in the desired distance
		if (l < desiredSeparation)
		{
			XMFLOAT3 dif = normaliseFloat3(vDiff);
			dif = divideFloat3(dif, l); // closer boids will have a greater weight
			nearby = addFloat3(nearby, dif);

			count++;
		}
	}

	if (magnitudeFloat3(nearby) > 0)
	{
		nearby = divideFloat3(nearby, count);
		nearby = normaliseFloat3(nearby);
		return nearby;
	}

	return m_direction;
}

XMFLOAT3 Boid::calculateAlignmentVector(vecBoid* boidList)
{
	XMFLOAT3 nearby = XMFLOAT3(0, 0, 0);
	if (boidList == nullptr)
		return nearby;

	for (Boid* b : *boidList)
	{
		nearby = addFloat3(*b->getDirection(), nearby);
	}
	if (boidList->size() > 0)
	{
		nearby = divideFloat3(nearby, boidList->size());

		return normaliseFloat3(nearby); // return the normalised (average) direction of nearby drawables
	}
	return m_direction;
}

XMFLOAT3 Boid::calculateCohesionVector(vecBoid* boidList)
{
	XMFLOAT3 nearby = XMFLOAT3(0, 0, 0);

	if (boidList == nullptr)
		return nearby;

	// calculate average position of nearby
	for (Boid* boid : *boidList)
	{
		nearby = addFloat3(*boid->getPosition(), nearby);
	}
	if (boidList->size() > 0)
	{
		nearby = divideFloat3(nearby, boidList->size()); // this is the avg position

		nearby = subtractFloat3(nearby, m_position); // this gets the direction to the avg position

		return normaliseFloat3(nearby); // nearby is the direction to where the other drawables are
	}
	return m_direction;
}

XMFLOAT3 Boid::vecToNearbyBoids(vecBoid* boidList)
{
	if (boidList == nullptr)
		return XMFLOAT3(0, 0, 0);

	// work out which is nearest fish, and calculate a vector away from that
	Boid* nearest = nullptr;
	XMFLOAT3 directionNearest;
	float shortestDistance = FLT_MAX;

	for (Boid* b : *boidList)
	{
		// ignore self
		if (b == this)
			continue;

		if (nearest == nullptr)
		{
			nearest = b;
		}
		else
		{
			// calculate the distance to each boid and find the shortest
			XMFLOAT3 vB = *(b->getPosition());
			XMFLOAT3 vDiff = subtractFloat3(m_position, vB);
			float l = magnitudeFloat3(vDiff);
			if (l < shortestDistance)
			{
				shortestDistance = l;
				nearest = b;
			}
		}
	}

	if (nearest != nullptr)
	{
		// get the direction from nearest boid to current boid
		directionNearest = subtractFloat3(*nearest->getPosition(), m_position);
		return normaliseFloat3(directionNearest);
	}

	// if there is not a nearby fish return current direction
	return m_direction;
}

XMFLOAT3 Boid::calculateFleeVector(vector<Predator*> predatorList)
{
	if (predatorList.empty())
		return XMFLOAT3(0, 0, 0);

	XMFLOAT3 dir = XMFLOAT3(0, 0, 0);

	for (Predator* p : predatorList)
	{
		// calculate the distance to each predator and flee if too close
		XMFLOAT3 vP = *(p->getPosition());
		XMFLOAT3 vDiff = subtractFloat3(m_position, vP);

		float l = magnitudeFloat3(vDiff);
		if (l > killDistance)
		{
			spotPredator = false;
			if (l < fleeDistance)
			{
				XMFLOAT3 toPredator = subtractFloat3(vP, m_position);
				if (compareAngle(m_direction, toPredator, FOV))
				{
					spotPredator = true;
				}
			}

			if (spotPredator)
			{
				dir = addFloat3(dir, vDiff);
			}
		}
		else
		{
			if (canDie)
				isAlive = false;
			else
				dir = addFloat3(dir, vDiff);
		}
	}
	if (magnitudeFloat3(dir) > 0)
		return dir;

	return m_direction;
}

bool Boid::compareAngle(XMFLOAT3 pos1, XMFLOAT3 pos2, float range)
{
	// Get the angle in degrees from position Vectors
	float n1 = 270 - atan2(pos1.y, pos1.x) * 180 / XM_PI;
	float angle1 = fmod(n1, 360);

	float n2 = 270 - atan2(pos2.y, pos2.x) * 180 / XM_PI;
	float angle2 = fmod(n2, 360);

	float lower = angle1 - (range * 0.5f);
	float upper = angle1 + (range * 0.5f);

	// If lower goes below 0, add 360 to it, if upper goes above 360, subtract 360 from it
	if (lower < 0.0f)
	{
		lower += 360.0f;
	}
	if (upper > 360.0f)
	{
		upper -= 360.0f;
	}

	//Check to see if each angle is in range
	bool inRange = false;
	if (lower <= angle2 && angle2 <= upper)
	{
		inRange = true;
	}
	else if (upper - lower <= 0.0f)
	{
		// Check if either upper or lower have looped around
		if (lower <= angle2 && angle2 <= 360.0f)
		{
			// If angle is between lower and 360, then in range
			inRange = true;
		}
		else if (0.0f <= angle2 && angle2 <= upper)
		{
			//If angle is between 0 and upper then in range
			inRange = true;
		}
	}

	if (inRange)
	{
		/*Debug::Print("Lower: " + to_string(lower));
		Debug::Print("Upper: " + to_string(upper));
		Debug::Print("Angle: " + to_string(angle2));*/
		return true;
	}

	/*Debug::Print("Lower: " + to_string(lower));
	Debug::Print("Upper: " + to_string(upper));
	Debug::Print("Angle: " + to_string(angle2));*/

	// angle not in range - return false
	return false;
}



// use but don't alter the methods below

XMFLOAT3 Boid::addFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x + f2.x;
	out.y = f1.y + f2.y;
	out.z = f1.z + f2.z;

	return out;
}

XMFLOAT3 Boid::subtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x - f2.x;
	out.y = f1.y - f2.y;
	out.z = f1.z - f2.z;

	return out;
}

XMFLOAT3 Boid::multiplyFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x * scalar;
	out.y = f1.y * scalar;
	out.z = f1.z * scalar;

	return out;
}

XMFLOAT3 Boid::divideFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x / scalar;
	out.y = f1.y / scalar;
	out.z = f1.z / scalar;

	return out;
}

float Boid::magnitudeFloat3(XMFLOAT3& f1)
{
	return sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));
}

XMFLOAT3 Boid::normaliseFloat3(XMFLOAT3& f1)
{
	float length = sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));

	f1.x /= length;
	f1.y /= length;
	f1.z /= length;

	return f1;
}

vecBoid Boid::nearbyBoids(vecBoid* boidList)
{
	vecBoid nearBoids;
	if (boidList->size() == 0)
		return nearBoids;

	for (Boid* boid : *boidList) {
		// ignore self
		if (boid == this)
			continue;

		// get the distance between the two
		XMFLOAT3 vB = *(boid->getPosition());
		XMFLOAT3 vDiff = subtractFloat3(m_position, vB);
		float l = magnitudeFloat3(vDiff);
		if (l < NEARBY_DISTANCE) {
			nearBoids.push_back(boid);
		}
	}

	return nearBoids;
}


void Boid::checkIsOnScreenAndFix(const XMMATRIX&  view, const XMMATRIX&  proj)
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

		// throw a bit of randomness into the mix
		//createRandomDirection();

		// method 1 - appear on the other side
		m_position.x = v4.x;
		m_position.y = v4.y;
		m_position.z = v4.z;

		// method2 - bounce off sides and head to centre
		/*if (v.x < -1 || v.x > 1 || v.y < -1 || v.y > 1)
		{
			m_direction = multiplyFloat3(m_direction, -1);;
			m_direction = normaliseFloat3(m_direction);
		}*/
	}

	return;
}