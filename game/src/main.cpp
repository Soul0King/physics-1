/*
This project uses the Raylib framework to provide us functionality for math, graphics, GUI, input etc.
See documentation here: https://www.raylib.com/, and examples here: https://www.raylib.com/examples.html
*/

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "game.h"
#include <string>
#include <vector>
#include <cmath>

const unsigned int TARGET_FPS = 50;
float dt = 1.0f / TARGET_FPS;
float time = 0;

float restitution = 0.9f;
float coefficientOfFriction = 1.0f;

enum FizziksShape {
	CIRCLE,
	HALF_SPACE,
	AABB
};

class FizziksObjekt {

public:
	bool isStatic = false;
	Vector2 position = { 0,0 };
	Vector2 velocity = { 0,0 };
	float mass = 1; // in kg
	Vector2 netForce = { 0,0 };
	float grippiness = 0.5f;

	float bounciness = 0.9f; // for determining coefficient of restitution

	std::string name = "objekt";
	Color color = GREEN;
	Color baseColor = GREEN;

	virtual void draw() {
		DrawCircle(position.x, position.y, 2, color);
	}

	virtual FizziksShape Shape() = 0;
};



class FizziksCircle : public FizziksObjekt {
public:
	float radius = 15; // circle radius in pixels

	

	void draw() override {
		DrawCircle(position.x, position.y, radius, color);
		DrawLineEx(position, position + velocity, 1, color);
	}

	FizziksShape Shape() override
	{
		return CIRCLE;
	}
};

class FizziksHalfspace : public FizziksObjekt {
private:
	float rotation = 0;
	Vector2 normal = { 0, -1 };

public:

	void setRotationDegrees(float rotationDegrees) {
		rotation = rotationDegrees;
		normal = Vector2Rotate({ 0, -1 }, rotation * DEG2RAD);
	}

	float getRotation() {
		return rotation;
	}

	Vector2 getNormal() {
		return normal;
	}

	void draw() override {
		DrawCircle(position.x, position.y, 8, color);

		DrawLineEx(position, position + normal * 30, 1, color);

		Vector2 parallelToSurface = Vector2Rotate(normal, PI * 0.5f);
		DrawLineEx(position - parallelToSurface * 4000, position + parallelToSurface * 4000, 1, color);
	}
	FizziksShape Shape() override
	{
		return HALF_SPACE;
	}
};

class FizziksAABB : public FizziksObjekt {
public:
	Vector2 sizeXY = { 10,10 };
	Vector2 maxCoords = { position.x + sizeXY.x, position.y + sizeXY.y };
	
	void draw() override {
		DrawRectangle(position.x, position.y, sizeXY.x, sizeXY.y, color);
	}

	FizziksShape Shape() override
	{
		return AABB;
	}
};

bool CircleCircleOverlap(FizziksCircle* circleA, FizziksCircle* circleB);
bool CircleHalfspaceOverlap(FizziksCircle* circle, FizziksHalfspace* halfspace);
bool AABBAABBOverlap(FizziksAABB* aabbA, FizziksAABB* aabbB);
bool AABBCircleOverlap(FizziksAABB* aabb, FizziksCircle* circle);
bool AABBHalfspaceOverlap(FizziksAABB* aabb, FizziksHalfspace* halfspace);

class FizziksWorld {
public:
	std::vector<FizziksObjekt*> objekts;

	Vector2 accelerationGravity = { 0, 50 };

	void add(FizziksObjekt* newObject) {
		objekts.push_back(newObject);
	}

	void resetNetForces() {
		for (int i = 0; i < objekts.size(); i++) {
			objekts[i]->netForce = { 0,0 };
		}
	}

	void addGravityForces() {
		for (int i = 0; i < objekts.size(); i++) {

			FizziksObjekt* objekt = objekts[i];

			if (objekt->isStatic) continue;

			Vector2 FGravity = accelerationGravity * objekt->mass;
			objekt->netForce += FGravity;
			DrawLineEx(objekt->position, objekt->position + FGravity, 1, PURPLE);
		}
	}

