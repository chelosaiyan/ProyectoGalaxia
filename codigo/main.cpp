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
    string tipo; 
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
    string tipo;  
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
    unordered_map<string, int> rango;

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

//API y JSON

// Funcion: escribirRespuesta
// Callback interno de curl. Cada vez que llegan datos del servidor, esta funcion los acumula en el string "salida". 

size_t escribirRespuesta(void* contenido, size_t tam, size_t n, string* salida) {
    salida->append((char*)contenido, tam * n);
    return tam * n;
}
 
// Funcion: consultarAPI
// Hace una peticion GET a la URL indicada y retorna toda la respuesta como string de texto.

string consultarAPI(const string& url) {
    CURL* curl = curl_easy_init();
    string respuesta = "";
 
    if (!curl) {
        cout << "Error: No se pudo inicializar curl." << endl;
        return "";
    }
 
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, escribirRespuesta);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respuesta);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
 
    CURLcode resultado = curl_easy_perform(curl);
 
    if (resultado != CURLE_OK) {
        cout << "Error curl: " << curl_easy_strerror(resultado) << endl;
        respuesta = "";
    }
 
    curl_easy_cleanup(curl);
    return respuesta;
}
 

// Funcion: buscarVertice
// Busca una galaxia en el grafo por su id. Retorna nullptr si no se encuentra.
Vertice* buscarVertice(const string& id) {
    Vertice* actual = grafo;
    while (actual != nullptr) {
        if (actual->id == id) return actual;
        actual = actual->sigV;
    }
    return nullptr;
}
 
// Funcion: buscarVerticePorCodigo
// Busca una galaxia en el grafo por su codigo. Retorna nullptr si no se encuentra.
Vertice* buscarVerticePorCodigo(const string& codigo) {
    Vertice* actual = grafo;
    while (actual != nullptr) {
        if (actual->codigo == codigo) return actual;
        actual = actual->sigV;
    }
    return nullptr;
}
 
// Funcion: insertarVertice
// Crea una nueva galaxia y la agrega al inicio de la lista enlazada del grafo.
// Valida que no exista ya otra galaxia con el mismo id o el mismo codigo.

void insertarVertice(const string& id, const string& codigo, const string& nombre, const string& tipo, float x, float y, float z, const string& descripcion) {
    if (buscarVertice(id) != nullptr) {
        cout << "La galaxia con id '" << id << "' ya estaba registrada. No se duplico." << endl;
        return;
    }
    if (buscarVerticePorCodigo(codigo) != nullptr) {
        cout << "La galaxia con codigo '" << codigo << "' ya estaba registrada. No se duplico." << endl;
        return;
    }
 
    Vertice* nuevo = new Vertice;
    nuevo->id = id;
    nuevo->codigo = codigo;
    nuevo->nombre = nombre;
    nuevo->tipo = tipo;
    nuevo->x = x;
    nuevo->y = y;
    nuevo->z = z;
    nuevo->descripcion = descripcion;
    nuevo->subListaArcos = nullptr;
    nuevo->sigV = grafo;
    grafo = nuevo;
}
// Funcion: cargarGalaxias
// Consulta el endpoint /galaxias, parsea el JSON y llama a insertarVertice por cada galaxia.

void cargarGalaxias() {
    cout << "Cargando galaxias." << endl;
    string texto = consultarAPI("https://galaxias-mock-api.onrender.com/galaxias");
    if (texto.empty()) {
        cout << "Error: No se recibieron galaxias." << endl;
        return;
    }
 
    json datos = json::parse(texto, nullptr, false);
    if (datos.is_discarded()) {
        cout << "Error: El JSON de galaxias no es valido." << endl;
        return;
    }
 
    for (auto& item : datos) {
        insertarVertice(item["id"], item["codigo"], item["nombre"], item["tipo"], item["x"], item["y"], item["z"], item["descripcion"] );
    }
    cout << "Galaxias cargadas correctamente." << endl;
}

 

// Funcion: insertarArco
// Busca la galaxia origen y le agrega un nuevo arco (ruta) al inicio de su sublista de arcos.
// Valida que no exista ya una ruta con el mismo id en esa sublista.

