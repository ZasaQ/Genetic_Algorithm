#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <numeric>
#include <ctime>


struct Point 
{
    float x, y;

    Point(float x, float y) : x(x), y(y) {}
};

struct Polygon 
{
    std::vector<Point> vertices;

    Polygon() {}
    Polygon(const std::vector<Point>& vertices) : vertices(vertices) {}
};

/*
    template <class ForwardIterator, class T>
    void iota(ForwardIterator first, ForwardIterator last, T val)
    {
        while (first != last) {
            *first = val;
            ++first;
            ++val;
        }
    }
*/

std::vector<Point> computeLineSegmentRectangleIntersections(float p0x, float p0y, float p1x, float p1y, float r0x, float r0y, float r1x, float r1y)
{
    std::vector<Point> intersections;

    // Obliczanie r�wna� linii
    float dx = p1x - p0x;
    float dy = p1y - p0y;
    float drx = r1x - r0x;
    float dry = r1y - r0y;

    float determinant = dx * dry - drx * dy;

    // Sprawdzanie czy odcinek i prostok�t s� r�wnoleg�e
    if (determinant == 0)
    {
        return intersections; // Brak przeci��
    }

    // Obliczanie parametr�w przeci�cia
    float t = ((r0x - p0x) * dry - (r0y - p0y) * drx) / determinant;
    float u = ((r0x - p0x) * dy - (r0y - p0y) * dx) / determinant;

    // Sprawdzanie czy przeci�cie mie�ci si� na odcinku i w prostok�cie
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) 
    {
        float intersectionX = p0x + t * dx;
        float intersectionY = p0y + t * dy;
        intersections.push_back(Point(intersectionX, intersectionY));
    }

    return intersections;
}

std::vector<float> computeLineRectangleIntersections(float p0x, float p0y, float p1x, float p1y, float r0x, float r0y, float r1x, float r1y)
{
    std::vector<float> intersections;

    // Obliczanie r�wna� linii
    float dx = p1x - p0x;
    float dy = p1y - p0y;
    float drx = r1x - r0x;
    float dry = r1y - r0y;

    float determinant = dx * dry - drx * dy;

    // Sprawdzanie czy odcinek i prostok�t s� r�wnoleg�e
    if (determinant == 0)
    {
        return intersections; // Brak przeci��
    }

    // Obliczanie parametr�w przeci�cia
    float t = ((r0x - p0x) * dry - (r0y - p0y) * drx) / determinant;
    float u = ((r0x - p0x) * dy - (r0y - p0y) * dx) / determinant;

    // Sprawdzanie czy przeci�cie mie�ci si� na odcinku i w prostok�cie
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
    {
        float intersectionX = p0x + t * dx;
        float intersectionY = p0y + t * dy;
        intersections.push_back(intersectionX);
        intersections.push_back(intersectionY);
    }

    return intersections;
}

// Funkcja zwaracj�ca liczb� przeci�� mi�dzy wielok�tami, zastosowany zosta� tutaj algorytm SAT(Separating Axis Theorem)
int countIntersections(const Polygon& poly1, const Polygon& poly2)
{
    int intersectionCount = 0;

    // Sprawdzanie przeci�� dla ka�dej kraw�dzi obu wielok�t�w
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

            std::vector<float> intersections = computeLineRectangleIntersections(x0, y0, x1, y1, x2, y2, x3, y3);
            if (!intersections.empty())
            {
                intersectionCount += intersections.size() / 2;
            }
        }
    }

    return intersectionCount;
}

// Funkcja obliczaj�ca ocen� dostosowania (fitness) osobnika
int fitnessFunction(const std::vector<Polygon>& population)
{
    int totalIntersections = 0;

    // Obliczanie liczby przeci�� dla ka�dej pary osobnik�w w populacji
    for (size_t i = 0; i < population.size(); ++i)
    {
        for (size_t j = i + 1; j < population.size(); ++j)
        {
            totalIntersections += countIntersections(population[i], population[j]);
        }
    }

    // Im mniejsza liczba przeci��, tym lepsze dostosowanie
    return totalIntersections;
}

// Funkcja selekcji osobnik�w
std::vector<Polygon> selection(const std::vector<Polygon>& population, int numParents)
{
    // Sprawd� poprawno�� parametr�w
    if (numParents <= 0 || numParents > population.size())
    {
        throw std::invalid_argument("Nieprawid�owa liczba rodzic�w.");
    }

    // Utw�rz kopi� populacji, aby unikn�� modyfikacji oryginalnej populacji
    std::vector<Polygon> populationCopy = population;

    // Sortuj populacj� w kolejno�ci rosn�cej na podstawie liczby nachodz�cych na siebie maksymalnych wielok�t�w (lambda)
    std::sort(populationCopy.begin(), populationCopy.end(), [&](const Polygon& a, const Polygon& b)
        {
            int aIntersections = countIntersections(a, populationCopy[0]);
            int bIntersections = countIntersections(b, populationCopy[0]);
            return aIntersections < bIntersections;
        }
    );

    // Wybierz okre�lon� liczb� rodzic�w z posortowanej populacji
    std::vector<Polygon> parents;
    for (int i = 0; i < numParents; ++i)
    {
        parents.push_back(populationCopy[i]);
    }

    return parents;
}