	void applyKinematics() {
		for (int i = 0; i < objekts.size(); i++) {

			FizziksObjekt* objekt = objekts[i];

			if (objekt->isStatic) continue;

			objekt->position = objekt->position + objekt->velocity * dt;

			Vector2 acceleration = objekt->netForce / objekt->mass; //a = F/m

			objekt->velocity = objekt->velocity + acceleration * dt;

		}
	}

	void update() {
		

		resetNetForces();

		addGravityForces();

		checkCollisions();

		applyKinematics();

	}

	void checkCollisions() {
		std::vector<bool> isColliding(objekts.size(), false);

		for (int i = 0; i < objekts.size(); i++) {
			for (int j = i + 1; j < objekts.size(); j++) {

				FizziksObjekt* objektPointerA = objekts[i];
				FizziksObjekt* objektPointerB = objekts[j];

				FizziksShape shapeOfA = objektPointerA->Shape();
				FizziksShape shapeOfB = objektPointerB->Shape();

				if (shapeOfA == CIRCLE && shapeOfB == CIRCLE) {

					if (CircleCircleOverlap((FizziksCircle*)objektPointerA, (FizziksCircle*)objektPointerB)) {
						isColliding[i] = true;
						isColliding[j] = true;
					}
				}
				else if (shapeOfA == CIRCLE && shapeOfB == HALF_SPACE) {
					if (CircleHalfspaceOverlap((FizziksCircle*)objektPointerA, (FizziksHalfspace*)objektPointerB)) {
						isColliding[i] = true;
						isColliding[j] = true;
					}
				}
				else if (shapeOfA == HALF_SPACE && shapeOfB == CIRCLE) {
					if (CircleHalfspaceOverlap((FizziksCircle*)objektPointerB, (FizziksHalfspace*)objektPointerA)) {
						isColliding[i] = true;
						isColliding[j] = true;
					}
				}
				else if (shapeOfA == AABB && shapeOfB == AABB) {
					if (AABBAABBOverlap((FizziksAABB*)objektPointerA, (FizziksAABB*)objektPointerB)) {
						isColliding[i] = true;
						isColliding[j] = true;
					}
				}
				else if (shapeOfA == AABB && shapeOfB == CIRCLE) {
					if (AABBCircleOverlap((FizziksAABB*)objektPointerA, (FizziksCircle*)objektPointerB)) {
						isColliding[i] = true;
						isColliding[j] = true;
					}
				}
				else if (shapeOfA == CIRCLE && shapeOfB == AABB) {
					if (AABBCircleOverlap((FizziksAABB*)objektPointerB, (FizziksCircle*)objektPointerA)) {
						isColliding[i] = true;
						isColliding[j] = true;
					}
				}
				else if (shapeOfA == AABB && shapeOfB == HALF_SPACE) {
					if (AABBHalfspaceOverlap((FizziksAABB*)objektPointerA, (FizziksHalfspace*)objektPointerB)) {
						isColliding[i] = true;
						isColliding[j] = true;
					}
				}
				else if (shapeOfA == HALF_SPACE && shapeOfB == AABB) {
					if (AABBHalfspaceOverlap((FizziksAABB*)objektPointerB, (FizziksHalfspace*)objektPointerA)) {
						isColliding[i] = true;
						isColliding[j] = true;
					}
				}
			}
		}
		
		for (int i = 0; i < objekts.size(); i++) {
			objekts[i]->color = isColliding[i] ? RED : objekts[i]->baseColor;
		}
	}
};

float speed = 100;
float angle = 0;
float startX = 100;
float startY = 500;


FizziksWorld world;
FizziksHalfspace halfspace;


void MakeDeleteableObjekts() {
	
	FizziksAABB* aabb = new FizziksAABB();
	FizziksAABB* aabb1 = new FizziksAABB();
	FizziksAABB* aabb2 = new FizziksAABB();

	aabb->position = { 400, 600 };
	aabb->sizeXY = { 150, 50 };
	aabb->velocity = { 0, 0 };
	aabb->color = GREEN;
	aabb->baseColor = GREEN;
	aabb->isStatic = true;
	world.add(aabb);

	aabb1->position = { 450, 400 };
	aabb1->sizeXY = { 50, 100 };
	aabb1->velocity = { 0, 0 };
	aabb1->color = BLUE;
	aabb1->baseColor = BLUE;
	world.add(aabb1);

	aabb2->position = { 400, 200 };
	aabb2->sizeXY = { 150, 150 };
	aabb2->velocity = { 0, 0 };
	aabb2->color = YELLOW;
	aabb2->baseColor = YELLOW;
	world.add(aabb2);


}


