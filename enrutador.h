// Enrutador.h
#ifndef ENRUTADOR_H
#define ENRUTADOR_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <queue>
#include <limits>
#include <algorithm>

class Enrutador {
public:
    std::string nombre;
    int distancia;
    bool visitado;
    Enrutador* anterior;
    std::unordered_map<Enrutador*, int> vecinos;

    Enrutador(const std::string& nombre);

    void agregarVecino(Enrutador* vecino, int costo);

    void eliminarVecino(Enrutador* vecino);
};

#endif // ENRUTADOR_H