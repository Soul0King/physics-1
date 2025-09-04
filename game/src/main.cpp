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
float x = 500;
float y = 500;
float frequency = 1;
float amplitude = 100;

void update()
{
	dt = 1.0f / TARGET_FPS;
	time += dt;

	x = x + (-sin(time * frequency)) * frequency * amplitude * dt;
	y = y + (cos(time * frequency)) * frequency * amplitude * dt;

}
void draw()
{
	BeginDrawing();
	ClearBackground(BLACK);
	DrawText("Mactavish Carney 101534351", 10, float(GetScreenHeight() - 25), 20, LIGHTGRAY);



	GuiSliderBar(Rectangle{ 10, 10, 800, 20 }, "", TextFormat("%.2f", time), &time, 0, 240);
	GuiSliderBar(Rectangle{ 10, 30, 800, 20 }, "", TextFormat("%.2f", frequency), &frequency, 0, 240);
	GuiSliderBar(Rectangle{ 10, 50, 800, 20 }, "", TextFormat("%.2f", amplitude), &amplitude, 0, 240);
	DrawText(TextFormat("T: %3.2f", time), GetScreenWidth() - 150, 5, 30, LIGHTGRAY);
	DrawText(TextFormat("f: %3.2f", frequency), GetScreenWidth() - 150, 25, 30, LIGHTGRAY);
	DrawText(TextFormat("a: %3.2f", amplitude), GetScreenWidth() - 150, 45, 30, LIGHTGRAY);

	DrawCircle(x, y, 70, RED);

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
