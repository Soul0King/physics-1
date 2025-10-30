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

enum FizziksShape {
	CIRCLE,
	HALF_SPACE
};

class FizziksObjekt {

public:
	bool isStatic = false;
	Vector2 position = { 0, 0 };
	Vector2 velocity = { 0,0 };
	float mass = 1; // in kg

	std::string name = "objekt";
	Color color = GREEN;

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

		Vector2 normalAToB = (displacementFromAToB / distance);
		Vector2 mtv = normalAToB * overlap; // minimum translation vector

		circleA->position -= mtv * 0.5f;
		circleB->position += mtv * 0.5f;
		return true;
	}
	else
		return false;
}

bool CircleHalfspaceOverlap(FizziksCircle* circle, FizziksHalfspace* halfspace) {

	Vector2 displacementToCircle = circle->position - halfspace->position;

	float dot = Vector2DotProduct(displacementToCircle, halfspace->getNormal());
	Vector2 vectorProjection = halfspace->getNormal() * dot;


	DrawLineEx(circle->position, circle->position - vectorProjection, 1, GRAY);

	Vector2 midpoint = circle->position - vectorProjection * 0.5f;
	DrawText(TextFormat("D: %6.0f", dot), midpoint.x, midpoint.y, 30, GRAY);

	float overlap = circle->radius - dot;
	

	if (overlap > 0) {
		Vector2 normalAToB = (vectorProjection / dot);
		Vector2 mtv = normalAToB * overlap; // minimum translation vector

		circle->position += mtv;
		return true;
	}
	else return false;
}

class FizziksWorld {
public: 
	std::vector<FizziksObjekt*> objekts;

	Vector2 accelerationGravity = { 0, 9 };

	void add(FizziksObjekt* newObject) {
		objekts.push_back(newObject);
	}

	// update state of all phiysics objects
	void update() {
		

		for (int i = 0; i < objekts.size(); i++) {

			FizziksObjekt* objekt = objekts[i];

			if (objekt->isStatic) continue;

			//vel = change in position / time, therefore     change in position = vel * time 
			objekt->position = objekt->position + objekt->velocity * dt;

			//accel = deltaV / time (change in velocity over time) therefore     deltaV = accel * time
			objekt->velocity = objekt->velocity + accelerationGravity * dt;
		}

		checkCollisions();
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
			objekts[i]->color = isColliding[i] ? RED : GREEN;
		}
	}
};

float speed = 100;
float angle = 0;
float startX = 100;
float startY = 500;


FizziksWorld world;
FizziksHalfspace halfspace;
FizziksHalfspace halfspace2;


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

		world.add(newBird);
	}

	

}
void draw()
{
	BeginDrawing();
	ClearBackground(BLACK);
	DrawText("Mactavish Carney 101534351", 10, float(GetScreenHeight() - 25), 20, LIGHTGRAY);



	GuiSliderBar(Rectangle{ 10, 10, 800, 20 }, "", TextFormat("%.2f", time), &time, 0, 240);

	GuiSliderBar(Rectangle{ 10, 30, 800, 20 }, "Speed", TextFormat("Speed: %.0f", speed), &speed, -1000, 1000);

	GuiSliderBar(Rectangle{ 10, 50, 800, 20 }, "Angle", TextFormat("Angle: %.0f Degrees", angle), &angle, -180, 180);

	GuiSliderBar(Rectangle{ 10, 70, 400, 20 }, "StartPosX", TextFormat("StartPosX: %.0f", startX), &startX, 0, GetScreenWidth());

	GuiSliderBar(Rectangle{ 600, 70, 400, 20 }, "StartPosY", TextFormat("StartPosY: %.0f", startY), &startY, 0, GetScreenHeight());

	GuiSliderBar(Rectangle{ 10, 90, 800, 20 }, "Gravity Y", TextFormat("Gravity Y: %.0f Px/sec^2", world.accelerationGravity.y), &world.accelerationGravity.y, -1000, 1000);

	DrawText(TextFormat("T: %3.2f", time), GetScreenWidth() - 150, 5, 30, LIGHTGRAY);

	Vector2 startPos = { startX, startY };
	Vector2 velocity = { speed * cos(angle * DEG2RAD), -speed * sin(angle * DEG2RAD)};

	DrawLineEx(startPos, startPos + velocity, 3, RED);

	GuiSliderBar(Rectangle{ 10, 110, 400, 20 }, "halfspace X", TextFormat("X: %.0f", halfspace.position.x), &halfspace.position.x, 0, GetScreenWidth());
	GuiSliderBar(Rectangle{ 500, 110, 400, 20 }, "Y", TextFormat("Y: %.0f", halfspace.position.y), &halfspace.position.y, 0, GetScreenHeight());

	float halfspaceRotation = halfspace.getRotation();
	GuiSliderBar(Rectangle{ 10, 130, 800, 20 }, "rotation", TextFormat("rotation: %.0f", halfspace.getRotation()), &halfspaceRotation, -360, 360);
	halfspace.setRotationDegrees(halfspaceRotation);

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
	halfspace2.isStatic = true;
	halfspace2.position = { 200, 400 };
	halfspace2.setRotationDegrees(10);
	world.add(&halfspace2);

	while (!WindowShouldClose())
	{
		update();
		draw();
	}

	CloseWindow();
	return 0;
}