bool CircleCircleOverlap(FizziksCircle* circleA, FizziksCircle* circleB) {
	Vector2 displacementFromAToB = circleB->position - circleA->position;
	float distance = Vector2Length(displacementFromAToB);
	float sumOfRadii = circleA->radius + circleB->radius;
	float overlap = sumOfRadii - distance;


	if (overlap > 0) {
		Vector2 normalAToB;
		if (abs(distance) < 0.0001f) {
			normalAToB = { 0,1 };
		}
		else
			normalAToB = displacementFromAToB / distance;

		Vector2 mtv = normalAToB * overlap;

		circleA->position -= mtv * 0.5f;
		circleB->position += mtv * 0.5f;

		//from perspective of A
		Vector2 velocityBRelativeToA = circleB->velocity - circleA->velocity;
		float closingVelocity1D = Vector2DotProduct(velocityBRelativeToA, normalAToB);
		//if dot is negative then we are coliding. if positive, not colliding
		if (closingVelocity1D >= 0) return true;

		float restitution = circleA->bounciness * circleB->bounciness;

		float totalMass = circleA->mass + circleB->mass;
		float impulseMagnitude = ((1.0f + restitution) * closingVelocity1D * circleA->mass * circleB->mass) / totalMass;
		//A-->  <-B 
		Vector2 impulseForA = normalAToB * impulseMagnitude;
		Vector2 impulseForB = normalAToB * -impulseMagnitude;

		//apply impulse
		circleA->velocity += impulseForA / circleA->mass;
		circleB->velocity += impulseForB / circleB->mass;

		return true;
	}
	else
		return false;
}

bool CircleHalfspaceOverlap(FizziksCircle* circle, FizziksHalfspace* halfspace) {

	Vector2 displacementToCircle = circle->position - halfspace->position;

	float dot = Vector2DotProduct(displacementToCircle, halfspace->getNormal());
	Vector2 vectorProjection = halfspace->getNormal() * dot;


	Vector2 midpoint = circle->position - vectorProjection * 0.5f;

	float overlap = circle->radius - dot;


	if (overlap > 0) {
		Vector2 normalAToB = (vectorProjection / dot);
		Vector2 mtv = normalAToB * overlap;

		circle->position += mtv;

		//get gravity force
		Vector2 Fgravity = world.accelerationGravity * circle->mass;

		//apply normal force

		Vector2 FgPerp = halfspace->getNormal() * Vector2DotProduct(Fgravity, halfspace->getNormal());
		Vector2 Fnormal = FgPerp * -1;
		circle->netForce += Fnormal;
		DrawLineEx(circle->position, circle->position + Fnormal, 1, GREEN);

		//friction
		//f = uN
		float u = circle->grippiness * halfspace->grippiness;
		float frictionMagnitude = u * Vector2Length(Fnormal);

		Vector2 FgPara = Fgravity - FgPerp;

		Vector2 frictionDirection;
		if (FgPara.x > 0) {
			frictionDirection = Vector2Normalize(FgPara) * -1;
			frictionMagnitude = Clamp(frictionMagnitude, 0.0f, Vector2Length(FgPara)); // frictionMagnitude cant be more than FgPara
		}
		else {
			frictionDirection = Vector2Normalize(circle->velocity) * -1;
			frictionMagnitude = Clamp(frictionMagnitude, 0.0f, Vector2Length(circle->velocity));
		}

		
		

		Vector2 Ffriction = frictionDirection * frictionMagnitude;

		circle->netForce += Ffriction;
		DrawLineEx(circle->position, circle->position + Ffriction, 2, ORANGE);


		//Bouncing!
		//from perspective of A
		float closingVelocity1D = Vector2DotProduct(circle->velocity, halfspace->getNormal());
		//if dot is negative then we are coliding. if positive, not colliding
		if (closingVelocity1D >= -2) return true;

		float restitution = circle->bounciness * halfspace->bounciness;
		circle->velocity += halfspace->getNormal() * closingVelocity1D * -(1.0f + restitution);
		


		return true;
	}
	else return false;
}


