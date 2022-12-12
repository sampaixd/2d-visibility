#include "raylib.h"
#include <math.h>

const int screenWidth = 1920;
const int screenHeight = 1080;
const int targetFPS = 60;

const float playerSize = 10;

const int maxRects = 10;

typedef struct rect_t
{
    Vector2 startPos;
    Vector2 endPos;
    bool isMoving;
} rect_t;

typedef struct corner_data_t
{
    Vector2 pos;
    float distanceFromPlayer;
    float radian;
} corner_data_t;

void DrawRays(Vector2 playerPos, corner_data_t *corners, int rectsPlaced)
{
    for (int i = 0; i < (rectsPlaced * 4) + 4; i++)
    {
        DrawText(TextFormat("corner %d", i), corners[i].pos.x, corners[i].pos.y, 20, RED);
        DrawLineV(playerPos, corners[i].pos, WHITE);
    }
}

Vector2 GetRectSize(rect_t rect)
{
    return (Vector2){rect.endPos.x - rect.startPos.x, rect.endPos.y - rect.startPos.y};
}

void DrawRects(rect_t *rects, int rectsPlaced)
{
    for (int i = 0; i < rectsPlaced; i++)
    {
        Vector2 size = GetRectSize(rects[i]);
        DrawRectangleV(rects[i].startPos, size, GREEN);
    }
}

void CollectCornerData(corner_data_t *corners, Vector2 playerPos, int rectsPlaced)
{
    for (int i = 0; i < (rectsPlaced * 4) + 4; i++)
    {
        float dX = playerPos.x - corners[i].pos.x;
        float dY = playerPos.y - corners[i].pos.y;
        // pythagoras theorem
        corners[i].distanceFromPlayer = dX * dX + dY * dY;
        // gets the radian by doing tan-1()
        corners[i].radian = atan(dX / dY);
    }
}
void GetCornerVectors(corner_data_t *corners, rect_t *rects, int rectsPlaced)
{
    for (int i = 0; i < rectsPlaced; i++)
    {

        corners[(i * 4)].pos = rects[i].startPos;                                     // upper left corner
        corners[(i * 4) + 1].pos = (Vector2){rects[i].endPos.x, rects[i].startPos.y}; // upper right corner
        corners[(i * 4) + 2].pos = (Vector2){rects[i].startPos.x, rects[i].endPos.y}; // lower left corner
        corners[(i * 4) + 3].pos = rects[i].endPos;                                   // lower right corner
    }
    corners[(rectsPlaced * 4)].pos = (Vector2){0, 0};                          // upper left corner
    corners[(rectsPlaced * 4) + 1].pos = (Vector2){screenWidth, 0};            // upper right corner
    corners[(rectsPlaced * 4) + 2].pos = (Vector2){0, screenHeight};           // lower left corner
    corners[(rectsPlaced * 4) + 3].pos = (Vector2){screenWidth, screenHeight}; // lower right corner
    return;
}

bool MouseIsInsideObject(Vector2 mousePos, rect_t rect)
{
    return (mousePos.x > rect.startPos.x && mousePos.x < rect.endPos.x) &&
           (mousePos.y > rect.startPos.y && mousePos.y < rect.endPos.y);
}

void RemoveRectMovement(rect_t *rects, int rectsPlaced)
{
    for (int i = 0; i < rectsPlaced; i++)
    {
        rects[i].isMoving = false;
    }
}

void MoveObjectOrPlayer(Vector2 *playerPos, rect_t *rects, int rectsPlaced, Vector2 *mouseToRectStartDistance, Vector2 *mouseToRectEndDistance)
{
    Vector2 mousePos = GetMousePosition();
    // if a object is already selected
    for (int i = 0; i < rectsPlaced; i++)
    {
        if (rects[i].isMoving)
        {
            rects[i].startPos = (Vector2){mousePos.x - mouseToRectStartDistance->x, mousePos.y - mouseToRectStartDistance->y};
            rects[i].endPos = (Vector2){mousePos.x + mouseToRectEndDistance->x, mousePos.y + mouseToRectEndDistance->y};
            return;
        }
    }
    // if you selected an object
    for (int i = 0; i < rectsPlaced; i++)
    {
        if (MouseIsInsideObject(mousePos, rects[i]))
        {
            *mouseToRectStartDistance = GetRectSize((rect_t){rects[i].startPos, mousePos});
            *mouseToRectEndDistance = GetRectSize((rect_t){mousePos, rects[i].endPos});
            rects[i].isMoving = true;

            return;
        }
    }
    // if no object is found, the player will be set to mouse position
    *playerPos = mousePos;
}

void PlaceRect(rect_t *rects, rect_t placedRect, int *rectsPlaced)
{
    rects[*rectsPlaced] = placedRect;
    *rectsPlaced += 1;
}

int main()
{
    InitWindow(screenWidth, screenHeight, "2d visibility");
    SetTargetFPS(targetFPS);

    Vector2 playerPos = {screenWidth / 2, screenHeight / 2};
    rect_t rects[maxRects] = {0};
    corner_data_t corners[maxRects * 4 + 4] = {0}; // stores all corners on the map, including map borders
    int rectsPlaced = 0;
    Vector2 mouseToRectStartDistance = {0, 0};
    Vector2 mouseToRectEndDistance = {0, 0};
    rect_t testRect = {(Vector2){200, 200},
                       (Vector2){500, 500}};
    PlaceRect(rects, testRect, &rectsPlaced);

    while (!WindowShouldClose())
    {
        ClearBackground(BLACK);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            MoveObjectOrPlayer(&playerPos, rects, rectsPlaced, &mouseToRectStartDistance, &mouseToRectEndDistance);
        }
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            RemoveRectMovement(rects, rectsPlaced);
        }
        GetCornerVectors(corners, rects, rectsPlaced);
        BeginDrawing();

        DrawCircleV(playerPos, playerSize, YELLOW);
        DrawRects(rects, rectsPlaced);
        DrawRays(playerPos, corners, rectsPlaced);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}