void insertarArco(const string& origen_id, const string& id, const string& destino_id, const string& tipo, float costo, float tiempo_dias, bool activa) {
    Vertice* origen = buscarVertice(origen_id);
    if (origen == nullptr) {
        cout << "Error: no existe la galaxia origen con id '" << origen_id << "'. No se pudo registrar la ruta '" << id << endl;
        return;
    }
 
    Arco* recorre = origen->subListaArcos;
    while (recorre != nullptr) {
        if (recorre->id == id) {
            cout << "La ruta con id '" << id << "' ya estaba registrada. No se duplico." << endl;
            return;
        }
        recorre = recorre->sigA;
    }
 
    if (buscarVertice(destino_id) == nullptr) {
        cout << "La ruta '" << id << "' apunta a un destino ('" << destino_id << "') que no existe en el grafo." << endl;
    }
 
    Arco* nuevo = new Arco;
    nuevo->id = id;
    nuevo->destino_id = destino_id;
    nuevo->tipo = tipo;
    nuevo->costo = costo;
    nuevo->tiempo_dias = tiempo_dias;
    nuevo->activa = activa;
    nuevo->sigA = origen->subListaArcos;
    origen->subListaArcos = nuevo;
}


// Funcion: cargarRutas
// Consulta el endpoint /rutas, parsea el JSON y llama a insertarArco por cada ruta.
void cargarRutas() {
    cout << "Cargando rutas." << endl;
    string texto = consultarAPI("https://galaxias-mock-api.onrender.com/rutas");
    if (texto.empty()) {
        cout << "Error: No se recibieron rutas." << endl;
        return;
    }
 
    json datos = json::parse(texto, nullptr, false);
    if (datos.is_discarded()) {
        cout << "Error: El JSON de rutas no es valido." << endl;
        return;
    }
 
    for (auto& item : datos) {
        insertarArco(item["origen_id"], item["id"], item["destino_id"], item["tipo"], item["costo"], item["tiempo_dias"], item["activa"] );
    }
    cout << "Rutas cargadas correctamente." << endl;
}
 


// Funcion: buscarNave
// Busca una nave en la lista por su id. Retorna nullptr si no se encuentra.
Nave* buscarNave(const string& id) {
    Nave* actual = naves;
    while (actual != nullptr) {
        if (actual->id == id) return actual;
        actual = actual->sigN;
    }
    return nullptr;
}
 
// Funcion: buscarNavePorCodigo
// Busca una nave en la lista por su codigo. Retorna nullptr si no se encuentra.
Nave* buscarNavePorCodigo(const string& codigo) {
    Nave* actual = naves;
    while (actual != nullptr) {
        if (actual->codigo == codigo) return actual;
        actual = actual->sigN;
    }
    return nullptr;
}
 
// Funcion: insertarNave
// Crea una nueva nave y la agrega al inicio de la lista enlazada de naves.
// Valida que no exista ya otra nave con el mismo id o el mismo codigo.
void insertarNave(const string& id, const string& codigo, const string& nombre, int capacidad, int velocidad_max, bool activa) {
    if (buscarNave(id) != nullptr) {
        cout << "La nave con id '" << id << "' ya estaba registrada. No se duplico." << endl;
        return;
    }
    if (buscarNavePorCodigo(codigo) != nullptr) {
        cout << "La nave con codigo '" << codigo << "' ya estaba registrada. No se duplico." << endl;
        return;
    }
 
    Nave* nueva = new Nave;
    nueva->id = id;
    nueva->codigo = codigo;
    nueva->nombre = nombre;
    nueva->capacidad = capacidad;
    nueva->velocidad_max = velocidad_max;
    nueva->activa = activa;
    nueva->sigN = naves;
    naves = nueva;
}

// Funcion: cargarNaves
// Consulta el endpoint /naves, parsea el JSON y llama a insertarNave por cada nave. 

void cargarNaves() {
    cout << "Cargando naves" << endl;
    string texto = consultarAPI("https://galaxias-mock-api.onrender.com/naves");
    if (texto.empty()) {
        cout << "Error: No se recibieron naves." << endl;
        return;
    }
 
    json datos = json::parse(texto, nullptr, false);
    if (datos.is_discarded()) {
        cout << "Error: El JSON de naves no es valido." << endl;
        return;
    }
 
    for (auto& item : datos) {
        insertarNave( item["id"], item["codigo"], item["nombre"], item["capacidad"], item["velocidad_max"], item["activa"] );
    }
    cout << "Naves cargadas correctamente." << endl;
}
 

// Funcion: buscarViaje
// Busca un viaje en el historial por su id. Retorna nullptr si no se encuentra.