bool AABBAABBOverlap(FizziksAABB* aabbA, FizziksAABB* aabbB) {
	Vector2 cA = { aabbA->position.x + aabbA->sizeXY.x * 0.5f, aabbA->position.y + aabbA->sizeXY.y * 0.5f };
	Vector2 cB = { aabbB->position.x + aabbB->sizeXY.x * 0.5f, aabbB->position.y + aabbB->sizeXY.y * 0.5f };

	float halfWidthA = aabbA->sizeXY.x * 0.5f;
	float halfWidthB = aabbB->sizeXY.x * 0.5f;
	float halfHeightA = aabbA->sizeXY.y * 0.5f;
	float halfHeightB = aabbB->sizeXY.y * 0.5f;

	Vector2 d = { cB.x - cA.x, cB.y - cA.y };

	float overlapX = (halfWidthA + halfWidthB) - fabsf(d.x);
	float overlapY = (halfHeightA + halfHeightB) - fabsf(d.y);

	if (overlapX > 0.0f && overlapY > 0.0f) {
		// choose the smaller overlap to separate along that axis
		bool separateOnX = overlapX < overlapY;

		if (separateOnX) {
			float sign = (d.x >= 0.0f) ? 1.0f : -1.0f;
			float push = overlapX * sign; 

			// If one is static move the other fully otherwise split equally
			if (aabbA->isStatic && !aabbB->isStatic) {
				aabbB->position.x += push;
			}
			else if (!aabbA->isStatic && aabbB->isStatic) {
				aabbA->position.x -= push;
			}
			else {
				aabbA->position.x -= push * 0.5f;
				aabbB->position.x += push * 0.5f;
			}

			if (aabbA->isStatic && !aabbB->isStatic) {
				aabbB->velocity.x = 0.0f;
			}
			else if (!aabbA->isStatic && aabbB->isStatic) {
				aabbA->velocity.x = 0.0f;
			}
			else {
				float vxA = aabbA->velocity.x;
				float vxB = aabbB->velocity.x;
				aabbA->velocity.x = vxB;
				aabbB->velocity.x = vxA;
			}
			
		}
		else {
			float sign = (d.y >= 0.0f) ? 1.0f : -1.0f;
			float push = overlapY * sign;

			if (aabbA->isStatic && !aabbB->isStatic) {
				aabbB->position.y += push;
			}
			else if (!aabbA->isStatic && aabbB->isStatic) {
				aabbA->position.y -= push;
			}
			else {
				aabbA->position.y -= push * 0.5f;
				aabbB->position.y += push * 0.5f;
			}
			if (aabbA->isStatic && !aabbB->isStatic) {
				aabbB->velocity.y = 0.0f;
			}
			else if (!aabbA->isStatic && aabbB->isStatic) {
				aabbA->velocity.y = 0.0f;
			}
			else {
				float vyA = aabbA->velocity.y;
				float vyB = aabbB->velocity.y;
				aabbA->velocity.y = vyB;
				aabbB->velocity.y = vyA;
			}
		}

		return true;
	}

	return false;
}

