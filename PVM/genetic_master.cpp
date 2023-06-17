#include <stdlib.h>
#include <vector>
#include <random>
#include <iostream>
#include <pvm3.h>

#define SLAVE_NUM 1
#define POPULATION_SIZE 100
#define GENERATIONS_NUM 3
#define MUTATION_RATE 0.1f
#define SLAVE "genetic_slave"

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

std::ostream& operator << (std::ostream& out, Polygon& Polygon)
{
    for (auto& InVertex : Polygon.vertices)
    {
        out << "x = " << InVertex.x << ",\t y = " << InVertex.y << "\n";
    }

    return out;
}

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
    return min + rand() % (max - min + 1);
}

void sendPolygon(Polygon& inPolygon, int inTid)
{
    int inPolygonSize = inPolygon.vertices.size();

    for (int i = 0; i < inPolygonSize; i++)
    {
        Point& vertex = inPolygon.vertices[i];
        pvm_initsend(PvmDataDefault);
        pvm_pkfloat(&vertex.x, 1, 1);
        pvm_send(inTid, 3);

        pvm_initsend(PvmDataDefault);
        pvm_pkfloat(&vertex.y, 1, 1);
        pvm_send(inTid, 3);
    }
}

void receivePolygon(Polygon& inPolygon, int inTid)
{
    int inPolygonSize = inPolygon.vertices.size();

    for (int i = 0; i < inPolygonSize; i++)
    {
        Point vertex;

        pvm_recv(inTid, 3);
        pvm_upkfloat(&vertex.x, 1, 1);

        pvm_recv(inTid, 3);
        pvm_upkfloat(&vertex.y, 1, 1);

        inPolygon.vertices[i] = vertex;
    }
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

void distributePopulation(std::vector<Polygon>& populationToEvaluate, int (&tIds)[SLAVE_NUM])
{
    std::cout << "Na poczatku distributePopulation\n";

    int pTid = pvm_mytid();
    int num_slaves, num_arch;
    struct pvmhostinfo *slaves;
    int a = 0;

    pvm_config(&num_slaves, &num_arch, &slaves);
    int taskNum = pvm_spawn(SLAVE, (char**)NULL, PvmTaskDefault, "", SLAVE_NUM, tIds);

    if (taskNum <= 0)
    {
        std::cout << pTid << ": Cant create slaves\n";
        pvm_perror("pvm_spawn");

        pvm_exit();
        return;
    }

    int populationSize = populationToEvaluate.size();
    int populationChunkSize = populationSize / SLAVE_NUM;

    float mutationRate = MUTATION_RATE;
    int generationNum = GENERATIONS_NUM;

    for (int i = 0; i < taskNum; i++)
    {
        std::vector<Polygon> toSendPopulationChunk(populationChunkSize);

        int startIndex = i * populationChunkSize;
        int endIndex = (i + 1) * populationChunkSize - 1;

        //std::copy(populationToEvaluate.begin() + startIndex, populationToEvaluate.begin() + endIndex, populationChunk.begin());
        toSendPopulationChunk.assign(populationToEvaluate.begin() + startIndex, populationToEvaluate.begin() + endIndex);

        pvm_initsend(PvmDataDefault);
        pvm_pkint(&populationChunkSize, 1, 1);
        pvm_send(tIds[i], 1);

        /*
        for (int j = 0; j < populationChunkSize; j++)
        {
            Polygon toSendEachPolygonChunk = toSendPopulationChunk[j];
            int eachPolygonChunkVerSize = toSendEachPolygonChunk.vertices.size();

            pvm_initsend(PvmDataDefault);
            pvm_pkint(&eachPolygonChunkVerSize, 1, 1);
            pvm_send(tIds[i], 2);

            pvm_recv(tIds[i], 1);
            pvm_upkint(&a, 1, 1);
            std::cout << "a: " << a << "\n";

            //sendPolygon(toSendEachPolygonChunk, tIds[i], 3);

            pvm_initsend(PvmDataDefault);
            pvm_pkbyte(reinterpret_cast<char*>(toSendEachPolygonChunk.vertices.data()), eachPolygonChunkVerSize * sizeof(Point), 1);
            pvm_send(tIds[i], 3);
        }
        */


        for (auto& toSendEachPolygonChunk : toSendPopulationChunk)
        {
            int eachPolygonChunkVerSize = toSendEachPolygonChunk.vertices.size();

            pvm_initsend(PvmDataDefault);
            pvm_pkint(&eachPolygonChunkVerSize, 1, 1);
            pvm_send(tIds[i], 2);

            sendPolygon(toSendEachPolygonChunk, tIds[i]);
        }


        pvm_initsend(PvmDataDefault);
        pvm_pkfloat(&mutationRate, 1, 1);
        pvm_send(tIds[i], 4);

        pvm_initsend(PvmDataDefault);
        pvm_pkint(&generationNum, 1, 1);
        pvm_send(tIds[i], 5);
    }
    std::cout << "Na koncu distributePopulation\n";
}

void receiveEvaluationResult(std::vector<std::vector<Polygon>>& results, int (&tIds)[SLAVE_NUM])
{
    std::cout << "Start receiveEvaluationResult\n";
    for (int i = 0; i < SLAVE_NUM; i++)
    {
        std::cout << "Start petla, i: " << tIds[i] << "\n";
        int receivedPolygonSize;
        pvm_recv(tIds[i], 1);
        std::cout << "Odebrane\n";
        pvm_upkint(&receivedPolygonSize, 1, 1);
        std::cout << "Odebrane i rozpakowane receivedPolygonSize\n";

        std::vector<Polygon> receivedPolygons(receivedPolygonSize);

        for (auto& eachReceivedPolygon : receivedPolygons)
        {
            std::cout << "eachReceivedPolygon\n";
            pvm_recv(tIds[i], 2);
            int eachReceivedPolygonVerSize;
            pvm_upkint(&eachReceivedPolygonVerSize, 1, 1);
            std::cout << "Odebrane i rozpakowane eachReceivedPolygonVerSize\n";


            receivePolygon(eachReceivedPolygon, tIds[i]);
        }

        results.push_back(receivedPolygons);
    }
}

int main()
{
    int tIds[SLAVE_NUM];
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

    pvm_catchout(stdout);

    Polygon initialPolygon = initialPolygonVertices;
    std::vector<Polygon> population;
    std::vector<std::vector<Polygon>> result;

    initializePolygons(population);
    distributePopulation(population, tIds);
    std::cout << tIds[0] << "\n";
    receiveEvaluationResult(result, tIds);

    for (auto& polygons : result)
    {
        std::cout << polygons << std::endl;
    }

    pvm_exit();
    return 0;
}
