#include <stdio.h>
#include <stdlib.h>
#include <pvm3.h>
#include <conio.h>

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
    Polygon(const vector<Point>& vertices) : vertices(vertices) {}
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

int verticesNumGenerator(int min, int max)
{
    return min + (rand() % (max - min + 1));
}

void initializePolygons(std::vector<Polygon>& polygons) {
    for (auto& InPolygon : polygons)
    {
        int NumOfGeneratedVertices = verticesNumGenerator(3, 12);

        for (int i : NumOfGenerataedVertices)
        {
            float x = randomPlacement(0.0f, 10.0f);
            float y = randomPlacement(0.0f, 10.0f);
            InPolygon.vertices.push_back(Point(x,y));
        }
    }
}

void evaluatePolygons(std::vector<Polygon>& polygons) {
    for (int i = 0; i < SLAVE_COUNT; i++) {
        pvm_initsend(PvmDataDefault);
        pvm_pkbyte((char*)polygons.data(), sizeof(Polygon) * polygons.size(), 1);
        pvm_send(slave_tid[i], 1);
    }

    // Odbierz ocenione wielokąty od węzłów slave
    for (int i = 0; i < SLAVE_COUNT; i++) {
        pvm_recv(slave_tid[i], 1);
        pvm_upkbyte((char*)polygons.data(), sizeof(Polygon) * polygons.size(), 1);
    }

}

int main(int argc, char **argv) {
    int tid, parent;
    int count = MAX_POLYGONS;
    vector<Polygon> polygons[MAX_POLYGONS];
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

    tid = pvm_mytid();
    parent = pvm_parent();

    if (parent == PvmNoParent) {
        // Kod dla węzła master

        // Inicjalizacja PVM
        pvm_catchout(stdout);
        pvm_recv(-1, 1);
        pvm_upkint(&count, 1, 1);

        // Inicjalizacja populacji wielokątów
        initializePolygons(polygons, count);

        // Rozesłanie danych do slave'ów
        int slaveCount = SLAVE_COUNT;
        int i, start = 0, end;
        int chunkSize = count / SLAVE_COUNT;

        for (i = 0; i < SLAVE_COUNT; i++) {
            if (i == SLAVE_COUNT - 1)
                end = count - 1;
            else
                end = start + chunkSize - 1;

            pvm_initsend(PvmDataDefault);
            pvm_pkint(&count, 1, 1);
            pvm_pkint(&start, 1, 1);
            pvm_pkint(&end, 1, 1);
            pvm_pkbyte((char*)polygons, sizeof(ConvexPolygon) * count, 1);
            pvm_send(tid, 1);

            start = end + 1;
        }

        // Odbiór ocenionych wielokątów od slave'ów
        for (i = 0; i < SLAVE_COUNT; i++) {
            pvm_recv(tid, 1);
            pvm_upkbyte((char*)polygons, sizeof(ConvexPolygon) * count, 1);
            evaluatePolygons(polygons, count);
        }

        // Wybór najlepszych wielokątów i ewentualne zakończenie algorytmu

    } else {
        // Kod dla węzłów slave

        // Odbiór danych od mastera
        pvm_recv(parent, 1);
        pvm_upkint(&count, 1, 1);
        pvm_upkint(&start, 1, 1);
        pvm_upkint(&end, 1, 1);
        pvm_upkbyte((char*)polygons, sizeof(Polygon) * count, 1);

        // Ocenianie wielokątów
        evaluatePolygons(polygons, count);

        // Wysyłanie ocenionych danych do mastera
        pvm_initsend(PvmDataDefault);
        pvm_pkbyte((char*)polygons, sizeof(Polygon) * count, 1);
        pvm_send(parent, 1);
    }

    pvm_exit();
    return 0;
}
