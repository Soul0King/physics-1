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

class FizziksObjekt {

public:

	Vector2 position = { 0, 0 };
	Vector2 velocity = { 0,0 };
	float mass = 1; // in kg

	std::string name = "objekt";
	Color color = GREEN;

	virtual void draw() {
		DrawCircle(position.x, position.y, 2, color);
	}
};

class FizziksBox : public FizziksObjekt {
public:
	Vector2 size; // x = width, y = hight
};

class FizziksCircle : public FizziksObjekt {
public:
	float radius = 15; // circle radius in pixels

	void draw() override {
		DrawCircle(position.x, position.y, radius, color);
		DrawLineEx(position, position + velocity, 1, color);
	}
};

bool CircleCircleOverlap(FizziksCircle* circleA, FizziksCircle* circleB) {
	Vector2 displacementFromAToB = circleB->position - circleA->position;
	float diatance = Vector2Length(displacementFromAToB);
	float sumOfRadii = circleA->radius + circleB->radius;

	if (sumOfRadii > diatance) {
		return true;
	}
	else
		return false;
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

			FizziksObjekt* object = objekts[i];

			//vel = change in position / time, therefore     change in position = vel * time 
			object->position = object->position + object->velocity * dt;

			//accel = deltaV / time (change in velocity over time) therefore     deltaV = accel * time
			object->velocity = object->velocity + accelerationGravity * dt;
		}

		checkCollisions();
	}

	void checkCollisions() {
		std::vector<bool> isColliding(objekts.size(), false);

		for (int i = 0; i < objekts.size(); i++) {
			for (int j = i + 1; j < objekts.size(); j++) {

				FizziksCircle* circleA = (FizziksCircle*)objekts[i];
				FizziksCircle* circleB = (FizziksCircle*)objekts[j];


				if (CircleCircleOverlap(circleA, circleB)) {
					isColliding[i] = true;
					isColliding[j] = true;
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

void cleanup() {

	for (int i = 0; i < world.objekts.size(); i++) {

		FizziksObjekt* objekt = world.objekts[i];

		if (	objekt->position.y > GetScreenHeight()
			||	objekt->position.y < 0
			||	objekt->position.x > GetScreenHeight()
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

	GuiSliderBar(Rectangle{ 10, 70, 800, 20 }, "StartPosX", TextFormat("StartPosX: %.0f", startX), &startX, 100, 900);

	GuiSliderBar(Rectangle{ 10, 90, 800, 20 }, "StartPosY", TextFormat("StartPosY: %.0f", startY), &startY, 200, 500);

	GuiSliderBar(Rectangle{ 10, 120, 500, 30 }, "Gravity Y", TextFormat("Gravity Y: %.0f Px/sec^2", world.accelerationGravity.y), &world.accelerationGravity.y, -1000, 1000);

	DrawText(TextFormat("T: %3.2f", time), GetScreenWidth() - 150, 5, 30, LIGHTGRAY);

	Vector2 startPos = { startX, startY };
	Vector2 velocity = { speed * cos(angle * DEG2RAD), -speed * sin(angle * DEG2RAD)};

	DrawLineEx(startPos, startPos + velocity, 3, RED);

	for (int i = 0; i < world.objekts.size(); i++) {
		world.objekts[i]->draw();
	}

	EndDrawing();

}
int main()
{
	InitWindow(InitialWidth, InitialHeight, "Mactavish Carney 101534351 GAME2005");
	SetTargetFPS(TARGET_FPS);

	while (!WindowShouldClose())
	{
		update();
		draw();
	}

	CloseWindow();
	return 0;
}
