#ifndef RED_H
#define RED_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include "enrutador.h"


class Red
{
public:
    std::unordered_map<std::string, Enrutador*> enrutadores;

    ~Red();

    void agregarEnrutador(const std::string& nombre);

    void eliminarEnrutador(const std::string& nombre);

    void actualizarEnlace(const std::string& origen, const std::string& destino, int costo);

    int calcularCosto(const std::string& origen, const std::string& destino);

    std::vector<std::string> obtenerRuta(const std::string& origen, const std::string& destino);

    void cargarDesdeArchivo(const std::string& nombreArchivo);

    void generarRedAleatoria(int numeroEnrutadores);

    void imprimirTablaVecinos();

    void eliminarConexion(const std::string& origen, const std::string& destino);

};

#endif // RED_H