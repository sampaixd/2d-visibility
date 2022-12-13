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
    int rectIndex;
} corner_data_t;

void DrawRays(Vector2 playerPos, corner_data_t *corners, int validCorners)
{
    for (int i = 0; i < validCorners; i++)
    {
        DrawText(TextFormat("corner %d, rad: %.2f", i, corners[i].radian), corners[i].pos.x, corners[i].pos.y, 20, RED);
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

void CreateTrianglesFromRays(corner_data_t *corners, Vector2 playerPos, int rectsPlaced)
{
}

void SwapCornerData(corner_data_t *corner1Data, corner_data_t *corner2Data)
{
    corner_data_t corner1DataTemp = *corner1Data;
    *corner1Data = *corner2Data;
    *corner2Data = corner1DataTemp;
}

void SortCornersByDistanceFromPlayer(corner_data_t *corners, int rectsPlaced)
{
    for (int i = 0; i < (rectsPlaced * 4) + 4 - 1; i++)
    {
        int minIndex = i;
        for (int j = i + 1; j < (rectsPlaced * 4) + 4; j++)
        {
            if (corners[j].distanceFromPlayer < corners[minIndex].distanceFromPlayer)
            {
                minIndex = j;
            }
        }
        SwapCornerData(&corners[i], &corners[minIndex]);
    }
}

// if player is ahead of rect: 1, else if player is behind rect: -1 else: 0
int GetPlayerRectOffset(float playerPos, float rectStart, float rectEnd)
{
    if (playerPos > rectEnd)
    {
        return 1;
    }
    else if (playerPos < rectStart)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void GetCornerData(corner_data_t *corner, Vector2 playerPos, int rectIndex)
{
    corner->rectIndex = rectIndex;

    float dX = fabs(playerPos.x - corner->pos.x);
    float dY = fabs(playerPos.y - corner->pos.y);

    corner->distanceFromPlayer = dX * dX + dY * dY;

    float radian = atan(dX / dY);

    int playerToCornerOffsetX = GetPlayerRectOffset(playerPos.x, corner->pos.x, corner->pos.x);
    int playerToCornerOffsetY = GetPlayerRectOffset(playerPos.y, corner->pos.y, corner->pos.y);
    // checks where on the unit circle the corner is with the player as the centre
    //  0 is top
    //  top right corner
    if (playerToCornerOffsetX <= 0 && playerToCornerOffsetY >= 0)
    
    {
        corner->radian = radian;
    }
    // bottom right corner
    else if (playerToCornerOffsetX <= 0 && playerToCornerOffsetY <= 0)
    {
        corner->radian = PI - radian;
    }
    // bottom left corner
    else if (playerToCornerOffsetX >= 0 && playerToCornerOffsetY <= 0)
    {
        corner->radian = -(PI - radian);
    }
    // top left corner
    else
    {
        corner->radian = -radian;
    }
}

void GetCornerVectors(Vector2 playerPos, corner_data_t *corners, rect_t *rects, int rectsPlaced, int *validCorners)
{

    for (int i = 0; i < rectsPlaced; i++)
    {
        int playerToRectPosX = GetPlayerRectOffset(playerPos.x, rects[i].startPos.x, rects[i].endPos.x);
        int playerToRectPosY = GetPlayerRectOffset(playerPos.y, rects[i].startPos.y, rects[i].endPos.y);

        switch (playerToRectPosY)
        {
        case 1:
            corners[*validCorners].pos = (Vector2){rects[i].startPos.x, rects[i].endPos.y}; // lower left corner
            GetCornerData(&corners[*validCorners], playerPos, i);
            *validCorners += 1;
            corners[*validCorners].pos = rects[i].endPos; // lower right corner
            GetCornerData(&corners[*validCorners], playerPos, i);
            *validCorners += 1;
            break;
        case -1:
            corners[*validCorners].pos = rects[i].startPos; // upper left corner
            GetCornerData(&corners[*validCorners], playerPos, i);
            *validCorners += 1;
            corners[*validCorners].pos = (Vector2){rects[i].endPos.x, rects[i].startPos.y}; // upper right corner
            GetCornerData(&corners[*validCorners], playerPos, i);
            *validCorners += 1;
            break;
        }

        switch (playerToRectPosX)
        {
        case 1:
            // if two sides are visible, this will stop their common corner to be selected twice
            if (playerToRectPosY == 1)
            {
                corners[*validCorners].pos = (Vector2){rects[i].endPos.x, rects[i].startPos.y}; // upper right corner
                GetCornerData(&corners[*validCorners], playerPos, i);
                *validCorners += 1;
            }
            else if (playerToRectPosY == -1)
            {
                corners[*validCorners].pos = rects[i].endPos; // lower right corner
                GetCornerData(&corners[*validCorners], playerPos, i);
                *validCorners += 1;
            }
            else
            {
                corners[*validCorners].pos = (Vector2){rects[i].endPos.x, rects[i].startPos.y}; // upper right corner
                GetCornerData(&corners[*validCorners], playerPos, i);
                *validCorners += 1;
                corners[*validCorners].pos = rects[i].endPos; // lower right corner
                GetCornerData(&corners[*validCorners], playerPos, i);
                *validCorners += 1;
            }
            break;

        case -1:
            if (playerToRectPosY == 1)
            {
                corners[*validCorners].pos = rects[i].startPos; // upper left corner
                GetCornerData(&corners[*validCorners], playerPos, i);
                *validCorners += 1;
            }
            else if (playerToRectPosY == -1)
            {
                corners[*validCorners].pos = (Vector2){rects[i].startPos.x, rects[i].endPos.y}; // lower left corner
                GetCornerData(&corners[*validCorners], playerPos, i);
                *validCorners += 1;
            }
            else
            {
                corners[*validCorners].pos = (Vector2){rects[i].startPos.x, rects[i].endPos.y}; // lower left corner
                GetCornerData(&corners[*validCorners], playerPos, i);
                *validCorners += 1;
                corners[*validCorners].pos = rects[i].startPos; // upper left corner
                GetCornerData(&corners[*validCorners], playerPos, i);
                *validCorners += 1;
            }
            break;
        }
    }
    corners[*validCorners].pos = (Vector2){0, 0}; // upper left corner
    GetCornerData(&corners[*validCorners], playerPos, rectsPlaced);
    *validCorners += 1;
    corners[*validCorners].pos = (Vector2){screenWidth, 0}; // upper right corner
    GetCornerData(&corners[*validCorners], playerPos, rectsPlaced);
    *validCorners += 1;
    corners[*validCorners].pos = (Vector2){0, screenHeight}; // lower left corner
    GetCornerData(&corners[*validCorners], playerPos, rectsPlaced);
    *validCorners += 1;
    corners[*validCorners].pos = (Vector2){screenWidth, screenHeight}; // lower right corner
    GetCornerData(&corners[*validCorners], playerPos, rectsPlaced);
    *validCorners += 1;
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
    int rectsPlaced = 0;
    corner_data_t corners[maxRects * 4 + 4] = {0}; // stores all corners on the map, including map borders
    int validCorners = 0;
    Vector2 mouseToRectStartDistance = {0, 0};
    Vector2 mouseToRectEndDistance = {0, 0};
    rect_t testRect = {(Vector2){200, 200},
                       (Vector2){500, 500}};
    PlaceRect(rects, testRect, &rectsPlaced);
    rect_t testRect2 = {(Vector2){1000, 700},
                        (Vector2){1200, 800}};
    PlaceRect(rects, testRect2, &rectsPlaced);

    while (!WindowShouldClose())
    {
        validCorners = 0;
        ClearBackground(BLACK);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            MoveObjectOrPlayer(&playerPos, rects, rectsPlaced, &mouseToRectStartDistance, &mouseToRectEndDistance);
        }
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            RemoveRectMovement(rects, rectsPlaced);
        }
        GetCornerVectors(playerPos, corners, rects, rectsPlaced, &validCorners);
        SortCornersByDistanceFromPlayer(corners, rectsPlaced);
        BeginDrawing();

        DrawCircleV(playerPos, playerSize, YELLOW);
        DrawRects(rects, rectsPlaced);
        DrawRays(playerPos, corners, validCorners);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}