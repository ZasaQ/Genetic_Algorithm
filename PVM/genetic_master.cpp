#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <numeric>
#include <random>
#include <iostream>
#include <pvm3.h>

#define SLAVE_NUM 10
#define POPULATION_SIZE 100
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
    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        Polygon inPolygon;
        do
        {
            inPolygon.vertices.clear();
            int NumOfGeneratedVertices = verticesAmountGeneratedPerPolygon(3, 12);

            for (int j = 0; j < NumOfGeneratedVertices; j++)
            {
                float x = randFloat(0.0f, 10.0f);
                float y = randFloat(0.0f, 10.0f);
                inPolygon.vertices.push_back(Point(x,y));
            }
        } while (!isPolygonConvex(inPolygon));

        population.push_back(inPolygon);
    }
}

void distributePopulation(std::vector<Polygon>& population)
{
    int ptid = pvm_mytid();

    int mutationRate = MUTATION_RATE;
    int generationNum = GENERATIONS_NUM;

    int populationSize = population.size();
    int populationChunkSize = populationSize / SLAVE_NUM;

    std::vector<Polygon> populationChunk(populationChunkSize);
    int dataTag = 1;

    for (int i = 0; i < SLAVE_NUM; i++)
    {
        int startIndex = i * populationChunkSize;
        int endIndex = (i + 1) * populationChunkSize - 1;

        std::copy(population.begin() + startIndex, population.begin() + endIndex + 1, populationChunk.begin());

        int tid = ptid + (i + 1);

        pvm_initsend(PvmDataDefault);
        pvm_pkint(&populationChunkSize, 1, 1);
        pvm_pkbyte(populationChunk.data(), populationChunkSize * sizeof(Polygon), 1);
        pvm_pkint(&mutationRate, 1, 1);
        pvm_pkint(&generationNum, 1, 1);

        pvm_send(tid, dataTag);
    }
}

void receiveEvaluationResults(std::vector<std::vector<Polygon>>& results)
{
    int ptid = pvm_mytid();
    int dataTag = 2;

    for (int i = 0; i < SLAVE_NUM; i++)
    {
        int tid = ptid + (i + 1);
        int bufSize = 0;

        pvm_probe(tid, dataTag);
        pvm_upkint(&bufSize, 1, 1);

        std::vector<Polygon> slaveResults;
        slaveResults.reserve(bufSize);

        for (int j = 0; j < bufSize; j++) {
            Polygon polygon;
            int verticesSize = 0;

            pvm_upkint(&verticesSize, 1, 1);

            pvm_upkbyte(polygon.vertices.data(), verticesSize * sizeof(Point), 1);

            slaveResults.push_back(polygon);
        }

        results.push_back(slaveResults);
    }
}

std::ostream& operator << (std::ostream& out, std::vector<Polygon>& Polygon)
{
    int i = 0;

    out << "Num of Polygons: " << Polygon.size() << "\n";

    for (auto& InPolygon : Polygon)
    {
        out << "Polygon (" << i << "):\n";

        for (auto& InVertex : InPolygon.vertices)
        {
            out << "x = " << InVertex.x << ",\t y = " << InVertex.y << "\n";
        }
        i++;

        out << "\n";
    }

    return out;
}

int main()
{
    std::vector<Point> initialPolygonVertices = {
            {-5.0f, 5.0f},
            {0.0f, 10.0f},
            {5.0f, 5.0f},
            {7.0f, 2.0f},
            {10.0f, 0.0f},
            {7.0f, -2.0f},
            {5.0f, -5.0f},
            {0.0f, -10.0f},
            {-5.0f, -5.0f},
            {-7.0f, -2.0f},
            {-10.0f, 0.0f},
            {-7.0f, 2.0f}
    };
    Polygon initialPolygon = initialPolygonVertices;
    std::vector<Polygon> population;
    std::vector<std::vector<Polygon>> result;

    initializePolygons(population);
    distributePopulation(population);

    receiveEvaluationResults(result);

    pvm_exit();
    return 0;
}
