#include <iostream>
#include <vector>
#include <algorithm>

//TODO: To jest ew. do poprawy, oba structy mam na mysli, wydaje mi sie ze mozna lepiej by to zrobic
struct Point {
    double x, y;
};

struct Polygon {
    std::vector<Point> points;
};

// Funkcja sprawdza, czy punkt C znajduje siê na lewo od odcinka AB, poza wnêtrzem wielok¹ta.
bool isLeft(const Point& A, const Point& B, const Point& C) {
    double crossProduct = (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);

    return crossProduct > 0;
}

// Funkcja sprawdza, czy punkt C znajduje siê na prawo od odcinka AB, poza wnêtrzem wielok¹ta.
bool isRight(const Point& A, const Point& B, const Point& C) {
    double crossProduct = (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);

    return crossProduct < 0;
}

// Funkcja sprawdza, czy punkt C le¿y na odcinku AB.
bool isOnSegment(const Point& A, const Point& B, const Point& C) {
    return ((C.x >= std::min(A.x, B.x)) && (C.x <= std::max(A.x, B.x)) &&
        (C.y >= std::min(A.y, B.y)) && (C.y <= std::max(A.y, B.y)) &&
        ((B.x - A.x) * (C.y - A.y) == (B.y - A.y) * (C.x - A.x)));
}

// Funkcja sprawdza, czy odcinki AB i CD przecinaj¹ siê
bool doIntersect(const Point& A, const Point& B, const Point& C, const Point& D) {
    if (isOnSegment(A, B, C) || isOnSegment(A, B, D) || isOnSegment(C, D, A) || isOnSegment(C, D, B)) {
        return false;
    }

    return (isLeft(A, B, C) != isLeft(A, B, D)) && (isLeft(C, D, A) != isLeft(C, D, B));
}

// Funkcja sprawdza, czy punkt P jest zawarty wewn¹trz wielok¹ta poly.
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

// Funkcja oblicza przeciêcie odcinka AB z wielok¹tem polygon i zwraca przeciêty fragment jako nowy wielok¹t.
Polygon getIntersectionPolygon(const Point& A, const Point& B, const Polygon& polygon) {
    Polygon intersectedPolygon;
    const size_t n = polygon.points.size();

    for (size_t i = 0; i < n; ++i) {
        size_t nextIndex = (i + 1) % n;
        const Point& C = polygon.points[i];
        const Point& D = polygon.points[nextIndex];

        if (doIntersect(A, B, C, D)) {
            // Zmienne przechowuj¹ wspó³rzêdne punktów przeciêcia odcinków AB i CD:
            // Dla X
            double intersectionX = 
                ((C.x * D.y - C.y * D.x) * (A.x - B.x) -
                (A.x * B.y - A.y * B.x) * (C.x - D.x)) /
                ((C.x - D.x) * (A.y - B.y) - (A.x - B.x) * (C.y - D.y));
            // Dla Y
            double intersectionY = 
                ((C.x * D.y - C.y * D.x) * (A.y - B.y) -
                (A.x * B.y - A.y * B.x) * (C.y - D.y)) /
                ((C.x - D.x) * (A.y - B.y) - (A.x - B.x) * (C.y - D.y));

            intersectedPolygon.points.push_back({ intersectionX, intersectionY });
        }
    }

    return intersectedPolygon;
}

// Funkcja oblicza minimaln¹ liczbê nachodz¹cych na siebie maksymalnych wielok¹tów wypuk³ych pokrywaj¹cych dany wielok¹t.
int getMinimumOverlappingPolygons(const Polygon& polygon) {
    const size_t n = polygon.points.size();
    std::vector<std::vector<int>> overlappingPolygons(n, std::vector<int>(n, 0));

    for (size_t i = 2; i < n; ++i) {
        for (size_t j = 0; j < n - i; ++j) {
            size_t k = i + j;
            overlappingPolygons[j][k] = std::numeric_limits<int>::max();

            for (size_t m = j + 1; m < k; ++m) {
                overlappingPolygons[j][k] = std::min(
                    overlappingPolygons[j][k],
                    overlappingPolygons[j][m] + overlappingPolygons[m][k] + 1
                );
            }

            Polygon intersectionPolygon = getIntersectionPolygon(
                polygon.points[j], polygon.points[k], polygon
            );

            if (!intersectionPolygon.points.empty()) {
                overlappingPolygons[j][k] = std::min(
                    overlappingPolygons[j][k],
                    getMinimumOverlappingPolygons(intersectionPolygon)
                );
            }
        }
    }

    return overlappingPolygons[0][n - 1];
}


int main() {
    // Przyk³adowe dane wejœciowe
    std::vector<Polygon> polygons;

    Polygon polygon;
    polygon.points = { { {0, 0}, {4, 0}, {4, 4}, {0, 4} } };

    int minimumOverlappingPolygons = getMinimumOverlappingPolygons(polygon);
    std::cout << "Minimalna liczba nachodzacych na siebie maksymalnych wielokatow: " << minimumOverlappingPolygons << std::endl;

    return 0;
}