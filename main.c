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

typedef struct triangle_t
{
    Vector2 a;
    Vector2 b;
    Vector2 c;
} triangle_t;

typedef struct vertex_data_t
{
    Vector2 pos;
    float distanceFromPlayer;
    float radian;
} vertex_data_t;

typedef struct edge_t
{
    vertex_data_t vertex1;
    vertex_data_t vertex2;
    int rectIndex;
} edge_t;

void DrawVisibleTriangles(triangle_t *triangles, int trianglesCount)
{
    for (int i = 0; i < trianglesCount; i++)
    {
        DrawTriangleLines(triangles[i].a, triangles[i].b, triangles[i].c, YELLOW);
    }
}

void DrawRays(Vector2 playerPos, edge_t *sides, int validSides)
{
    for (int i = 0; i < validSides; i++)
    {
        DrawText(TextFormat("corner %d, rad: %.2f", i, sides[i].vertex1.radian), sides[i].vertex1.pos.x, sides[i].vertex1.pos.y, 20, RED);
        DrawText(TextFormat("corner %d, rad: %.2f", i, sides[i].vertex2.radian), sides[i].vertex2.pos.x, sides[i].vertex2.pos.y, 20, RED);
        DrawLineV(playerPos, sides[i].vertex1.pos, WHITE);
        DrawLineV(playerPos, sides[i].vertex2.pos, WHITE);
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

Vector2 GetTriangleThirdPoint(edge_t edge, vertex_data_t intersectingVertex, Vector2 playerPos)
{
    // if edge is aligned with Y axis
    if (edge.vertex1.pos.x == edge.vertex2.pos.x)
    {

        double dXIntersect = fabs(intersectingVertex.pos.x - playerPos.x);
        double dYIntersect = fabs(intersectingVertex.pos.y - playerPos.y);

        //these make no sense in here, only the one where they are same value on both sides (x/y)
        double dXEdge = fabs(edge.vertex1.pos.x - playerPos.x);
        double dYEdge = fabs(edge.vertex1.pos.y - playerPos.y);

        double dXVertexes = fabs(dXEdge - dXIntersect);
        double dYVertexes = fabs(dYEdge - dYIntersect);

        return (Vector2){edge.vertex1.pos.x, playerPos.y + dYIntersect * ((dXIntersect / dXEdge))};
        // return (Vector2){edge.vertex1.pos.x, intersectingVertex.pos.y};
    }

    else
    {
        double dXIntersect = fabs(intersectingVertex.pos.x - playerPos.x);
        double dYIntersect = fabs(intersectingVertex.pos.y - playerPos.y);

        double dXEdge = fabs(edge.vertex1.pos.x - playerPos.x);
        double dYEdge = fabs(edge.vertex1.pos.y - playerPos.y);

        double dXVertexes = dXEdge - dXIntersect;
        double dYVertexes = dYEdge - dYIntersect;

        return (Vector2){playerPos.x + dXIntersect * ((dYIntersect / dYEdge)), edge.vertex1.pos.y};
        // return (Vector2){intersectingVertex.pos.x, edge.vertex1.pos.y};
    }
}

void AddHiddenRectangle(edge_t edge, Vector2 playerPos, triangle_t *hiddenTriangles, int* trianglesCount)
{
    Vector2 vertex1WallCollision;
    Vector2 vertex2WallCollision;
    float radian = edge.vertex1.radian;
    // touches top of screen
    if (radian >= 7 * PI / 4 || radian < PI / 4)
    {
        vertex1WallCollision.x = fabs(0 - playerPos.x) / cos(radian); 
        vertex1WallCollision.y = 0;
    }
    // touches right of screen
    else if (radian >= PI / 4 && radian < 3 * PI / 4)
    {
        vertex1WallCollision.x = screenWidth;
        vertex1WallCollision.y = (screenHeight - playerPos.y) / sin(radian);
    }
    // touches bottom of screen
    else if (radian >= 3 * PI / 4 && radian < 5 * PI / 4)
    {
        vertex1WallCollision.x = 
    }
    // touches left of screen
    else {

    }

}

void CreateHiddenTrianglesFromRays(edge_t *edges, triangle_t *visibleTriangles, Vector2 playerPos, int edgesCount, int *trianglesCount)
{
    float lastRadian = 0;
    for (int i = 0; i < edgesCount; i++)
    {
        if (edges[i].vertex2.radian < lastRadian)
        {
            continue;
        }

    }
}

void CreateTrianglesFromRays(edge_t *edges, triangle_t *visibleTriangles, Vector2 playerPos, int edgesCount, int *trianglesCount)
{
    // keeps track of the radian of the latest edge in order to avoid recalculating edges
    vertex_data_t lastVertex = {0};
    for (int i = 0; i < edgesCount; i++)
    {
        //lastVertex.radian = 0;
        // if the algorithm has gone past this edge
        if (lastVertex.radian > edges[i].vertex2.radian)
        {
            continue;
        }
        // this happens if the edge is above and one side is on the left side of the unit circle whilst one side is on the right side of the unit circle
        if (edges[i].vertex1.radian > edges[i].vertex2.radian)
        {
            for (int j = i + 1; j < edgesCount; j++)
            {
                // checks that corner is inside current edge parameters and is closer than corner2
                if ((edges[j].vertex1.radian > edges[i].vertex1.radian || edges[j].vertex1.radian < edges[i].vertex2.radian) &&
                    edges[j].vertex1.distanceFromPlayer < edges[i].vertex2.distanceFromPlayer)
                {
                    Vector2 thirdTrianglePoint = GetTriangleThirdPoint(edges[i], edges[j].vertex1, playerPos);
                    visibleTriangles[*trianglesCount] = (triangle_t){playerPos, edges[i].vertex1.pos, thirdTrianglePoint};
                    *trianglesCount += 1;
                    lastVertex = edges[j].vertex1;
                    break;
                }
            }
        }
        else
        {
            for (int j = i + 1; j < edgesCount; j++)
            {
                if (edges[j].vertex1.radian > edges[i].vertex1.radian && edges[j].vertex1.radian < edges[i].vertex2.radian &&
                    edges[j].vertex1.distanceFromPlayer < edges[i].vertex2.distanceFromPlayer)
                {
                    Vector2 thirdTrianglePoint = GetTriangleThirdPoint(edges[i], edges[j].vertex1, playerPos);
                    visibleTriangles[*trianglesCount] = (triangle_t){playerPos, edges[i].vertex1.pos, thirdTrianglePoint};
                    *trianglesCount += 1;
                    lastVertex = edges[j].vertex1;
                    break;
                }
            }
        }
        if (lastVertex.radian <= edges[i].vertex1.radian)
        {
            visibleTriangles[*trianglesCount] = (triangle_t){playerPos, edges[i].vertex1.pos, edges[i].vertex2.pos};
            *trianglesCount += 1;
            lastVertex = edges[i].vertex2;
        }
        else if (lastVertex.radian > edges[i].vertex1.radian && lastVertex.radian < edges[i].vertex2.radian)
        {
            // Vector2 thirdTrianglePoint = GetTriangleThirdPoint()
            lastVertex = edges[i].vertex2;
        }
    }
}

void SwapCornerData(edge_t *edge1Data, edge_t *edge2Data)
{
    edge_t edge1DataTemp = *edge1Data;
    *edge1Data = *edge2Data;
    *edge2Data = edge1DataTemp;
}

void SortCornersByDistance(edge_t *edges, int edgesCount)
{

    for (int i = 0; i < edgesCount - 1; i++)
    {
        float side1SmallestDistance = edges[i].vertex1.distanceFromPlayer < edges[i].vertex2.distanceFromPlayer ? edges[i].vertex1.distanceFromPlayer : edges[i].vertex2.distanceFromPlayer;
        int minIndex = i;
        for (int j = i + 1; j < edgesCount; j++)
        {
            float side2SmallestDistance = edges[j].vertex1.distanceFromPlayer < edges[j].vertex2.distanceFromPlayer ? edges[j].vertex1.distanceFromPlayer : edges[j].vertex2.distanceFromPlayer;
            if (side1SmallestDistance > side2SmallestDistance)
            {
                minIndex = j;
                side1SmallestDistance = side2SmallestDistance;
            }
        }
        SwapCornerData(&edges[i], &edges[minIndex]);
    }
}

void SortCornersByRadian(edge_t *edges, int edgesCount)
{

    for (int i = 0; i < edgesCount - 1; i++)
    {
        float side1SmallestRadian = edges[i].vertex1.radian < edges[i].vertex2.radian ? edges[i].vertex1.radian : edges[i].vertex2.radian;
        int minIndex = i;
        for (int j = i + 1; j < edgesCount; j++)
        {
            float side2SmallestRadian = edges[j].vertex1.radian < edges[j].vertex2.radian ? edges[j].vertex1.radian : edges[j].vertex2.radian;
            if (side1SmallestRadian > side2SmallestRadian)
            {
                minIndex = j;
                side1SmallestRadian = side2SmallestRadian;
            }
        }
        SwapCornerData(&edges[i], &edges[minIndex]);
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

void GetCornerData(vertex_data_t *corner, Vector2 playerPos, int rectIndex)
{

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
        corner->radian = PI + radian;
    }
    // top left corner
    else
    {
        corner->radian = (2 * PI) - radian;
    }
}

void GetCornerVectors(Vector2 playerPos, edge_t *sides, rect_t *rects, int rectsPlaced, int *validSides)
{

    for (int i = 0; i < rectsPlaced; i++)
    {
        int playerToRectPosX = GetPlayerRectOffset(playerPos.x, rects[i].startPos.x, rects[i].endPos.x);
        int playerToRectPosY = GetPlayerRectOffset(playerPos.y, rects[i].startPos.y, rects[i].endPos.y);

        switch (playerToRectPosY)
        {
        case 1:

            sides[*validSides].vertex1.pos = rects[i].endPos; // lower right corner
            GetCornerData(&sides[*validSides].vertex1, playerPos, i);

            sides[*validSides].vertex2.pos = (Vector2){rects[i].startPos.x, rects[i].endPos.y}; // lower left corner
            GetCornerData(&sides[*validSides].vertex2, playerPos, i);
            *validSides += 1;
            break;
        case -1:
            sides[*validSides].vertex1.pos = rects[i].startPos; // upper left corner
            GetCornerData(&sides[*validSides].vertex1, playerPos, i);

            sides[*validSides].vertex2.pos = (Vector2){rects[i].endPos.x, rects[i].startPos.y}; // upper right corner
            GetCornerData(&sides[*validSides].vertex2, playerPos, i);
            *validSides += 1;
            break;
        }

        switch (playerToRectPosX)
        {
        case 1:
            sides[*validSides].vertex1.pos = (Vector2){rects[i].endPos.x, rects[i].startPos.y}; // upper right corner
            GetCornerData(&sides[*validSides].vertex1, playerPos, i);
            sides[*validSides].vertex2.pos = rects[i].endPos; // lower right corner
            GetCornerData(&sides[*validSides].vertex2, playerPos, i);
            *validSides += 1;

            break;

        case -1:

            sides[*validSides].vertex1.pos = (Vector2){rects[i].startPos.x, rects[i].endPos.y}; // lower left corner
            GetCornerData(&sides[*validSides].vertex1, playerPos, i);

            sides[*validSides].vertex2.pos = rects[i].startPos; // upper left corner
            GetCornerData(&sides[*validSides].vertex2, playerPos, i);
            *validSides += 1;

            break;
        }
    }
    Vector2 upperRight = {screenWidth, 0};
    Vector2 lowerRight = {screenWidth, screenHeight};
    Vector2 lowerLeft = {0, screenHeight};
    Vector2 upperLeft = {0, 0};

    Vector2 mapCorners[] = {upperRight, lowerRight, lowerLeft, upperLeft};

    for (int i = 0; i < 4; i++)
    {
        sides[*validSides].vertex1.pos = mapCorners[i];
        GetCornerData(&sides[*validSides].vertex1, playerPos, rectsPlaced);
        sides[*validSides].vertex2.pos = mapCorners[(i + 1) % 4];
        GetCornerData(&sides[*validSides].vertex2, playerPos, rectsPlaced);
        *validSides += 1;
    }
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
    InitWindow(screenWidth * 1.2, screenHeight * 1.2, "2d visibility");
    SetTargetFPS(targetFPS);

    Vector2 playerPos = {screenWidth / 2, screenHeight / 2};
    rect_t rects[maxRects] = {0};
    int rectsPlaced = 0;
    edge_t edges[maxRects * 4 + 4] = {0}; // stores all sides on the map, including map border sides
    int edgesCount = 0;
    triangle_t visibleTriangles[maxRects * 4] = {0}; // unsure if this will be sufficient memory, if seg faults happen check this value
    int trianglesCount = 0;
    Vector2 mouseToRectStartDistance = {0, 0};
    Vector2 mouseToRectEndDistance = {0, 0};
    rect_t testRect = {(Vector2){200, 200},
                       (Vector2){500, 500}};
    PlaceRect(rects, testRect, &rectsPlaced);
    rect_t testRect2 = {(Vector2){1200, 1000},
                        (Vector2){1500, 1300}};
    PlaceRect(rects, testRect2, &rectsPlaced);

    while (!WindowShouldClose())
    {

        edgesCount = 0;
        trianglesCount = 0;
        ClearBackground(BLACK);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            MoveObjectOrPlayer(&playerPos, rects, rectsPlaced, &mouseToRectStartDistance, &mouseToRectEndDistance);
        }
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            RemoveRectMovement(rects, rectsPlaced);
        }
        GetCornerVectors(playerPos, edges, rects, rectsPlaced, &edgesCount);
        SortCornersByRadian(edges, edgesCount);
        CreateTrianglesFromRays(edges, visibleTriangles, playerPos, edgesCount, &trianglesCount);
        BeginDrawing();

        DrawCircleV(playerPos, playerSize, YELLOW);
        DrawRects(rects, rectsPlaced);
        // DrawRays(playerPos, edges, edgesCount);
        DrawVisibleTriangles(visibleTriangles, trianglesCount);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}