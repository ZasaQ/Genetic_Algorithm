#include <stdio.h>
#include <stdlib.h>
#include <pvm3.h>
#include <vector.h>

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

void evaluatePolygons(vector<Polygon> polygons, int count) {
    
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
