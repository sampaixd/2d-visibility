#include "raylib.h"
#include <math.h>

// #define DEV_MODE  // comment out to disable dev mode

const int screenWidth = 1280;
const int screenHeight = 720;
const int targetFPS = 60;

const float playerSize = 10;

const int maxRects = 10;
// used for testing
Color colors[] = {
    RED,
    ORANGE,
    YELLOW,
    GREEN,
    BLUE,
    VIOLET,
    WHITE,
    GRAY,
    MAROON,
    PURPLE,
    LIME,
    LIGHTGRAY,
    DARKGRAY,
};

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
    Color rectIndex;
} triangle_t;

typedef struct vertex_data_t
{
    Vector2 pos;
    float distanceFromPlayer;
    double radian;
} vertex_data_t;

typedef struct edge_t
{
    vertex_data_t vertex1;
    vertex_data_t vertex2;
    int rectIndex;
} edge_t;

void DrawMapCorners()
{
    DrawLine(0, 0, screenWidth, 0, WHITE);
    DrawLine(screenWidth, 0, screenWidth, screenHeight, WHITE);
    DrawLine(screenWidth, screenHeight, 0, screenHeight, WHITE);
    DrawLine(0, screenHeight, 0, 0, WHITE);
}

void DrawVisibleTriangles(triangle_t *triangles, int trianglesCount)
{
    for (int i = 0; i < trianglesCount; i++)
    {
#ifdef DEV_MODE
        DrawTriangle(triangles[i].a, triangles[i].b, triangles[i].c, triangles[i].rectIndex);
        DrawTriangleLines(triangles[i].a, triangles[i].b, triangles[i].c, triangles[i].rectIndex);
        DrawText(TextFormat("rectangle asociated: %d", triangles[i].rectIndex), triangles[i].b.x, triangles[i].b.y, 20, triangles[i].rectIndex);
        DrawText(TextFormat("triangles present: %d", trianglesCount), screenWidth / 2, screenHeight / 2, 20, WHITE);
#else
        DrawTriangle(triangles[i].a, triangles[i].b, triangles[i].c, BLACK);
#endif
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

        // these make no sense in here, only the one where they are same value on both sides (x/y)
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

Vector2 GetCornerBetweenWallIntersects(Vector2 vertex1WallCollision, Vector2 vertex2WallCollision)
{
    Vector2 corner;
    // looking at GetWallCollision, if x is assigned either 0 or screenWidth, it would mean that
    // its edge is either top or bot. This would also mean the neighor edge would be the
    // y value of the other vertex collision. This might be very unclear, If i dont rewrite
    // this, refer to the function below
    if (vertex1WallCollision.x == screenWidth || vertex1WallCollision.x == 0)
    {
        corner.x = vertex1WallCollision.x;
        corner.y = vertex2WallCollision.y;
    }
    else
    {
        corner.x = vertex2WallCollision.x;
        corner.y = vertex1WallCollision.y;
    }
    return corner;
}

Vector2 GetWallCollision(vertex_data_t vertex, Vector2 playerPos)
{
    Vector2 wallCollision;
    // touches top of screen
    float radian = vertex.radian;
    float playerEdgeDX = playerPos.x - vertex.pos.x;
    float playerEdgeDY = playerPos.y - vertex.pos.y;
    if (radian >= 7 * PI / 4 || radian < PI / 4)
    {

        float multiplier = playerPos.y / playerEdgeDY;
        wallCollision.x = playerPos.x - playerEdgeDX * multiplier;
        wallCollision.y = 0;
    }
    // touches right of screen
    else if (radian >= PI / 4 && radian < 3 * PI / 4)
    {

        float multiplier = (screenWidth - playerPos.x) / playerEdgeDX;
        wallCollision.x = screenWidth;
        wallCollision.y = playerPos.y + playerEdgeDY * multiplier;
    }
    // touches bottom of screen
    else if (radian >= 3 * PI / 4 && radian < 5 * PI / 4)
    {
        float multiplier = (screenHeight - playerPos.y) / playerEdgeDY;
        wallCollision.x = playerPos.x + playerEdgeDX * multiplier;
        wallCollision.y = screenHeight;
    }
    // touches left of screen
    else
    {
        float multiplier = playerPos.x / playerEdgeDX;
        wallCollision.x = 0;
        wallCollision.y = playerPos.y - playerEdgeDY * multiplier;
    }
    return wallCollision;
}

void AddHiddenRectangle(edge_t edge, Vector2 playerPos, triangle_t *hiddenTriangles, int *trianglesCount, int currentRect)
{
    Vector2 vertex1WallCollision = GetWallCollision(edge.vertex1, playerPos);
    Vector2 vertex2WallCollision = GetWallCollision(edge.vertex2, playerPos);
    // if they are on opposide side of each other left/right
    if (fabs(vertex1WallCollision.x - vertex2WallCollision.x) == screenWidth)
    {
        // if cone goes downwards
        if (edge.vertex1.radian > PI / 2)
        {
            Vector2 leftBotCorner = {0, screenHeight};
            Vector2 rightBotCorner = {screenWidth, screenHeight};
            hiddenTriangles[*trianglesCount] = (triangle_t){vertex1WallCollision, leftBotCorner, vertex2WallCollision, RED};
            *trianglesCount += 1;
            hiddenTriangles[*trianglesCount] = (triangle_t){leftBotCorner, rightBotCorner, vertex2WallCollision, BLUE};
            *trianglesCount += 1;
        }
        else
        {
            Vector2 leftTopCorner = {0, 0};
            Vector2 rightTopCorner = {screenWidth, 0};
            hiddenTriangles[*trianglesCount] = (triangle_t){vertex1WallCollision, rightTopCorner, vertex2WallCollision, RED};
            *trianglesCount += 1;
            hiddenTriangles[*trianglesCount] = (triangle_t){vertex2WallCollision, rightTopCorner, leftTopCorner, BLUE};
            *trianglesCount += 1;
        }
    }
    // if they are on opposide side of each other top/bot
    else if (fabs(vertex1WallCollision.y - vertex2WallCollision.y) == screenHeight)
    {
        // if cone goes to the right
        if (edge.vertex2.radian < PI)
        {
            Vector2 rightTopCorner = {screenWidth, 0};
            Vector2 rightBotCorner = {screenWidth, screenHeight};
            hiddenTriangles[*trianglesCount] = (triangle_t){rightTopCorner, vertex2WallCollision, vertex1WallCollision, RED};
            *trianglesCount += 1;
            hiddenTriangles[*trianglesCount] = (triangle_t){rightTopCorner, vertex1WallCollision, rightBotCorner, BLUE};
            *trianglesCount += 1;
        }
        else
        {
            Vector2 leftTopCorner = {0, 0};
            Vector2 leftBotCorner = {0, screenHeight};
            hiddenTriangles[*trianglesCount] = (triangle_t){leftTopCorner, vertex2WallCollision, vertex1WallCollision, RED};
            *trianglesCount += 1;
            hiddenTriangles[*trianglesCount] = (triangle_t){leftBotCorner, vertex2WallCollision, leftTopCorner, BLUE};
            *trianglesCount += 1;
        }
    }
    // if they are on different sides of the screen (not adjacent)
    else if (vertex1WallCollision.x != vertex2WallCollision.x && vertex1WallCollision.y != vertex2WallCollision.y)
    {
        Vector2 corner = GetCornerBetweenWallIntersects(vertex1WallCollision, vertex2WallCollision);
        hiddenTriangles[*trianglesCount] = (triangle_t){vertex1WallCollision, corner, vertex2WallCollision, RED};
        *trianglesCount += 1;
    }
    hiddenTriangles[*trianglesCount] = (triangle_t){edge.vertex2.pos, edge.vertex1.pos, vertex1WallCollision, YELLOW};
    *trianglesCount += 1;
    hiddenTriangles[*trianglesCount] = (triangle_t){vertex2WallCollision, edge.vertex2.pos, vertex1WallCollision, PURPLE};
    *trianglesCount += 1;
}

void CreateHiddenTrianglesFromRays(edge_t *edges, triangle_t *visibleTriangles, Vector2 playerPos, int edgesCount, int *trianglesCount)
{
    double lastRadian = 0;
    for (int i = 0; i < edgesCount; i++)
    {
        AddHiddenRectangle(edges[i], playerPos, visibleTriangles, trianglesCount, i);
    }
}

void CreateTrianglesFromRays(edge_t *edges, triangle_t *visibleTriangles, Vector2 playerPos, int edgesCount, int *trianglesCount)
{
    // keeps track of the radian of the latest edge in order to avoid recalculating edges
    vertex_data_t lastVertex = {0};
    for (int i = 0; i < edgesCount; i++)
    {
        // lastVertex.radian = 0;
        //  if the algorithm has gone past this edge
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
        double side1SmallestRadian = edges[i].vertex1.radian < edges[i].vertex2.radian ? edges[i].vertex1.radian : edges[i].vertex2.radian;
        int minIndex = i;
        for (int j = i + 1; j < edgesCount; j++)
        {
            double side2SmallestRadian = edges[j].vertex1.radian < edges[j].vertex2.radian ? edges[j].vertex1.radian : edges[j].vertex2.radian;
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

    corner->distanceFromPlayer = pow(dX * dX + dY * dY, 1 / 2);

    double radian = atan(dX / dY);

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

    /*for (int i = 0; i < 4; i++)
    {
        sides[*validSides].vertex1.pos = mapCorners[i];
        GetCornerData(&sides[*validSides].vertex1, playerPos, rectsPlaced);
        sides[*validSides].vertex2.pos = mapCorners[(i + 1) % 4];
        GetCornerData(&sides[*validSides].vertex2, playerPos, rectsPlaced);
        *validSides += 1;
    }*/
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
    edge_t edges[maxRects * 4 + 4] = {0}; // stores all sides on the map, including map border sides
    int edgesCount = 0;

    int trianglesCount = 0;
    Vector2 mouseToRectStartDistance = {0, 0};
    Vector2 mouseToRectEndDistance = {0, 0};
    rect_t testRect = {(Vector2){200, 200},
                       (Vector2){500, 500}};
    PlaceRect(rects, testRect, &rectsPlaced);
    rect_t testRect2 = {(Vector2){700, 700},
                        (Vector2){1000, 800}};
    PlaceRect(rects, testRect2, &rectsPlaced);
    while (!WindowShouldClose())
    {
        triangle_t visibleTriangles[maxRects * 4] = {0}; // unsure if this will be sufficient memory, if seg faults happen check this value
        edgesCount = 0;
        trianglesCount = 0;
        ClearBackground(LIGHTGRAY);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            MoveObjectOrPlayer(&playerPos, rects, rectsPlaced, &mouseToRectStartDistance, &mouseToRectEndDistance);
        }
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            RemoveRectMovement(rects, rectsPlaced);
        }
        if (IsKeyPressed(KEY_F))
        {
            ToggleFullscreen();
        }
        GetCornerVectors(playerPos, edges, rects, rectsPlaced, &edgesCount);
        SortCornersByDistance(edges, edgesCount);
        // SortCornersByRadian(edges, edgesCount);
        // CreateTrianglesFromRays(edges, visibleTriangles, playerPos, edgesCount, &trianglesCount);
        CreateHiddenTrianglesFromRays(edges, visibleTriangles, playerPos, edgesCount, &trianglesCount);
        BeginDrawing();

        DrawRects(rects, rectsPlaced);
#ifdef DEV_MODE
        DrawRays(playerPos, edges, edgesCount);
        DrawMapCorners();
#endif
        DrawVisibleTriangles(visibleTriangles, trianglesCount);
        DrawCircleV(playerPos, playerSize, YELLOW);
        EndDrawing();
        for (int i = 0; i < edgesCount; i++)
        {
            edges[i] = (edge_t){(vertex_data_t){0}, (vertex_data_t){0}, 0};
        }
    }
    CloseWindow();
    return 0;
}