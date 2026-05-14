#include "red.h"
#include "enrutador.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <stdexcept>

Red::~Red() {
    for (auto& enrutador : enrutadores) delete enrutador.second;
}

// ─── agregarEnrutador ─────────────────────────────────────────────────────────
void Red::agregarEnrutador(const std::string& nombre) {
    if (nombre.empty()) {
        std::cerr << "[Error] El nombre del enrutador no puede estar vacío.\n";
        return;
    }
    if (enrutadores.find(nombre) != enrutadores.end()) {
        std::cout << "[Aviso] El enrutador '" << nombre << "' ya existe.\n";
        return;
    }
    enrutadores[nombre] = new Enrutador(nombre);
}

// ─── eliminarEnrutador ────────────────────────────────────────────────────────
void Red::eliminarEnrutador(const std::string& nombre) {
    auto it = enrutadores.find(nombre);
    if (it == enrutadores.end()) {
        std::cerr << "[Error] El enrutador '" << nombre << "' no existe.\n";
        return;
    }
    // Quitar este nodo de los vecinos de todos los demás
    for (auto& [nombreVecino, enrutadorVecino] : enrutadores) {
        if (nombreVecino != nombre)
            enrutadorVecino->eliminarVecino(it->second);
    }
    delete it->second;
    enrutadores.erase(it);
}

// ─── actualizarEnlace ─────────────────────────────────────────────────────────
void Red::actualizarEnlace(const std::string& origen,
                           const std::string& destino,
                           int costo) {
    if (origen == destino) {
        std::cerr << "[Error] Un nodo no puede conectarse consigo mismo.\n";
        return;
    }
    if (costo <= 0) {
        std::cerr << "[Error] El costo debe ser un valor positivo (> 0).\n";
        return;
    }
    auto itOrigen  = enrutadores.find(origen);
    auto itDestino = enrutadores.find(destino);

    if (itOrigen == enrutadores.end()) {
        std::cerr << "[Error] El enrutador origen '" << origen << "' no existe.\n";
        return;
    }
    if (itDestino == enrutadores.end()) {
        std::cerr << "[Error] El enrutador destino '" << destino << "' no existe.\n";
        return;
    }

    itOrigen->second->agregarVecino(itDestino->second, costo);
    itDestino->second->agregarVecino(itOrigen->second, costo);
}

// ─── calcularCosto ────────────────────────────────────────────────────────────
int Red::calcularCosto(const std::string& origen, const std::string& destino) {
    if (enrutadores.find(origen) == enrutadores.end()) {
        std::cerr << "[Error] Enrutador origen '" << origen << "' no existe.\n";
        return -1;
    }
    if (enrutadores.find(destino) == enrutadores.end()) {
        std::cerr << "[Error] Enrutador destino '" << destino << "' no existe.\n";
        return -1;
    }
    if (origen == destino) return 0;

    // Inicializar Dijkstra
    for (auto& [nombre, enrutador] : enrutadores) {
        enrutador->distancia = std::numeric_limits<int>::max();
        enrutador->visitado  = false;
        enrutador->anterior  = nullptr;
    }
    enrutadores[origen]->distancia = 0;

    using ParDistancia = std::pair<int, Enrutador*>;
    std::priority_queue<ParDistancia,
                        std::vector<ParDistancia>,
                        std::greater<>> cola;
    cola.push({0, enrutadores[origen]});

    while (!cola.empty()) {
        auto [dist, actual] = cola.top();
        cola.pop();

        if (actual->visitado) continue;
        actual->visitado = true;

        for (const auto& [vecino, costo] : actual->vecinos) {
            // Guardia contra desbordamiento de enteros
            if (actual->distancia != std::numeric_limits<int>::max()) {
                int nuevaDist = actual->distancia + costo;
                if (nuevaDist < vecino->distancia) {
                    vecino->distancia = nuevaDist;
                    vecino->anterior  = actual;
                    cola.push({nuevaDist, vecino});
                }
            }
        }
    }

    return enrutadores[destino]->distancia;
}

// ─── obtenerRuta ──────────────────────────────────────────────────────────────
std::vector<std::string> Red::obtenerRuta(const std::string& origen,
                                          const std::string& destino) {
    std::vector<std::string> ruta;

    if (enrutadores.find(origen) == enrutadores.end() ||
        enrutadores.find(destino) == enrutadores.end()) {
        std::cerr << "[Error] Uno de los nodos no existe.\n";
        return ruta;
    }
    if (origen == destino) {
        ruta.push_back(origen);
        return ruta;
    }

    // Re-ejecutar Dijkstra para tener los punteros 'anterior' actualizados
    int costo = calcularCosto(origen, destino);
    if (costo == std::numeric_limits<int>::max() || costo < 0) {
        std::cout << "[Info] No existe ruta entre '" << origen
                  << "' y '" << destino << "'.\n";
        return ruta;
    }

    Enrutador* actual = enrutadores[destino];
    // Protección contra ciclos (no debería ocurrir, pero por seguridad)
    int pasos = 0;
    int maxPasos = static_cast<int>(enrutadores.size()) + 1;
    while (actual != nullptr && pasos <= maxPasos) {
        ruta.push_back(actual->nombre);
        actual = actual->anterior;
        ++pasos;
    }
    std::reverse(ruta.begin(), ruta.end());
    return ruta;
}

