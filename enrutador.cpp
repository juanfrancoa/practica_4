#include "enrutador.h"
#include <climits>
#include <queue>
#include <vector>
#include <stack>
#include <iostream>
#include <iomanip>

Enrutador::Enrutador(const std::string& nombre)
    : nombre(nombre), distancia(std::numeric_limits<int>::max()), visitado(false), anterior(nullptr) {}

void Enrutador::agregarVecino(Enrutador* vecino, int costo) {
    vecinos[vecino] = costo;
}

void Enrutador::eliminarVecino(Enrutador* vecino) {
    vecinos.erase(vecino);
}