Viaje* buscarViaje(const string& id) {
    Viaje* actual = viajes;
    while (actual != nullptr) {
        if (actual->id == id) return actual;
        actual = actual->sigVj;
    }
    return nullptr;
}
 
// Funcion: insertarViaje
// Crea un nuevo viaje y lo agrega al inicio de la lista enlazada del historial.
// Valida que no exista ya un viaje con el mismo id.
void insertarViaje(const string& id, const string& nave_id, const string& origen_id, const string& destino_id, const string& fecha) {
    if (buscarViaje(id) != nullptr) {
        cout << "El viaje con id '" << id << "' ya estaba registrado. No se duplico." << endl;
        return;
    }
    if (buscarNave(nave_id) == nullptr) {
        cout << "El viaje '" << id << "' referencia una nave ('" << nave_id << "') que no existe." << endl;
    }
    if (buscarVertice(origen_id) == nullptr || buscarVertice(destino_id) == nullptr) {
        cout << "El viaje '" << id << "' referencia una galaxia de origen o destino que no existe." << endl;
    }
 
    Viaje* nuevo = new Viaje;
    nuevo->id = id;
    nuevo->nave_id = nave_id;
    nuevo->origen_id = origen_id;
    nuevo->destino_id = destino_id;
    nuevo->fecha = fecha;
    nuevo->sigVj = viajes;
    viajes = nuevo;
}

// Funcion: cargarViajes
// Descripcion: Consulta el endpoint /historial, parsea el JSON y llama a insertarViaje por cada viaje.

void cargarViajes() {
    cout << "Cargando historial de viajes" << endl;
    string texto = consultarAPI("https://galaxias-mock-api.onrender.com/historial");
    if (texto.empty()) {
        cout << "Error: No se recibio historial." << endl;
        return;
    }
 
    json datos = json::parse(texto, nullptr, false);
    if (datos.is_discarded()) {
        cout << "Error: El JSON del historial no es valido." << endl;
        return;
    }
 
    for (auto& item : datos) {
        string idViaje = item["id"].is_null() ? "" : string(item["id"]);
        string naveId = item["nave_id"].is_null() ? "" : string(item["nave_id"]);
        string origenId = item["origen_id"].is_null() ? "" : string(item["origen_id"]);
        string destinoId = item["destino_id"].is_null() ? "" : string(item["destino_id"]);
        string fecha = item["fecha"].is_null() ? "" : string(item["fecha"]);
        insertarViaje(idViaje, naveId, origenId, destinoId, fecha);
    }
    cout << "Historial cargado correctamente" << endl;
}
 

// Funcion: cargarDatosIniciales
// Llama a todas las funciones de carga en el orden correcto. Primero galaxias, luego rutas, luego naves y viajes.
void cargarDatosIniciales() {
    cout << " Cargando datos del universo " << endl;
    cargarGalaxias();
    cargarRutas();
    cargarNaves();
    cargarViajes();
    cout << " Datos cargados exitosamente" << endl;
}

//Representacion del Grafo

// Funcion: mostrarGrafo
// Recorre la lista enlazada de galaxias (grafo) y para cada una recorre su
// sublista de arcos (subListaArcos) mostrando las rutas que salen de ella.
// Sirve para comprobar que la lista de adyacencia quedo bien construida.

void mostrarGrafo() {
    if (grafo == nullptr) {
        cout << "El grafo esta vacio. No hay galaxias cargadas." << endl;
        return;
    }

    Vertice* actual = grafo;
    while (actual != nullptr) {
        cout << "\nGalaxia: " << actual->nombre << " (codigo: " << actual->codigo << ", id: " << actual->id << ")" << endl;
        cout << " Tipo: " << actual->tipo << " Coordenadas: (" << actual->x << ", " << actual->y << ", " << actual->z << ")" << endl;

        if (actual->subListaArcos == nullptr) {
            cout << " No tiene rutas salientes registradas." << endl;
        } else {
            cout << " Rutas salientes:" << endl;
            Arco* arco = actual->subListaArcos;
            while (arco != nullptr) {
                Vertice* destino = buscarVertice(arco->destino_id);
                string nombreDestino = (destino != nullptr) ? destino->nombre : "desconocido";

                cout << " -> " << nombreDestino << " (" << arco->destino_id << ")"
                     << " tipo: " << arco->tipo
                     << " costo: " << arco->costo
                     << " tiempo: " << arco->tiempo_dias << " dias"
                     << " activa: " << (arco->activa ? "si" : "no") << endl;
                arco = arco->sigA;
            }
        }
        actual = actual->sigV;

    }
}