bool AABBCircleOverlap(FizziksAABB* aabb, FizziksCircle* circle) {
	Vector2 aMin = aabb->position;
	Vector2 aMax = { aabb->position.x + aabb->sizeXY.x, aabb->position.y + aabb->sizeXY.y };

	float closestX = fmaxf(aMin.x, fminf(circle->position.x, aMax.x));
	float closestY = fmaxf(aMin.y, fminf(circle->position.y, aMax.y));
	Vector2 closestPoint = { closestX, closestY };

	Vector2 displacement = circle->position - closestPoint;
	float dist = Vector2Length(displacement);
	float overlap = circle->radius - dist;

	if (overlap > 0.0f) {
		Vector2 normal;
		if (dist < 0.0001f) {
			normal = { 0, -1 };
		}
		else {
			normal = displacement / dist;
		}

		Vector2 mtv = normal * overlap;

		if (aabb->isStatic && !circle->isStatic) {
			circle->position += mtv;
		}
		else if (!aabb->isStatic && circle->isStatic) {
			aabb->position -= mtv;
		}
		else if (!aabb->isStatic && !circle->isStatic) {
			circle->position += mtv * 0.5f;
			aabb->position -= mtv * 0.5f;
		}
		else {
			
		}

		Vector2 relVel = circle->velocity - aabb->velocity;
		float closingVel = Vector2DotProduct(relVel, normal);

		if (closingVel < 0.0f) {
			float e = circle->bounciness * aabb->bounciness;
			float totalMass = circle->mass + aabb->mass;
			float impulseMag = -(1.0f + e) * closingVel;
			if (aabb->isStatic && !circle->isStatic) {
				circle->velocity += normal * (impulseMag);
			}
			else if (!aabb->isStatic && circle->isStatic) {
				aabb->velocity -= normal * (impulseMag);
			}
			else if (!aabb->isStatic && !circle->isStatic) {
				Vector2 impulseOnCircle = normal * (impulseMag * (aabb->mass / totalMass));
				Vector2 impulseOnAABB = normal * (-impulseMag * (circle->mass / totalMass));
				circle->velocity += impulseOnCircle / circle->mass;
				aabb->velocity += impulseOnAABB / aabb->mass;
			}
		}
		return true;
	}
	return false;
}


bool AABBHalfspaceOverlap(FizziksAABB* aabb, FizziksHalfspace* halfspace) {
	Vector2 corners[4];
	corners[0] = aabb->position; // top-left
	corners[1] = { aabb->position.x + aabb->sizeXY.x, aabb->position.y }; // top-right
	corners[2] = { aabb->position.x, aabb->position.y + aabb->sizeXY.y }; // bottom-left
	corners[3] = { aabb->position.x + aabb->sizeXY.x, aabb->position.y + aabb->sizeXY.y }; // bottom-right

	Vector2 n = halfspace->getNormal();

	float minDot = FLT_MAX;
	for (int i = 0; i < 4; i++) {
		float d = Vector2DotProduct(corners[i] - halfspace->position, n);
		if (d < minDot) minDot = d;
	}

	if (minDot < 0.0f) {
		float overlap = -minDot;

		if (aabb->isStatic) {
			
		}
		else {
			aabb->position += n * overlap;
		}

		//float velAlongNormal = Vector2DotProduct(aabb->velocity, n);
		//if (velAlongNormal < 0.0f) { // traveling into the plane
		//	float e = aabb->bounciness * halfspace->bounciness;
		//	aabb->velocity += n * (-(1.0f + e) * velAlongNormal);
		//}

		if (!aabb->isStatic) {
			Vector2 Fgravity = world.accelerationGravity * aabb->mass;
			Vector2 FgPerp = n * Vector2DotProduct(Fgravity, n);
			Vector2 Fnormal = FgPerp * -1;
			aabb->netForce += Fnormal;
			DrawLineEx(aabb->position, aabb->position + Fnormal, 1, GREEN);
		}

		return true;
	}

	return false;
}

void cleanup() {

	for (int i = 0; i < world.objekts.size(); i++) {

		FizziksObjekt* objekt = world.objekts[i];

		if (	objekt->position.y > GetScreenHeight()
			||	objekt->position.y < 0
			||	objekt->position.x > GetScreenWidth()
			||	objekt->position.x < 0
			)
		{
			auto iterator = (world.objekts.begin() + i);
			FizziksObjekt* pointerToFizziksObjekt = *iterator;
			delete pointerToFizziksObjekt;

			world.objekts.erase(iterator);
			i--;
		}
	}
}

