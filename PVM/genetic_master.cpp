#include <stdlib.h>
#include <vector>
#include <random>
#include <iostream>
#include <pvm3.h>
#include <iomanip>

#define SLAVE_NUM 1
#define POPULATION_SIZE 1000
#define GENERATIONS_NUM 5
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
            out << "x = " << std::setprecision(4) << InVertex.x << ",\t y = " << std::setprecision(4) << InVertex.y << "\n";
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
        out << "x = " << std::setprecision(4) << InVertex.x << ",\t y = " << std::setprecision(4) << InVertex.y << "\n";
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

void sendPolygon(Polygon& inPolygon, int inTid, int dataTag)
{
    int inPolygonSize = inPolygon.vertices.size();

    for (int i = 0; i < inPolygonSize; i++)
    {
        Point vertex = inPolygon.vertices[i];

        pvm_initsend(PvmDataDefault);
        pvm_pkfloat(&vertex.x, 1, 1);
        pvm_send(inTid, dataTag);

        pvm_initsend(PvmDataDefault);
        pvm_pkfloat(&vertex.y, 1, 1);
        pvm_send(inTid, dataTag);
    }
}

void receivePolygon(Polygon& inPolygon, int inTid, int dataTag)
{
    int inPolygonSize = inPolygon.vertices.size();

    for (int i = 0; i < inPolygonSize; i++)
    {
        Point vertex;

        pvm_recv(inTid, dataTag);
        pvm_upkfloat(&vertex.x, 1, 1);

        pvm_recv(inTid, dataTag);
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

void distributePopulation(std::vector<Polygon>& populationToEvaluate, Polygon& inInitialPolygon, int (&tIds)[SLAVE_NUM])
{
    int pTid = pvm_mytid();

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

    int inInitialPolygonSize = inInitialPolygon.vertices.size();

    for (int i = 0; i < taskNum; i++)
    {
        std::vector<Polygon> toSendPopulationChunk(populationChunkSize);

        int startIndex = i * populationChunkSize;
        int endIndex = (i + 1) * populationChunkSize;

        toSendPopulationChunk.assign(populationToEvaluate.begin() + startIndex, populationToEvaluate.begin() + endIndex);

        pvm_initsend(PvmDataDefault);
        pvm_pkint(&populationChunkSize, 1, 1);
        pvm_send(tIds[i], 1);

        for (int j = 0; j < populationChunkSize; j++)
        {
            Polygon toSendEachPolygonChunk = toSendPopulationChunk[j];

            int toSendEachPolygonChunkSize = toSendEachPolygonChunk.vertices.size();
            pvm_initsend(PvmDataDefault);
            pvm_pkint(&toSendEachPolygonChunkSize, 1, 1);
            pvm_send(tIds[i], 2);

            sendPolygon(toSendEachPolygonChunk, tIds[i], 3);
        }

        pvm_initsend(PvmDataDefault);
        pvm_pkfloat(&mutationRate, 1, 1);
        pvm_send(tIds[i], 4);

        pvm_initsend(PvmDataDefault);
        pvm_pkint(&generationNum, 1, 1);
        pvm_send(tIds[i], 5);

        pvm_initsend(PvmDataDefault);
        pvm_pkint(&inInitialPolygonSize, 1, 1);
        pvm_send(tIds[i], 6);

        sendPolygon(inInitialPolygon, tIds[i], 7);
    }
}

void receiveEvaluationResult(std::vector<Polygon>& results, int (&tIds)[SLAVE_NUM])
{
    for (int i = 0; i < SLAVE_NUM; i++)
    {
        int receivedPolygonSize;
        pvm_recv(tIds[i], 1);
        pvm_upkint(&receivedPolygonSize, 1, 1);

        std::vector<Polygon> receivedPolygons(receivedPolygonSize);

        for (int j = 0; j < receivedPolygonSize; j++)
        {
            Polygon eachReceivedPolygon = receivedPolygons[j];

            int eachReceivedPolygonSize;
            pvm_recv(tIds[i], 2);
            pvm_upkint(&eachReceivedPolygonSize, 1, 1);

            eachReceivedPolygon.vertices.resize(eachReceivedPolygonSize);

            receivePolygon(eachReceivedPolygon, tIds[i], 3);
            results.push_back(eachReceivedPolygon);
        }
    }
}

int main()
{
    int tIds[SLAVE_NUM];
    struct timeval start_time, end_time;
    int num_slaves, num_arch;
    struct pvmhostinfo *slaves;

    pvm_config(&num_slaves, &num_arch, &slaves);
    gettimeofday(&start_time, NULL);

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
    std::vector<Polygon> result;

    initializePolygons(population);
    distributePopulation(population, initialPolygon, tIds);
    receiveEvaluationResult(result, tIds);

    std::cout << result << std::endl;

    gettimeofday(&end_time, NULL);
    long elapsed_time = (double)(end_time.tv_sec - start_time.tv_sec) * 1000 + ((double)(end_time.tv_usec - start_time.tv_usec) / 1000);

    FILE *fp = fopen("genetic_execution_times.csv", "a");
    fprintf(fp, "Slaves: %d; Elapsed time: %ld\n", num_slaves - 1, elapsed_time);
    fclose(fp);

    std::cout << "Num of Slaves used in process: " << num_slaves - 1 << "\nElapsed time: " << elapsed_time << " milliseconds\n";

    pvm_exit();
    return 0;
}