//Kruskal 


// Funcion: kruskal
// Consulta el endpoint /grafo/kruskal que ya trae las aristas en orden aleatorio y construye el arbol de expansion.

void kruskal() {
    if (grafo == nullptr) {
        cout << "No hay galaxias cargadas. Carga el grafo primero." << endl;
        return;
    }

    string texto = consultarAPI("https://galaxias-mock-api.onrender.com/grafo/kruskal");
    if (texto.empty()) {
        cout << "Error: No se recibieron aristas para Kruskal." << endl;
        return;
    }

    json datos = json::parse(texto, nullptr, false);
    if (datos.is_discarded()) {
        cout << "Error: El JSON de Kruskal no es valido." << endl;
        return;
    }

    // Paso 1: Cargar aristas en vector 

    vector<AristaKruskal> aristas;
    for (auto& item : datos["aristas"]) {
        aristas.emplace_back(string(item["origen_id"]), string(item["destino_id"]), float(item["costo"]));
    }

    // Paso 2: Crear conjuntos, uno por galaxia

    EncontrarUnion eu;
    for (Vertice* v = grafo; v != nullptr; v = v->sigV) {
        eu.hacerConjunto(v->id);
    }

    // Paso 3: Procesar aristas en el orden aleatorio que vienen

    vector<AristaKruskal> mst;
    float pesoTotal = 0;
    for (int i = 0; i < aristas.size(); i++) {
        string raizOrigen  = eu.encontrar(aristas[i].origen_id);
        string raizDestino = eu.encontrar(aristas[i].destino_id);

        if (raizOrigen != raizDestino) {
            mst.push_back(aristas[i]);
            pesoTotal += aristas[i].costo;
            eu.unir(aristas[i].origen_id, aristas[i].destino_id);
        }
    }

    // Paso 4: Mostrar resultados

    cout << "\nArbol de expansion:" << endl;
    for (int i = 0; i < mst.size(); i++) {
        Vertice* origen = buscarVertice(mst[i].origen_id);
        Vertice* destino = buscarVertice(mst[i].destino_id);
        string nOrigen = (origen != nullptr) ? origen->nombre : mst[i].origen_id;
        string nDestino = (destino != nullptr) ? destino->nombre : mst[i].destino_id;
        cout << nOrigen << nDestino << ": " << mst[i].costo << endl;
    }
    cout << "Costo total del arbol: " << pesoTotal << endl;
}
//Archivos

// Funcion: crearArchivo
// Crea un archivo de texto vacio con el nombre indicado.

void crearArchivo(string nombreArchivo) {
    ofstream archivo(nombreArchivo);
    if (archivo.is_open()) {
        cout << "Archivo '" << nombreArchivo << "' creado exitosamente." << endl;
        archivo.close();
    } else {
        cout << "Error al crear el archivo" << endl;
    }
}

// Funcion: escribirEnArchivo
// Agrega una linea de texto al final del archivo indicado.

void escribirEnArchivo(string nombreArchivo, string datos) {
    ofstream archivo(nombreArchivo, ios::app);
    if (archivo.is_open()) {
        archivo << datos << endl;
        archivo.close();
    } else {
        cout << "Error al abrir el archivo para escritura." << endl;
    }
}

// Funcion: leerArchivo
// Lee y muestra en pantalla el contenido de un archivo de texto.

void leerArchivo(string nombreArchivo) {
    ifstream archivo(nombreArchivo);
    if (archivo.is_open()) {
        string linea;
        cout << "Contenido de '" << nombreArchivo << "':" << endl;
        while (getline(archivo, linea)) {
            cout << linea << endl;
        }
        archivo.close();
    } else {
        cout << "Error al abrir el archivo para lectura." << endl;
    }
}


//Menu

// Funcion: menuGalaxias
// Submenu para ver galaxias y el grafo completo.

void menuGalaxias() {
    int opcion;
    do {
        cout << "\nGalaxias" << endl;
        cout << "1. Ver todas las galaxias" << endl;
        cout << "2. Ver grafo completo" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch(opcion) {
            case 1: mostrarGrafo(); break;
            case 2: mostrarGrafo(); break;
            case 0: break;
            default: cout << "Opcion invalida" << endl;
        }
    } while(opcion != 0);
}

// Funcion: menuNaves
// Submenu para ver naves registradas.

