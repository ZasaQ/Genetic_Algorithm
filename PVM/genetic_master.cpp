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

void initializePolygons(std::vector<Polygon>& polygons)
{
    for (auto& InPolygon : polygons)
    {
        int NumOfGeneratedVertices = verticesAmountGeneratedPerPolygon(3, 12);

        for (int i = 0; i < NumOfGeneratedVertices; i++)
        {
            float x = randFloat(0.0f, 10.0f);
            float y = randFloat(0.0f, 10.0f);
            InPolygon.vertices.push_back(Point(x,y));
        }
    }
}

void evaluatePolygons(std::vector<Polygon>& polygons)
{
    
}

int main(int argc, char **argv)
{

}