void update()
{
	dt = 1.0f / TARGET_FPS;
	time += dt;

	cleanup();
	world.update();
	

	if (IsKeyPressed(KEY_SPACE))
	{
		FizziksCircle* newBird = new FizziksCircle();
		newBird->position = { startX, startY };
		newBird->velocity = { speed * (float)cos(angle * DEG2RAD), -speed * (float)sin(angle * DEG2RAD) };
		newBird->bounciness = restitution;
		newBird->grippiness = coefficientOfFriction;

		world.add(newBird);
	}

	if (IsKeyPressed(KEY_S))
	{
		FizziksAABB* newBird = new FizziksAABB();
		newBird->position = { startX, startY };
		newBird->velocity = { speed * (float)cos(angle * DEG2RAD), -speed * (float)sin(angle * DEG2RAD) };

		world.add(newBird);
	}

	if (IsKeyPressed(KEY_R)) {
		for (int i = 0; i < world.objekts.size(); i++) {

			FizziksObjekt* objekt = world.objekts[i];

			if (objekt->Shape() != HALF_SPACE)
			{
				auto iterator = (world.objekts.begin() + i);
				FizziksObjekt* pointerToFizziksObjekt = *iterator;
				delete pointerToFizziksObjekt;

				world.objekts.erase(iterator);
				i--;
			}
		}
		MakeDeleteableObjekts();
	 }

	

}
void draw()
{
	BeginDrawing();
	ClearBackground(BLACK);
	DrawText("Mactavish Carney 101534351", 10, float(GetScreenHeight() - 25), 20, LIGHTGRAY);
	


	GuiSliderBar(Rectangle{ 100, 10, 800, 20 }, "", TextFormat("%.2f", time), &time, 0, 240);

	GuiSliderBar(Rectangle{ 100, 30, 800, 20 }, "Speed", TextFormat("Speed: %.0f", speed), &speed, -1000, 1000);

	GuiSliderBar(Rectangle{ 100, 50, 800, 20 }, "Angle", TextFormat("Angle: %.0f Degrees", angle), &angle, -180, 180);

	GuiSliderBar(Rectangle{ 100, 70, 400, 20 }, "StartPosX", TextFormat("StartPosX: %.0f", startX), &startX, 0, GetScreenWidth());

	GuiSliderBar(Rectangle{ 700, 70, 400, 20 }, "StartPosY", TextFormat("StartPosY: %.0f", startY), &startY, 0, GetScreenHeight());

	GuiSliderBar(Rectangle{ 100, 90, 800, 20 }, "Gravity Y", TextFormat("Gravity Y: %.0f Px/sec^2", world.accelerationGravity.y), &world.accelerationGravity.y, -1000, 1000);

	DrawText(TextFormat("T: %3.2f", time), GetScreenWidth() - 150, 5, 30, LIGHTGRAY);

	Vector2 startPos = { startX, startY };
	Vector2 velocity = { speed * cos(angle * DEG2RAD), -speed * sin(angle * DEG2RAD)};

	DrawLineEx(startPos, startPos + velocity, 3, RED);

	GuiSliderBar(Rectangle{ 100, 110, 400, 20 }, "halfspace X", TextFormat("X: %.0f", halfspace.position.x), &halfspace.position.x, 0, GetScreenWidth());
	GuiSliderBar(Rectangle{ 700, 110, 400, 20 }, "halfspace Y", TextFormat("Y: %.0f", halfspace.position.y), &halfspace.position.y, 0, GetScreenHeight());

	float halfspaceRotation = halfspace.getRotation();
	GuiSliderBar(Rectangle{ 100, 130, 800, 20 }, "rotation", TextFormat("rotation: %.0f", halfspace.getRotation()), &halfspaceRotation, -360, 360);
	halfspace.setRotationDegrees(halfspaceRotation);

	//control for friction
	GuiSliderBar(Rectangle{ 700, 150, 400, 20 }, "u", TextFormat("Y: %.2f", coefficientOfFriction), &coefficientOfFriction, 0, 1);

	//control for restitution
	GuiSliderBar(Rectangle{ 100, 150, 400, 20 }, "restitution", TextFormat("R: %.2f", restitution), &restitution, 0, 1);
	


	for (int i = 0; i < world.objekts.size(); i++) {
		world.objekts[i]->draw();
	}


	EndDrawing();

}
int main()
{

	InitWindow(InitialWidth, InitialHeight, "Mactavish Carney 101534351 GAME2005");
	SetTargetFPS(TARGET_FPS);
	halfspace.isStatic = true;
	halfspace.position = { 500, 700 };
	world.add(&halfspace);

	MakeDeleteableObjekts();

	while (!WindowShouldClose())
	{
		update();
		draw();
	}

	CloseWindow();
	return 0;
}
