#include <iostream>
#include <vector>


//TODO: To jest ew. do poprawy, oba structy mam na mysli, wydaje mi sie ze mozna lepiej by to zrobic
struct Point {
    double x, y;
};

struct Polygon {
    std::vector<Point> points;
};

// Sprawdza, czy punkt C znajduje siê na lewo od odcinka AB
bool isLeft(const Point& A, const Point& B, const Point& C) {
    double crossProduct = (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);

    return crossProduct > 0;
}

// Sprawdza, czy punkt C znajduje siê na prawo od odcinka AB
bool isRight(const Point& A, const Point& B, const Point& C) {
    double crossProduct = (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);

    return crossProduct < 0;
}

// Sprawdza, czy odcinki AB i CD przecinaj¹ siê
bool doIntersect(const Point& A, const Point& B, const Point& C, const Point& D) {
    bool isACLeftOfBD = isLeft(A, B, C) && isLeft(A, B, D);
    bool isBDLeftOfAC = isLeft(C, D, A) && isLeft(C, D, B);

    return !isACLeftOfBD && !isBDLeftOfAC;
}

// Sprawdza, czy wielok¹t A jest zawarty wewn¹trz wielok¹ta B
bool isInside(const Point& P, const Polygon& poly) {
    int numPoints = poly.points.size();
    bool inside = false;

    for (int i = 0, j = numPoints - 1; i < numPoints; j = i++) {
        if (((poly.points[i].y > P.y) != (poly.points[j].y > P.y)) &&
            (P.x < (poly.points[j].x - poly.points[i].x) * (P.y - poly.points[i].y) / (poly.points[j].y - poly.points[i].y) + poly.points[i].x))
            inside = !inside;
    }
    return inside;
}

// Oblicza maksymaln¹ liczbê nachodz¹cych na siebie wielok¹tów wypuk³ych
int calculateMaxOverlappingPolygons(const std::vector<Polygon>& polygons) {
    int maxOverlapping = 0;
    int numPolygons = polygons.size();

    // SprawdŸ, ile wielok¹tów jest zawartych wewn¹trz ka¿dego innego wielok¹ta
    for (int i = 0; i < numPolygons; ++i) {
        int overlapping = 0;
        for (int j = 0; j < numPolygons; ++j) {
            if (i != j && isInside(polygons[i].points[0], polygons[j]))
                overlapping++;
        }
        maxOverlapping = std::max(maxOverlapping, overlapping);
    }

    return maxOverlapping;
}

int main() {
    // Przyk³adowe dane wejœciowe
    std::vector<Polygon> polygons;

    Polygon polygon1{ { {0, 0}, {4, 0}, {4, 4}, {0, 4} } };
    Polygon polygon2{ { {1, 1}, {3, 1}, {3, 3}, {1, 3} } };
    Polygon polygon3{ { {2, 2}, {4, 2}, {4, 4}, {2, 4} } };
    polygons.push_back(polygon1);
    polygons.push_back(polygon2);
    polygons.push_back(polygon3);

    int maxOverlapping = calculateMaxOverlappingPolygons(polygons);
    std::cout << "Max overlapping polygons: " << maxOverlapping << std::endl;

    return 0;
}
