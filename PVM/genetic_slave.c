#include <stdio.h>
#include <stdlib.h>
#include <pvm3.h>
#include <vector.h>
#include <conio.h>

struct Point 
{
    float x, y;

    Point(float x, float y) : x(x), y(y) {}
};

struct Polygon 
{
    vector<Point> vertices;

    Polygon() = default;
    Polygon(const vector<Point>& vertices) : vertices(vertices) {}
};

float randFloat(float min, float max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

// Funkcja odpowiadajï¿œca za obliczanie przeciï¿œï¿œ dwï¿œch odcinkï¿œw
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


// Funkcja zwaracjï¿œca liczbï¿œ przeciï¿œï¿œ miï¿œdzy wielokï¿œtami, zastosowany zostaï¿œ tutaj algorytm SAT(Separating Axis Theorem)
int countIntersections(const Polygon& poly1, const Polygon& poly2)
{
    int intersectionCount = 0;

    // Sprawdzanie przeciï¿œï¿œ dla kaï¿œdej krawï¿œdzi obu wielokï¿œtï¿œw
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

// Funkcja obliczajï¿œca ocenï¿œ dostosowania (fitness) osobnika, im mniejsza liczba przeciï¿œï¿œ, tym lepsze dostosowanie
int fitnessFunction(const std::vector<Polygon>& population)
{
    int totalIntersections = 0;

    // Obliczanie liczby przeciï¿œï¿œ dla kaï¿œdej pary osobnikï¿œw w populacji
    for (size_t i = 0; i < population.size(); ++i)
    {
        for (size_t j = i + 1; j < population.size(); ++j)
        {
            totalIntersections += countIntersections(population[i], population[j]);
        }
    }

    return totalIntersections;
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

// Funkcja selekcji osobnikï¿œw
std::vector<Polygon> selection(const std::vector<Polygon>& population, int numParents)
{
    // Sprawdï¿œ poprawnoï¿œï¿œ parametrï¿œw
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

// Funkcja krzyï¿œowania osobnikï¿œw
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

        // Kopiujemy czï¿œï¿œ genotypu z rodzica 1 przed punktem krzyï¿œowania
        for (size_t j = 0; j < crossoverPoint; ++j)
        {
            child.vertices.push_back(parent1.vertices[j]);
        }

        // Kopiujemy czï¿œci genotypu z rodzica 2 po punkcie krzyï¿œowania
        for (size_t j = crossoverPoint; j < parent2.vertices.size(); ++j)
        {
            child.vertices.push_back(parent2.vertices[j]);
        }

        offspring.push_back(child);
    }

    return offspring;
}

// Funkcja mutacji osobnikï¿œw
void mutate(std::vector<Polygon>& population, float mutationRate) 
{
    for (auto& polygon : population)
    {
        for (auto& vertex : polygon.vertices)
        {
            if (randFloat(0, 1) < mutationRate)
            {
                // Wykonujemy mutacjï¿œ z prawdopodobieï¿œstwem mutationRate
                float newX = vertex.x + randFloat(-1, 1);
                float newY = vertex.y + randFloat(-1, 1);

                vertex.x = newX;
                vertex.y = newY;
            }
        }
    }
}

int main(int argc, char **argv) {
    int tid, parent;
    int count, start, end;
    ConvexPolygon polygons[MAX_POLYGONS];

    tid = pvm_mytid();
    parent = pvm_parent();

    // Odbiór danych od mastera
    pvm_recv(parent, 1);
    pvm_upkint(&count, 1, 1);
    pvm_upkint(&start, 1, 1);
    pvm_upkint(&end, 1, 1);
    pvm_upkbyte((char*)polygons, sizeof(ConvexPolygon) * count, 1);

    // Ocenianie wielokątów
    evaluatePolygons(polygons, count);

    // Wysyłanie ocenionych danych do mastera
    pvm_initsend(PvmDataDefault);
    pvm_pkbyte((char*)polygons, sizeof(ConvexPolygon) * count, 1);
    pvm_send(parent, 1);

    pvm_exit();
    return 0;
}
