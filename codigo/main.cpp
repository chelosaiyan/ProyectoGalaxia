
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <climits>
#include <curl/curl.h>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;


// Estructura: Arco
// Representa una ruta entre dos galaxias y es un nodo de la sublista de arcos de cada vertice.
struct Arco {
    string id;
    string destino_id;
    string tipo; // "subespacio","convencional", "hiperespacio"
    float costo;
    float tiempo_dias;
    bool activa;
    Arco* sigA;  
};

// Estructura: Vertice
// Representa una galaxia dentro del grafo y tiene una sublista de arcos que son las rutas que salen de ella.
struct Vertice {
    string id;
    string codigo;
    string nombre;
    string  tipo;  // "eliptica", "espiral"
    float x, y, z;
    string descripcion;
    Arco* subListaArcos; 
    Vertice* sigV;         
};

// Estructura: Nave
// Representa una nave registrada en el sistema.
struct Nave {
    string id;
    string codigo;
    string nombre;
    int capacidad;
    int velocidad_max;
    bool activa;
    Nave*sigN;        
};

// Estructura: Viaje
//Representa un viaje en el historial de una nave.
struct Viaje {
    string id;
    string nave_id;
    string origen_id;
    string destino_id;
    string fecha;
    Viaje* sigVj;       
};

// Estructura: AristaKruskal
//Estructura auxiliar para el algoritmo de Kruskal y guarda el origen, destino y costo de una arista.
struct AristaKruskal {
    string origen_id;
    string destino_id;
    float costo;
//Constructor para inicializar los campos de la arista.
    AristaKruskal(string o, string d, float c) : origen_id(o), destino_id(d), costo(c) {}
};

// Estructura: EncontrarUnion
//Estructura usada por Kruskal y permite unir conjuntos y encontrar la raiz de cada uno.
struct EncontrarUnion {
    unordered_map<string, string> padre;
    unordered_map<string, int>    rango;

    void hacerConjunto(const string& v) {
        padre[v] = v;
        rango[v] = 0;
    }

    string encontrar(const string& v) {
        if (padre[v] != v)
            padre[v] = encontrar(padre[v]); 
        return padre[v];
    }

    void unir(const string& a, const string& b) {
        string raizA = encontrar(a);
        string raizB = encontrar(b);
        if (raizA == raizB) return;
        if (rango[raizA] < rango[raizB]) {
            padre[raizA] = raizB;
        } else {
            padre[raizB] = raizA;
            if (rango[raizA] == rango[raizB])
                rango[raizA]++;
        }
    }
};

//Variables globales.
Vertice* grafo = nullptr; // lista enlazada de galaxias (vertices del grafo)
Nave* naves = nullptr; // lista enlazada de naves
Viaje* viajes = nullptr; // lista enlazada de viajes


//Menu
int main() {
    int opcion;

    do {
        cout << "\n===== GUARDIANES DE LA GALAXIA =====" << endl;
        cout << "1. Galaxias" << endl;
        cout << "2. Naves" << endl;
        cout << "3. Rutas" << endl;
        cout << "4. Historial de viajes" << endl;
        cout << "5. Consultas" << endl;
        cout << "6. Reportes" << endl;
        cout << "0. Salir" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch(opcion) {
            case 1: cout << "Modulo Galaxias" << endl; break;
            case 2: cout << "Modulo Naves" << endl; break;
            case 3: cout << "Modulo Rutas" << endl; break;
            case 4: cout << "Modulo Historial" << endl; break;
            case 5: cout << "Modulo Consultas" << endl; break;
            case 6: cout << "Modulo Reportes" << endl; break;
            case 0: cout << "Hasta luego!" << endl; break;
            default: cout << "Opcion invalida" << endl;
        }

    } while(opcion != 0);

    return 0;
}