#include <iostream>
#include <string>
using namespace std;

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