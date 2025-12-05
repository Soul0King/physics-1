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

const unsigned int TARGET_FPS = 50;
float dt = 1.0f / TARGET_FPS;
float time = 0;

float restitution = 0.9f;

//// LE7 slider variables
//float rCircleMass = 1;
//float rCircleSpeed = 1;
//Vector2 rCircleNormal = { 0, 1 };
//
//float gCircleMass = 1;
//float gCircleSpeed = 1;
//Vector2 gCircleNormal = { 0, 1 };
//
//float bCircleMass = 1;
//float bCircleSpeed = 100;
//Vector2 bCircleNormal = { 1, 0 };
//
//float pCircleMass = 1;
//float pCircleSpeed = 1;
//Vector2 pCircleNormal = { 0, 1 };
//
//float yCircleMass = 1;
//float yCircleSpeed = 1;


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

	float coefficientOfFriction = 0.5f;

	void draw() override {
		DrawCircle(position.x, position.y, radius, color);
		DrawLineEx(position, position + velocity, 1, color);
	}

	// Inherited via FizziksObjekt
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

	// Inherited via FizziksObjekt
	FizziksShape Shape() override
	{
		return HALF_SPACE;
	}
};

class FizziksAABB : public FizziksObjekt {
public:
	Vector2 sizeXY = { 0,0 };

	void draw() override {
		DrawRectangle(position.x, position.y, sizeXY.x, sizeXY.y, color);
		DrawLineEx(position, position + velocity, 1, color);
	}

	// Inherited via FizziksObjekt
	FizziksShape Shape() override
	{
		return AABB;
	}
};

