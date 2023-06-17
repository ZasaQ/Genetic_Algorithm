#include <stdlib.h>
#include <vector>
#include <numeric>
#include <random>
#include <iostream>
#include <pvm3.h>
#include <ctime>

struct Point
{
    float x, y;

    Point() = default;
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

std::vector<float> computeLineRectangleIntersections(float p0x, float p0y, float p1x, float p1y, float r0x, float r0y, float r1x, float r1y)
{
    std::vector<float> intersections;

    float dx = p1x - p0x;
    float dy = p1y - p0y;
    float drx = r1x - r0x;
    float dry = r1y - r0y;

    float determinant = dx * dry - drx * dy;

    if (determinant == 0)
    {
        return intersections;
    }

    float t = ((r0x - p0x) * dry - (r0y - p0y) * drx) / determinant;
    float u = ((r0x - p0x) * dy - (r0y - p0y) * dx) / determinant;

    if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
    {
        float intersectionX = p0x + t * dx;
        float intersectionY = p0y + t * dy;

        intersections.push_back(intersectionX);
        intersections.push_back(intersectionY);
    }

    return intersections;
}

int countIntersections(const Polygon& poly1, const Polygon& poly2)
{
    int intersectionCount = 0;

    for (size_t i = 0; i < poly1.vertices.size(); ++i)
    {
        size_t j = (i + 1) % poly1.vertices.size();

        float x0 = poly1.vertices[i].x;
        float y0 = poly1.vertices[i].y;
        float x1 = poly1.vertices[j].x;
        float y1 = poly1.vertices[j].y;

        for (size_t k = 0; k < poly2.vertices.size(); ++k)
        {
            size_t l = (k + 1) % poly2.vertices.size();

            float x2 = poly2.vertices[k].x;
            float y2 = poly2.vertices[k].y;
            float x3 = poly2.vertices[l].x;
            float y3 = poly2.vertices[l].y;

            std::vector<float> intersections = computeLineRectangleIntersections(x0, y0,
                                                                                 x1, y1,
                                                                                 x2, y2,
                                                                                 x3, y3);
            if (!intersections.empty())
            {
                intersectionCount += intersections.size() / 2;
            }
        }
    }

    return intersectionCount;
}

std::vector<Polygon> sortPopulation(const std::vector<Polygon>& population)
{
    std::vector<Polygon> sortedPopulation = population;
    int n = sortedPopulation.size();

    for (int i = 0; i < n - 1; ++i)
    {
        for (int j = 0; j < n - i - 1; ++j)
        {
            int intersections1 = countIntersections(sortedPopulation[j], sortedPopulation[0]);
            int intersections2 = countIntersections(sortedPopulation[j + 1], sortedPopulation[0]);

            if (intersections1 > intersections2)
            {
                std::swap(sortedPopulation[j], sortedPopulation[j + 1]);
            }
        }
    }

    return sortedPopulation;
}

std::vector<Polygon> selection(const std::vector<Polygon>& population, int numParents)
{
    if (numParents <= 0 || numParents > population.size())
    {
        numParents = population.size() / 2;
    }

    std::vector<Polygon> populationCopy = population;

    sortPopulation(populationCopy);

    std::vector<Polygon> parents;
    parents.reserve(numParents);

    for (int i = 0; i < numParents; ++i)
    {
        parents.push_back(populationCopy[i]);
    }

    return parents;
}

std::vector<Polygon> crossover(const std::vector<Polygon>& parents)
{
    std::vector<Polygon> offspring;

    if (parents.size() < 2)
    {
        return offspring;
    }

    size_t crossoverPoint = rand() % parents[0].vertices.size();

    for (size_t i = 0; i < parents.size() - 1; i += 2)
    {
        const Polygon& parent1 = parents[i];
        const Polygon& parent2 = parents[i + 1];

        Polygon child;

        for (size_t j = 0; j < crossoverPoint; ++j)
        {
            child.vertices.push_back(parent1.vertices[j]);
        }

        for (size_t j = crossoverPoint; j < parent2.vertices.size(); ++j)
        {
            child.vertices.push_back(parent2.vertices[j]);
        }

        offspring.push_back(child);
    }

    return offspring;
}

void mutate(std::vector<Polygon>& population, float mutationRate)
{
    for (auto& polygon : population)
    {
        for (auto& vertex : polygon.vertices)
        {
            if (randFloat(0, 1) < mutationRate)
            {
                float newX = vertex.x + randFloat(-1, 1);
                float newY = vertex.y + randFloat(-1, 1);

                vertex.x = newX;
                vertex.y = newY;
            }
        }
    }
}

void sendPolygon(Polygon& inPolygon, int inTid, int inDataTag)
{
    pvm_initsend(PvmDataDefault);
    int inPolygonSize = inPolygon.vertices.size();

    for (int i = 0; i < inPolygonSize; i++)
    {
        Point& vertex = inPolygon.vertices[i];
        pvm_pkfloat(&vertex.x, 1, 1);
        pvm_pkfloat(&vertex.y, 1, 1);
    }

    pvm_send(inTid, inDataTag);
}

void receivePolygon(Polygon& inPolygon, int inTid, int inDataTag)
{
    pvm_recv(inTid, inDataTag);
    int inPolygonSize = inPolygon.vertices.size();

    for (int i = 0; i < inPolygonSize; i++)
    {
        Point vertex;

        pvm_upkfloat(&vertex.x, 1, inDataTag);
        pvm_upkfloat(&vertex.y, 1, inDataTag);

        inPolygon.vertices[i] = vertex;
    }
}

void receiveInitializedPopulation(std::vector<Polygon>& populationToEvaluate, float& inMutationRate, int& inGenerationNum)
{
    std::cout << "Na poczatku receiveInitializedPopulation\n";
    int pTid = pvm_parent();

    int receivedPopulationChunkSize = 0;
    float mutationRate = 0.0f;
    int generationNum = 0;

    std::cout << "Przed chunkSize\n";
    pvm_recv(pTid, 1);
    pvm_upkint(&receivedPopulationChunkSize, 1, 1);

    std::cout << "Po chunkSize\n";

    std::vector<Polygon> receivedPopulationChunk(receivedPopulationChunkSize);

    for (auto& receivedPolygonChunk : receivedPopulationChunk)
    {
        pvm_recv(pTid, 2);
        int eachPolygonChunkVerticesSize;
        pvm_upkint(&eachPolygonChunkVerticesSize, 1, 2);

        pvm_recv(pTid, 3);
        receivedPolygonChunk.vertices = std::vector<Point>(eachPolygonChunkVerticesSize / sizeof(Point));
        pvm_upkbyte(reinterpret_cast<char*>(receivedPolygonChunk.vertices.data()), eachPolygonChunkVerticesSize * sizeof(Point), 3);
    }

    pvm_recv(pTid, 4);
    pvm_upkfloat(&mutationRate, 1, 4);

    pvm_recv(pTid, 5);
    pvm_upkint(&generationNum, 1, 5);

    populationToEvaluate.insert(populationToEvaluate.begin(), receivedPopulationChunk.begin(), receivedPopulationChunk.end());

    inMutationRate = mutationRate;
    inGenerationNum = generationNum;
}

std::vector<Polygon> evaluatePolygons(std::vector<Polygon>& population, float& mutationRate, int& generationNum)
{
    for (int generation = 0; generation < generationNum; ++generation)
    {
        std::vector<Polygon> parents = selection(population, population.size() / 2);
        std::vector<Polygon> offspring = crossover(parents);

        mutate(offspring, mutationRate);
        population = offspring;
    }

    return population;
}

void sendEvaluationResult(std::vector<Polygon>& evaluationResult, clock_t time)
{
    int resultSize = evaluationResult.size();

    pvm_initsend(PvmDataDefault);
    pvm_pkint(&resultSize, 1, 1);
    pvm_send(pvm_parent(), 1);

    for (auto& eachEvaluatedPolygon : evaluationResult)
    {
        pvm_initsend(PvmDataDefault);
        int eachEvaluatedPolygonVerSize = eachEvaluatedPolygon.vertices.size();
        pvm_pkint(&eachEvaluatedPolygonVerSize, 1, 2);
        pvm_send(pvm_parent(), 2);

        pvm_initsend(PvmDataDefault);
        pvm_pkbyte(reinterpret_cast<char*>(eachEvaluatedPolygon.vertices.data()), eachEvaluatedPolygonVerSize * sizeof(Point), 3);
        pvm_send(pvm_parent(), 3);
    }
}

int main() {
    srand(time(0) + pvm_mytid());

    std::vector<Polygon> populationToEvaluate;
    std::vector<Polygon> evaluationResult;

    float mutationRate = 0.0f;
    int generationNum = 0;

    clock_t now = clock();

    receiveInitializedPopulation(populationToEvaluate, mutationRate, generationNum);
    evaluationResult = evaluatePolygons(populationToEvaluate, mutationRate, generationNum);

    clock_t timer = clock() - now;
    sendEvaluationResult(evaluationResult, timer);

    pvm_exit();
    return 0;
}
