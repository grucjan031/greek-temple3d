#ifndef MYCUBE_H_INCLUDED
#define MYCUBE_H_INCLUDED


//myCubeVertices - homogeniczne współrzędne wierzchołków w przestrzeni modelu
//myCubeNormals - homogeniczne wektory normalne ścian (per wierzchołek) w przestrzeni modelu
//myCubeVertexNormals - homogeniczne wektory normalne wierzchołków w przestrzeni modelu
//myCubeTexCoords - współrzędne teksturowania
//myCubeColors - kolory wierzchołków
//myCubeC1 - kolumna macierzy TBN^-1
//myCubeC2 - kolumna macierzy TBN^-1
//myCubeC3 - kolumna macierzy TBN^-1

int floorVertexCount = 6; // 2 trójkąty x 3 wierzchołki = 6

float floorVerticess[] = {
    // Wierzchołki podłogi
    -500.0f,  0.0f, -500.0f, 1.0f,
    -500.0f,  0.0f,  500.0f, 1.0f,
     500.0f,  0.0f,  500.0f, 1.0f,
     500.0f,  0.0f,  500.0f, 1.0f,
     500.0f,  0.0f, -500.0f, 1.0f,
    -500.0f,  0.0f, -500.0f, 1.0f,
};

float floorColors[] = {
    // Kolory podłogi (na razie takie same dla uproszczenia)
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
};

float floorTexCoords[] = {
    // Współrzędne tekstur podłogi
    0.0f, 100.0f,
    0.0f, 0.0f,
    100.0f, 0.0f,
    100.0f, 0.0f,
    100.0f, 100.0f,
    0.0f, 100.0f,
};

float floorNormals[] = {
    // Normale podłogi (w górę)
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
};

#endif // MYCUBE_H_INCLUDED