// Funkcja krzy�owania osobnik�w
std::vector<Polygon> crossover(const std::vector<Polygon>& parents)
{
    std::vector<Polygon> offspring;

    // Sprawdzenie, czy liczba rodzic�w jest wystarczaj�ca
    if (parents.size() < 2)
    {
        return offspring; // Zwr�� pust� populacj� potomstwa
    }

    // Wyb�r punktu krzy�owania
    size_t crossoverPoint = rand() % parents[0].vertices.size();

    // Krzy�owanie rodzic�w
    for (size_t i = 0; i < parents.size() - 1; i += 2)
    {
        const Polygon& parent1 = parents[i];
        const Polygon& parent2 = parents[i + 1];

        Polygon child(parent1);

        // Skopiowanie cz�ci genotypu z rodzica 1 przed punktem krzy�owania
        for (size_t j = 0; j < crossoverPoint; ++j)
        {
            child.vertices.push_back(parent1.vertices[j]);
        }

        // Skopiowanie cz�ci genotypu z rodzica 2 po punkcie krzy�owania
        for (size_t j = crossoverPoint; j < parent2.vertices.size(); ++j)
        {
            child.vertices.push_back(parent2.vertices[j]);
        }

        offspring.push_back(child);
    }

    return offspring;
}

float randFloat(float min, float max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

float randomFloat(float min, float max)
{
    static bool randOnce = true;

    if (randOnce)
    {
        std::srand(static_cast<unsigned int>(std::time(NULL)));
        randOnce = false;
    }
    float normalized = static_cast<float>(std::rand()) / RAND_MAX;
    return min + normalized * (max - min);
}

// Funkcja mutacji osobnik�w
void mutate(std::vector<Polygon>& population, float mutationRate) 
{
    for (auto& polygon : population)
    {
        // Dla ka�dego osobnika w populacji

        for (auto& vertex : polygon.vertices)
        {
            // Dla ka�dego wierzcho�ka w osobniku

            if (randFloat(0, 1) < mutationRate)
            {
                // Wykonaj mutacj� z prawdopodobie�stwem mutationRate

                float newX = vertex.x + randFloat(-1, 1);
                float newY = vertex.y + randFloat(-1, 1);

                vertex.x = newX;
                vertex.y = newY;
            }
        }
    }
}

void randomPlacement(Polygon& polygon, float minX, float maxX, float minY, float maxY)
{
    // Resetowanie wierzcho�k�w wielok�ta
    // polygon.vertices.clear();

    // Losowe rozmieszczenie wierzcho�k�w
    for (size_t i = 0; i < polygon.vertices.size(); ++i)
    {
        float x = randFloat(minX, maxX);
        float y = randFloat(minY, maxY);
        polygon.vertices[i] = Point(x, y);
    }
}

// G��wna funkcja algorytmu genetycznego
std::vector<Polygon> geneticAlgorithm(const Polygon& initialPolygon, int populationSize, int numGenerations, float mutationRate)
{
    std::vector<Polygon> population(populationSize);

    // Inicjalizacja populacji pocz�tkowej
    for (int i = 0; i < populationSize; ++i)
    {
        Polygon polygon = initialPolygon;
        randomPlacement(polygon, 0.0f, 1.0f, 0.0f, 1.0f); // Losowe rozmieszczenie wierzcho�k�w w zakresie [0, 1]
        population[i] = polygon;
    }

    for (int generation = 0; generation < numGenerations; ++generation)
    {
        int fitness = fitnessFunction(population);

        std::vector<Polygon> parents = selection(population, populationSize / 2);
        std::vector<Polygon> offspring = crossover(parents);

        mutate(offspring, mutationRate);
        population = offspring;
    }

    return population;
}

std::ostream& operator << (std::ostream& out, std::vector<Polygon>& Polygon)
{
    return out << Polygon.size();
}

int main() {
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

    int populationSize = 100;
    int numGenerations = 50;
    float mutationRate = 0.1f;

    // Wywo�anie algorytmu genetycznego
    std::vector<Polygon> result = geneticAlgorithm(initialPolygon, populationSize, numGenerations, mutationRate);

    std::cout << result;

    return 0;
}