// ─── cargarDesdeArchivo ───────────────────────────────────────────────────────
void Red::cargarDesdeArchivo(const std::string& nombreArchivo) {
    if (nombreArchivo.empty()) {
        std::cerr << "[Error] Nombre de archivo vacío.\n";
        return;
    }

    std::ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        std::cerr << "[Error] No se pudo abrir el archivo '"
                  << nombreArchivo << "'. Verifique que existe.\n";
        return;
    }

    std::string linea;
    int lineaNum = 0;
    int cargadas = 0;

    while (std::getline(archivo, linea)) {
        ++lineaNum;
        if (linea.empty() || linea[0] == '#') continue; // comentarios/blancos

        std::istringstream ss(linea);
        std::string origen, destino;
        int costo;

        if (!(ss >> origen >> destino >> costo)) {
            std::cerr << "[Aviso] Linea " << lineaNum
                      << " mal formada, se omite: '" << linea << "'\n";
            continue;
        }
        if (costo <= 0) {
            std::cerr << "[Aviso] Linea " << lineaNum
                      << ": costo invalido (" << costo << "), se omite.\n";
            continue;
        }
        if (origen == destino) {
            std::cerr << "[Aviso] Línea " << lineaNum
                      << ": auto-enlace ignorado (" << origen << ").\n";
            continue;
        }

        agregarEnrutador(origen);
        agregarEnrutador(destino);
        actualizarEnlace(origen, destino, costo);
        ++cargadas;
    }

    archivo.close();
    std::cout << "[OK] Archivo cargado: " << cargadas << " enlace(s) procesado(s).\n";
}

// ─── generarRedAleatoria ──────────────────────────────────────────────────────
void Red::generarRedAleatoria(int numeroEnrutadores) {
    if (numeroEnrutadores <= 0) {
        std::cerr << "[Error] El numero de enrutadores debe ser positivo.\n";
        return;
    }
    if (numeroEnrutadores > 26) {
        std::cerr << "[Aviso] Maximo 26 nodos (A-Z). Se generaran 26.\n";
        numeroEnrutadores = 26;
    }

    srand(static_cast<unsigned>(time(nullptr)));

    for (int i = 0; i < numeroEnrutadores; ++i) {
        std::string nodoStr(1, static_cast<char>('A' + i));
        agregarEnrutador(nodoStr);
    }

    // Garantizar conectividad: cadena base A-B-C-...-Z
    for (int i = 0; i < numeroEnrutadores - 1; ++i) {
        std::string n1(1, static_cast<char>('A' + i));
        std::string n2(1, static_cast<char>('A' + i + 1));
        int costo = rand() % 10 + 1;
        actualizarEnlace(n1, n2, costo);
    }

    // Agregar enlaces adicionales aleatorios
    for (int i = 0; i < numeroEnrutadores; ++i) {
        for (int j = i + 1; j < numeroEnrutadores; ++j) {
            if (rand() % 2 == 0) {
                std::string n1(1, static_cast<char>('A' + i));
                std::string n2(1, static_cast<char>('A' + j));
                int costo = rand() % 10 + 1;
                actualizarEnlace(n1, n2, costo);
            }
        }
    }
}

// ─── imprimirTablaVecinos ─────────────────────────────────────────────────────
void Red::imprimirTablaVecinos() {
    if (enrutadores.empty()) {
        std::cout << "[Aviso] La red está vacia.\n";
        return;
    }

    std::cout << "\n=== Tabla de vecinos ===\n";
    for (const auto& [nombreEnrutador, enrutador] : enrutadores) {
        std::cout << "Nodo " << nombreEnrutador << ":\n";
        if (enrutador->vecinos.empty()) {
            std::cout << "  (sin vecinos)\n";
        } else {
            for (const auto& [vecino, costo] : enrutador->vecinos) {
                std::cout << "  -> " << vecino->nombre
                          << "  [costo: " << costo << "]\n";
            }
        }
    }
    std::cout << "========================\n";
}

// ─── eliminarConexion ─────────────────────────────────────────────────────────
void Red::eliminarConexion(const std::string& origen,
                           const std::string& destino) {
    auto itOrigen  = enrutadores.find(origen);
    auto itDestino = enrutadores.find(destino);

    if (itOrigen == enrutadores.end()) {
        std::cerr << "[Error] El enrutador '" << origen << "' no existe.\n";
        return;
    }
    if (itDestino == enrutadores.end()) {
        std::cerr << "[Error] El enrutador '" << destino << "' no existe.\n";
        return;
    }

    Enrutador* eOrigen  = itOrigen->second;
    Enrutador* eDestino = itDestino->second;

    if (eOrigen->vecinos.find(eDestino) == eOrigen->vecinos.end()) {
        std::cerr << "[Error] No existe un enlace entre '"
                  << origen << "' y '" << destino << "'.\n";
        return;
    }

    eOrigen->eliminarVecino(eDestino);
    eDestino->eliminarVecino(eOrigen);
}