bool CircleCircleOverlap(FizziksCircle* circleA, FizziksCircle* circleB);
bool CircleHalfspaceOverlap(FizziksCircle* circle, FizziksHalfspace* halfspace);


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

			//vel = change in position / time, therefore     change in position = vel * time 
			objekt->position = objekt->position + objekt->velocity * dt;

			Vector2 acceleration = objekt->netForce/objekt->mass; //a = F/m

			//accel = deltaV / time (change in velocity over time) therefore     deltaV = accel * time
			objekt->velocity = objekt->velocity + acceleration * dt;

		}
	}

	// update state of all phiysics objects
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
	
	/*FizziksCircle* rCircle = new FizziksCircle();
	FizziksCircle* gCircle = new FizziksCircle();
	FizziksCircle* bCircle = new FizziksCircle();
	FizziksCircle* yCircle = new FizziksCircle();
	FizziksCircle* pCircle = new FizziksCircle();

	rCircle->mass = rCircleMass;
	rCircle->bounciness = restitution;
	rCircle->radius = 20;
	rCircle->coefficientOfFriction = 0.1;
	rCircle->color = RED;
	rCircle->baseColor = RED;
	rCircle->position = { 100, 500 };
	rCircle->velocity = rCircleNormal * rCircleSpeed;
	world.add(rCircle);

	pCircle->mass = pCircleMass;
	pCircle->bounciness = restitution;
	pCircle->radius = 20;
	pCircle->coefficientOfFriction = 0.1;
	pCircle->color = PURPLE;
	pCircle->baseColor = PURPLE;
	pCircle->position = { 100, 480 };
	pCircle->velocity = pCircleNormal * pCircleSpeed;
	world.add(pCircle);

	gCircle->mass = gCircleMass;
	gCircle->bounciness = restitution;
	gCircle->coefficientOfFriction = 0.8;
	gCircle->color = GREEN;
	gCircle->baseColor = GREEN;
	gCircle->position = { 200, 500 };
	gCircle->velocity = gCircleNormal * gCircleSpeed;
	world.add(gCircle);

	bCircle->mass = bCircleMass;
	bCircle->bounciness = restitution;
	bCircle->radius = 20;
	bCircle->coefficientOfFriction = 0.1;
	bCircle->color = BLUE;
	bCircle->baseColor = BLUE;
	bCircle->position = { 600, 680 };
	bCircle->velocity = bCircleNormal * bCircleSpeed;
	world.add(bCircle);

	yCircle->mass = yCircleMass;
	yCircle->bounciness = restitution;
	yCircle->radius = 20;
	yCircle->coefficientOfFriction = 0.8;
	yCircle->color = YELLOW;
	yCircle->baseColor = YELLOW;
	yCircle->position = { 800, 680 };
	yCircle->velocity = { 0, 0 };
	world.add(yCircle);*/
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

		Vector2 mtv = normalAToB * overlap; // minimum translation vector

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
		Vector2 mtv = normalAToB * overlap; // minimum translation vector

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
		float u = circle->coefficientOfFriction;
		float frictionMagnitude = u * Vector2Length(Fnormal);

		Vector2 FgPara = Fgravity - FgPerp;
		Vector2 frictionDirection = Vector2Normalize(FgPara) * -1;

		frictionMagnitude = Clamp(frictionMagnitude, 0.0f, Vector2Length(FgPara)); // frictionMagnitude cant be more than FgPara

		Vector2 Ffriction = frictionDirection * frictionMagnitude;

		circle->netForce += Ffriction;
		DrawLineEx(circle->position, circle->position + Ffriction, 2, ORANGE);


		//Bouncing!
		//from perspective of A
		//Vector2 velocityBRelativeToA = circleB->velocity - circleA->velocity;
		float closingVelocity1D = Vector2DotProduct(circle->velocity, halfspace->getNormal());
		//if dot is negative then we are coliding. if positive, not colliding
		if (closingVelocity1D >= -2) return true;

		float restitution = circle->bounciness * halfspace->bounciness;
		//velFinal = velInitial + -(1 + resitiution * velInitial)
		circle->velocity += halfspace->getNormal() * closingVelocity1D * -(1.0f + restitution);
		


		return true;
	}
	else return false;
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
	//GuiSliderBar(Rectangle{ 700, 150, 400, 20 }, "u", TextFormat("Y: %.2f", coefficientOfFriction), &coefficientOfFriction, 0, 1);

	//control for restitution
	GuiSliderBar(Rectangle{ 100, 150, 400, 20 }, "restitution", TextFormat("R: %.2f", restitution), &restitution, 0, 1);

	//// LE7 controls
	//GuiSliderBar(Rectangle{ 100, 170, 400, 20 }, "Red Circle Mass", TextFormat("M: %.0f", rCircleMass), &rCircleMass, 1, 50);
	//GuiSliderBar(Rectangle{ 700, 170, 400, 20 }, "Red Circle Speed", TextFormat("M: %.0f", rCircleSpeed), &rCircleSpeed, 0, 1000);

	//GuiSliderBar(Rectangle{ 100, 190, 400, 20 }, "green Circle Mass", TextFormat("M: %.0f", gCircleMass), &gCircleMass, 1, 50);
	//GuiSliderBar(Rectangle{ 700, 190, 400, 20 }, "green Circle Speed", TextFormat("M: %.0f", gCircleSpeed), &gCircleSpeed, 0, 1000);

	//GuiSliderBar(Rectangle{ 100, 210, 400, 20 }, "blue Circle Mass", TextFormat("M: %.0f", bCircleMass), &bCircleMass, 1, 50);
	//GuiSliderBar(Rectangle{ 700, 210, 400, 20 }, "blue Circle Speed", TextFormat("M: %.0f", bCircleSpeed), &bCircleSpeed, 0, 1000);

	//GuiSliderBar(Rectangle{ 100, 230, 400, 20 }, "purple Circle Mass", TextFormat("M: %.0f", pCircleMass), &pCircleMass, 1, 50);
	//GuiSliderBar(Rectangle{ 700, 230, 400, 20 }, "purple Circle Speed", TextFormat("M: %.0f", pCircleSpeed), &pCircleSpeed, 0, 1000);

	//GuiSliderBar(Rectangle{ 100, 250, 400, 20 }, "yellow Circle Mass", TextFormat("M: %.0f", yCircleMass), &yCircleMass, 1, 50);
	


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
