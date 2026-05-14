#include <iostream>
#include <vector>
#include <limits>
#include <cctype>
#include "red.h"
#include "enrutador.h"
#include <algorithm>
#include <string>

using namespace std;

// ─── Utilidades de lectura segura ─────────────────────────────────────────────

// Lee un entero; si la entrada es inválida limpia el buffer y repite.
int leerEntero(const string& prompt) {
    int valor;
    while (true) {
        cout << prompt;
        if (cin >> valor) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return valor;
        }
        cout << "[Error] Ingrese un numero entero válido.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Lee un entero positivo (> 0).
int leerEnteroPositivo(const string& prompt) {
    int valor;
    while (true) {
        valor = leerEntero(prompt);
        if (valor > 0) return valor;
        cout << "[Error] El valor debe ser mayor que 0.\n";
    }
}

// Lee exactamente una letra (a-z / A-Z) y la convierte a mayúscula.
string leerNodoLetra(const string& prompt) {
    string entrada;
    while (true) {
        cout << prompt;
        cin >> entrada;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (entrada.size() == 1 && isalpha((unsigned char)entrada[0])) {
            entrada[0] = toupper((unsigned char)entrada[0]);
            return entrada;
        }
        cout << "[Error] Ingrese una sola letra (A-Z).\n";
    }
}

// Lee un entero dentro de un rango [min, max].
int leerOpcion(const string& prompt, int minVal, int maxVal) {
    int valor;
    while (true) {
        valor = leerEntero(prompt);
        if (valor >= minVal && valor <= maxVal) return valor;
        cout << "[Error] Opcion fuera de rango. Ingrese entre "
             << minVal << " y " << maxVal << ".\n";
    }
}

// ─── Helpers de menú ──────────────────────────────────────────────────────────

// Pide dos nodos distintos y valida que existan en la red.
// Retorna false si alguno no existe.
bool pedirDosNodosExistentes(const Red& red,
                             const string& promptA, const string& promptB,
                             string& nodoA, string& nodoB) {
    nodoA = leerNodoLetra(promptA);
    nodoB = leerNodoLetra(promptB);

    if (nodoA == nodoB) {
        cout << "[Error] Los dos nodos deben ser distintos.\n";
        return false;
    }
    if (red.enrutadores.find(nodoA) == red.enrutadores.end()) {
        cout << "[Error] El nodo '" << nodoA << "' no existe en la red.\n";
        return false;
    }
    if (red.enrutadores.find(nodoB) == red.enrutadores.end()) {
        cout << "[Error] El nodo '" << nodoB << "' no existe en la red.\n";
        return false;
    }
    return true;
}

// ─── Main ─────────────────────────────────────────────────────────────────────

int main() {
    Red red;

    // ── Menú inicial ──────────────────────────────────────────────────────────
    int opcionInicial = leerOpcion(
        "\nModelar una red manualmente (1), generar aleatoriamente (2), "
        "cargar por archivo (3): ", 1, 3);

    if (opcionInicial == 2) {
        int n = leerEnteroPositivo("Cantidad de nodos a generar: ");
        if (n > 26) {
            cout << "[Aviso] Se limita a 26 nodos (una letra por nodo). "
                    "Se usarán 26.\n";
            n = 26;
        }
        red.generarRedAleatoria(n);
        cout << "[OK] Red aleatoria de " << n << " nodos generada.\n";

    } else if (opcionInicial == 3) {
        cout << "Ingrese nombre del archivo (ej. topologia.txt): ";
        string archivo;
        cin >> archivo;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (archivo.empty()) {
            cout << "[Error] Nombre de archivo vacio. Se usara 'topologia.txt'.\n";
            archivo = "topologia.txt";
        }
        red.cargarDesdeArchivo(archivo);

    } else {
        // Modo manual
        int nodos = leerEnteroPositivo("Ingrese cantidad de nodos: ");
        if (nodos > 26) {
            cout << "[Aviso] Maximo 26 nodos. Se usaran 26.\n";
            nodos = 26;
        }

        for (int i = 0; i < nodos; i++) {
            string nodoStr;
            while (true) {
                nodoStr = leerNodoLetra("Ingrese letra para el nodo " +
                                        to_string(i + 1) + ": ");
                if (red.enrutadores.find(nodoStr) == red.enrutadores.end()) break;
                cout << "[Error] El nodo '" << nodoStr
                     << "' ya existe. Elija otra letra.\n";
            }
            red.agregarEnrutador(nodoStr);
            cout << "[OK] Nodo '" << nodoStr << "' agregado.\n";
        }

        // Agregar conexiones iniciales
        cout << "\nAhora agregue conexiones (enlaces) entre nodos.\n";
        while (true) {
            string nodoStr, nodoVecinoStr;
            if (!pedirDosNodosExistentes(red,
                                         "Nodo origen: ", "Nodo destino: ",
                                         nodoStr, nodoVecinoStr)) {
                continue;
            }
            int coste = leerEnteroPositivo("Costo del enlace: ");
            red.actualizarEnlace(nodoStr, nodoVecinoStr, coste);
            cout << "[OK] Enlace " << nodoStr << " <-> "
                 << nodoVecinoStr << " con costo " << coste << " agregado.\n";

            int seguir = leerOpcion("¿Agregar otra conexion? Sí (1) / No (2): ", 1, 2);
            if (seguir == 2) break;
        }
    }

    // ── Menú principal ────────────────────────────────────────────────────────
    int opcion = 0;
    while (opcion != 7) {
        cout << "\n--- MENU PRINCIPAL ---\n"
             << "  1) Agregar nodo\n"
             << "  2) Eliminar nodo\n"
             << "  3) Calcular costo/ruta\n"
             << "  4) Cambiar costo de enlace\n"
             << "  5) Eliminar conexion\n"
             << "  6) Mostrar tabla de vecinos\n"
             << "  7) Salir\n";

        opcion = leerOpcion("Seleccione una opcion: ", 1, 7);

        // 1 ── Agregar nodo ────────────────────────────────────────────────────
        if (opcion == 1) {
            string nodoStr;
            while (true) {
                nodoStr = leerNodoLetra("Nombre del nuevo nodo: ");
                if (red.enrutadores.find(nodoStr) == red.enrutadores.end()) break;
                cout << "[Error] El nodo '" << nodoStr << "' ya existe.\n";
            }
            red.agregarEnrutador(nodoStr);
            cout << "[OK] Nodo '" << nodoStr << "' agregado.\n";

            // Pregunta si conectar a un vecino de inmediato
            if (!red.enrutadores.empty()) {
                int conectar = leerOpcion(
                    "¿Conectar a un vecino ahora? Sí (1) / No (2): ", 1, 2);
                if (conectar == 1) {
                    string nodoVecinoStr;
                    if (pedirDosNodosExistentes(red,
                                                "Nodo origen (recien creado u otro): ",
                                                "Nodo vecino: ",
                                                nodoStr, nodoVecinoStr)) {
                        int coste = leerEnteroPositivo("Costo del enlace: ");
                        red.actualizarEnlace(nodoStr, nodoVecinoStr, coste);
                        cout << "[OK] Enlace agregado.\n";
                    }
                }
            }
        }

        // 2 ── Eliminar nodo ───────────────────────────────────────────────────
        else if (opcion == 2) {
            if (red.enrutadores.empty()) {
                cout << "[Aviso] La red esta vacia, no hay nodos que eliminar.\n";
                continue;
            }
            string nodoStr = leerNodoLetra("Nombre del nodo a eliminar: ");
            if (red.enrutadores.find(nodoStr) == red.enrutadores.end()) {
                cout << "[Error] El nodo '" << nodoStr << "' no existe.\n";
            } else {
                red.eliminarEnrutador(nodoStr);
                cout << "[OK] Nodo '" << nodoStr << "' eliminado.\n";
            }
        }

        // 3 ── Calcular costo / ruta ───────────────────────────────────────────
        else if (opcion == 3) {
            if (red.enrutadores.size() < 2) {
                cout << "[Aviso] Se necesitan al menos 2 nodos en la red.\n";
                continue;
            }
            string nodoStr1, nodoStr2;
            if (!pedirDosNodosExistentes(red,
                                         "Nodo inicio: ", "Nodo destino: ",
                                         nodoStr1, nodoStr2)) continue;

            int costo = red.calcularCosto(nodoStr1, nodoStr2);

            if (costo == numeric_limits<int>::max() || costo < 0) {
                cout << "[Info] No existe ruta posible entre '"
                     << nodoStr1 << "' y '" << nodoStr2 << "'.\n";
            } else {
                cout << "Costo de " << nodoStr1 << " a "
                     << nodoStr2 << ": " << costo << "\n";
                auto ruta = red.obtenerRuta(nodoStr1, nodoStr2);
                cout << "Ruta: ";
                for (size_t i = 0; i < ruta.size(); ++i) {
                    cout << ruta[i];
                    if (i + 1 < ruta.size()) cout << " -> ";
                }
                cout << "\n";
            }
        }

        // 4 ── Cambiar costo de enlace ─────────────────────────────────────────
        else if (opcion == 4) {
            if (red.enrutadores.size() < 2) {
                cout << "[Aviso] Se necesitan al menos 2 nodos en la red.\n";
                continue;
            }
            string nodoStr, nodoVecinoStr;
            if (!pedirDosNodosExistentes(red,
                                         "Nodo principal: ", "Nodo vecino: ",
                                         nodoStr, nodoVecinoStr)) continue;

            // Verificar que la conexión existe
            Enrutador* e = red.enrutadores.at(nodoStr);
            Enrutador* v = red.enrutadores.at(nodoVecinoStr);
            if (e->vecinos.find(v) == e->vecinos.end()) {
                cout << "[Error] No existe un enlace entre '"
                     << nodoStr << "' y '" << nodoVecinoStr << "'.\n";
                continue;
            }

            int coste = leerEnteroPositivo("Nuevo costo del enlace: ");
            red.actualizarEnlace(nodoStr, nodoVecinoStr, coste);
            cout << "[OK] Costo actualizado a " << coste << ".\n";
        }

        // 5 ── Eliminar conexión ───────────────────────────────────────────────
        else if (opcion == 5) {
            if (red.enrutadores.size() < 2) {
                cout << "[Aviso] Se necesitan al menos 2 nodos en la red.\n";
                continue;
            }
            string nodoStr, nodoVecinoStr;
            if (!pedirDosNodosExistentes(red,
                                         "Nodo principal: ", "Nodo vecino: ",
                                         nodoStr, nodoVecinoStr)) continue;

            Enrutador* e = red.enrutadores.at(nodoStr);
            Enrutador* v = red.enrutadores.at(nodoVecinoStr);
            if (e->vecinos.find(v) == e->vecinos.end()) {
                cout << "[Error] No existe un enlace entre '"
                     << nodoStr << "' y '" << nodoVecinoStr << "'.\n";
                continue;
            }

            red.eliminarConexion(nodoStr, nodoVecinoStr);
            cout << "[OK] Conexion eliminada.\n";
        }

        // 6 ── Mostrar tabla de vecinos ────────────────────────────────────────
        else if (opcion == 6) {
            if (red.enrutadores.empty()) {
                cout << "[Aviso] La red esta vacia.\n";
            } else {
                red.imprimirTablaVecinos();
            }
        }

        // 7 ── Salir ───────────────────────────────────────────────────────────
        else if (opcion == 7) {
            cout << "Saliendo...\n";
            break;
        }
    }

    return 0;
}