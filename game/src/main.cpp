/*
This project uses the Raylib framework to provide us functionality for math, graphics, GUI, input etc.
See documentation here: https://www.raylib.com/, and examples here: https://www.raylib.com/examples.html
*/

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "game.h"

const unsigned int TARGET_FPS = 50;
float dt = 1.0f / TARGET_FPS;
float time = 0;

float speed = 100;
float angle = 0;
float startX = 100;
float startY = GetScreenHeight() - 100;

Vector2 position = { 500, 500 };
Vector2 velocity = { 0,0 };
Vector2 accelerationGravity = { 0, 9 };

void update()
{
	dt = 1.0f / TARGET_FPS;
	time += dt;



	//vel = change in position / time, therefore     change in position = vel * time 
	position = position + velocity * dt;

	//accel = deltaV / time (change in velocity over time) therefore     deltaV = accel * time
	velocity = velocity + accelerationGravity * dt;

	if (IsKeyPressed(KEY_SPACE))
	{
		position = { 100, (float)GetScreenHeight() - 100 };
		velocity = { speed * (float)cos(angle * DEG2RAD), -speed * (float)sin(angle * DEG2RAD) };
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

	GuiSliderBar(Rectangle{ 10, 120, 500, 30 }, "Gravity Y", TextFormat("Gravity Y: %.0f Px/sec^2", accelerationGravity.y), &accelerationGravity.y, -1000, 1000);

	DrawText(TextFormat("T: %3.2f", time), GetScreenWidth() - 150, 5, 30, LIGHTGRAY);

	Vector2 startPos = { startX, startY };
	Vector2 velocity = { speed * cos(angle * DEG2RAD), -speed * sin(angle * DEG2RAD)};

	DrawLineEx(startPos, startPos + velocity, 3, RED);

	DrawCircle(position.x, position.y, 15, RED);

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
