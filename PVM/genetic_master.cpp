#include <stdio.h>
#include <stdlib.h>
#include <pvm3.h>
#include <vector>
#include <numeric>
#include <random>

#define SLAVE_COUNT 10
#define MAX_POLYGONS 100
#define GENERATIONS_NUM 3
#define MUTATION_RATE 0.1f

struct Point 
{
    float x, y;

    Point(float x, float y) : x(x), y(y) {}
};

struct Polygon 
{
    std::vector<Point> vertices;

    Polygon() = default;
    Polygon(const std::vector<Point>& vertices) : vertices(vertices) {}
};

bool isPolygonConvex(const Polygon& polygon) {
    const std::vector<Point>& vertices = polygon.vertices;
    int vertexCount = vertices.size();

    if (vertexCount < 3) {
        return false;
    }

    bool isConvex = true;
    for (int i = 0; i < vertexCount; i++) {
        const Point& p1 = vertices[i];
        const Point& p2 = vertices[(i + 1) % vertexCount];
        const Point& p3 = vertices[(i + 2) % vertexCount];

        float crossProduct = (p2.x - p1.x) * (p3.y - p2.y) - (p2.y - p1.y) * (p3.x - p2.x);

        if (crossProduct < 0) {
            return false;
        }
    }

    return true;
}

float randFloat(float min, float max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

// Funkcja losowo rozmieszczajï¿œca wierzchoï¿œki wielokï¿œta
void randomPlacement(Polygon& polygon, float minX, float maxX, float minY, float maxY)
{
    for (auto& InVertex : polygon.vertices)
    {
        float x = randFloat(minX, maxX);
        float y = randFloat(minY, maxY);
        InVertex = Point(x, y);
    }
}

int verticesAmountGeneratedPerPolygon(int min, int max)
{
    return min + (rand() % (max - min + 1));
}

void initializePolygons(std::vector<Polygon>& population)
{
    for (auto& inPolygon : population)
    {
        int NumOfGeneratedVertices = verticesAmountGeneratedPerPolygon(3, 12);

        for (int i = 0; i < NumOfGeneratedVertices; i++)
        {
            float x = randFloat(0.0f, 10.0f);
            float y = randFloat(0.0f, 10.0f);
            inPolygon.vertices.push_back(Point(x,y));
        }


    }
}

void evaluatePolygons(std::vector<Polygon>& polygons)
{
    
}

int main(int argc, char **argv)
{

}