void menuNaves() {
    int opcion;
    do {
        cout << "\nNaves" << endl;
        cout << "1. Ver todas las naves" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch(opcion) {
            case 1: {
                if (naves == nullptr) {
                    cout << "No hay naves registradas." << endl;
                } else {
                    Nave* actual = naves;
                    cout << "\n Naves registradas" << endl;
                    while (actual != nullptr) {
                        cout << "ID: " << actual->id
                             << " Codigo: " << actual->codigo
                             << " Nombre: " << actual->nombre
                             << " Capacidad: " << actual->capacidad
                             << " Vel Max: " << actual->velocidad_max
                             << " Activa: " << (actual->activa ? "si" : "no") << endl;
                        actual = actual->sigN;
                    }
                }
                break;
            }
            case 0: break;
            default: cout << "Opcion invalida" << endl;
        }
    } while(opcion != 0);
}

// Funcion: menuRutas
// Submenu para ver rutas desde una galaxia especifica.

void menuRutas() {
    int opcion;
    do {
        cout << "\nRutas" << endl;
        cout << "1. Ver rutas desde una galaxia" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch(opcion) {
            case 1: {
                string id;
                cout << "Ingrese el ID de la galaxia (ej: galaxia-1): ";
                cin >> id;
                Vertice* origen = buscarVertice(id);
                if (origen == nullptr) {
                    cout << "Galaxia no encontrada." << endl;
                } else if (origen->subListaArcos == nullptr) {
                    cout << "La galaxia " << origen->nombre << " no tiene rutas registradas." << endl;
                } else {
                    cout << "\n Rutas desde: " << origen->nombre << endl;
                    Arco* arco = origen->subListaArcos;
                    while (arco != nullptr) {
                        Vertice* destino = buscarVertice(arco->destino_id);
                        string nombreDestino = (destino != nullptr) ? destino->nombre : arco->destino_id;
                        cout << "-> " << nombreDestino
                             << " tipo: " << arco->tipo
                             << " costo: " << arco->costo
                             << " dias: " << arco->tiempo_dias
                             << " activa: " << (arco->activa ? "si" : "no") << endl;
                        arco = arco->sigA;
                    }
                }
                break;
            }
            case 0: break;
            default: cout << "Opcion invalida" << endl;
        }
    } while(opcion != 0);
}

// Funcion: menuHistorial
// Submenu para consultar el historial de viajes de una nave.

void menuHistorial() {
    int opcion;
    do {
        cout << "\n Historial de Viajes" << endl;
        cout << "1. Ver historial de una nave" << endl;
        cout << "0. Volver" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch(opcion) {
            case 1: {
                string id;
                cout << "Ingrese el ID de la nave (ej: nave-1): ";
                cin >> id;
                Nave* nave = buscarNave(id);
                if (nave == nullptr) {
                    cout << "Nave no encontrada." << endl;
                } else {
                    cout << "\nHistorial de: " << nave->nombre << endl;
                    bool encontrado = false;
                    Viaje* actual = viajes;
                    while (actual != nullptr) {
                        if (actual->nave_id == id) {
                            Vertice* origen = buscarVertice(actual->origen_id);
                            Vertice* destino = buscarVertice(actual->destino_id);
                            string nOrigen = (origen != nullptr) ? origen->nombre : actual->origen_id;
                            string nDestino = (destino != nullptr) ? destino->nombre : actual->destino_id;
                            cout << "Viaje: " << actual->id
                                 << " De: " << nOrigen
                                 << " -> " << nDestino
                                 << " Fecha: " << actual->fecha << endl;
                            encontrado = true;
                        }
                        actual = actual->sigVj;
                    }
                    if (!encontrado) cout << "Esta nave no tiene viajes registrados" << endl;
                }
                break;
            }
            case 0: break;
            default: cout << "Opcion invalida" << endl;
        }
    } while(opcion != 0);
}




int main() {
    int opcion;
    cargarDatosIniciales();
    do {
        cout << "\nGestion del Sistema" << endl;
        cout << "1. Galaxias" << endl;
        cout << "2. Naves" << endl;
        cout << "3. Rutas" << endl;
        cout << "4. Historial de viajes" << endl;
        cout << "0. Salir" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch(opcion) {
            case 1: menuGalaxias(); break;
            case 2: menuNaves(); break;
            case 3: menuRutas(); break;
            case 4: menuHistorial(); break;

            case 0: cout << "Hasta pronto" << endl; break;
            default: cout << "Opcion invalida" << endl;
        }
    } while(opcion != 0);

    return 0;
}