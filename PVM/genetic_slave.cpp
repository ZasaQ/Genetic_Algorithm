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

// Funkcja zwaracajaca liczbe przeciec miedzy wielokatami, zastosowany zostal tutaj algorytm SAT (Separating Axis Theorem)
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

void receiveInitializedPopulation(std::vector<Polygon>& population, int& inGenerationNum, float& inMutationRate)
{
    std::cout << "Na poczatku receiveInitializedPopulation\n";

    int tid = pvm_mytid();

    pvm_recv(pvm_parent(), 2);

    int chunkSize = 0;
    float mutationRate = 0.0f;
    int generationNum = 0;

    std::cout << "Przed chunkSize\n";

    pvm_upkint(&chunkSize, 1, 1);

    std::cout << "Po chunkSize\n";

    std::vector<Polygon> chunk(chunkSize);

    pvm_upkbyte(reinterpret_cast<char*>(chunk.data()), chunkSize * sizeof(Polygon), 1);
    pvm_upkfloat(&mutationRate, 1, 1);
    pvm_upkint(&generationNum, 1, 1);

    population.insert(population.end(), chunk.begin(), chunk.end());

    inMutationRate = mutationRate;
    inGenerationNum = generationNum;
}

std::vector<Polygon> evaluatePolygons(std::vector<Polygon>& population, int& numGeneration, float& mutationRate)
{
    for (int generation = 0; generation < numGeneration; ++generation)
    {
        std::vector<Polygon> parents = selection(population, population.size() / 2);
        std::vector<Polygon> offspring = crossover(parents);

        mutate(offspring, mutationRate);
        population = offspring;
    }

    return population;
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

void sendEvaluationResult(const std::vector<Polygon>& results, clock_t time)
{
    std::cout << "Na poczatku sendEvaluationResult\n";

    int tid = pvm_mytid();

    int activeSBuf = pvm_initsend(PvmDataDefault);

    pvm_pkint(reinterpret_cast<int*>(results.size()), 1, 1);
    //pvm_pklong(&time, 1, 1);

    for (const auto& polygon : results) {
        pvm_pkbyte(const_cast<char*>(reinterpret_cast<const char*>(polygon.vertices.data())), polygon.vertices.size() * sizeof(Point), 1);
    }

    pvm_send(pvm_parent(), tid);
    pvm_freebuf(activeSBuf);
}

int main() {
    std::vector<Polygon> population;
    std::vector<Polygon> evaluationResult;

    float mutationRate = 0.0f;
    int generationNum = 0;

    clock_t now = clock();

    receiveInitializedPopulation(population, generationNum, mutationRate);
    evaluationResult = evaluatePolygons(population, generationNum, mutationRate);
    std::cout << population;

    clock_t timer = clock() - now;
    sendEvaluationResult(evaluationResult, timer);

    pvm_exit();
    return 0;
}
