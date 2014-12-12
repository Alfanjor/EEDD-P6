/* 
 * File:   main.cpp
 * Author: zackreynolt
 *
 * Created on 21 de noviembre de 2014, 17:44
 */

#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <cctype>
#include <utility>
#include <map>
#include <unordered_map>

#include "tinythread.h"
#include "millisleep.h"
#include "Song.h"
#include "Request.h"
#include "ItemCancion.h"
#include "TableHash.h"

using namespace std;
using namespace tthread;

void CargarListaCaciones(map<int, Song> &vCanciones2);
long djb2 (char *str);

class RadioApp {
    map<int, Song> vCanciones2;
    
    vector<Request>  vPeticiones;
    vector<Song>     vCanciones;

    /* Tamaño de tablas según factor de carga:
        70%
        El primo más cercano a 410*1.7=697 es: 691
        El primo más cercano a 821*1.7=1395.7 es: 1381
        75%
        El primo más cercano a 410*1.75=717.5 es: 709
        El primo más cercano a 821*1.75=1436.75 es: 1433
        80%
        El primo más cercano a 410*1.80=738 es: 733
        El primo más cercano a 821*1.80=1477.8 es: 1471
        85%
        El primo más cercano a 410*1.85=758.5 es: 757
        El primo más cercano a 821*1.85=1518.85 es: 1511
        90%
        El primo más cercano a 410*1.9=779 es: 773
        El primo más cercano a 821*1.9=1559.9 es: 1553
     */
    unordered_map<long, ItemCancion> tablaAutores2;
    unordered_map<long, ItemCancion> tablaTitulos2;
    TableHash<ItemCancion, 691>  tablaAutores;
    TableHash<ItemCancion, 1381> tablaTitulos;
   
    thread  threadReproducirCanciones;
    mutex   semaforo;
    bool    pinchar;

    static void hebraReproducirCanciones(void *arg) {
        RadioApp *radioApp = static_cast<RadioApp *> (arg);
        radioApp->reproducirCanciones();
    }

public:

    RadioApp() : threadReproducirCanciones(hebraReproducirCanciones, this) {
        pinchar = true;
        CargarListaCaciones(vCanciones2);
        
        tablaAutores2.reserve(691);
        
        
        int pArtista = 0, pTitulo = 0;
        
        // Construcción de las tablas de dispersión
        for (map<int, Song>::iterator it = vCanciones2.begin(); it != vCanciones2.end(); ++it) {
            
            string artista, titulo;
            string lineArtist = it->second.GetArtist();
            string lineTittle = it->second.GetTitle();
            stringstream lineStreamArtist(lineArtist);
            stringstream lineStreamTittle(lineTittle);
            
//            while (getline(lineStreamArtist, artista, ' ')) {
//                for (int x = 0; x < artista.size(); x++)
//                    artista[x] = tolower(artista[x]);
//                
//                long key = djb2((char*) artista.c_str());
//                unordered_map<int, Song>::iterator it_aux = tablaAutores2.find(key);
//                if (!it_aux) {
//                    tablaAutores2.insert()
//                } else
//                    p->addSong(&vCanciones[i]);
//            };
//            
//            while (getline(lineStreamTittle, titulo, ' ')) {
//                for (int x = 0; x < titulo.size(); x++)
//                    titulo[x] = tolower(titulo[x]);
//                
//                long key = djb2((char*) titulo.c_str());
//                ItemCancion *p = tablaTitulos.search(key);
//                if (!p) {
//                    if(!tablaTitulos.insert(key, ItemCancion(titulo, &vCanciones[i])))
//                        cout << "Demasiadas colisiones para pTitulo: " << titulo << " en la canción " << i << endl;
//                    pTitulo++;
//                } else
//                    p->addSong(&vCanciones[i]);
//            };
        };
    };

    void reproducirCanciones() {
        do {
            // Esperar 1 seg. hasta ver si hay alguna canción nueva en la lista de peticiones
            millisleep(1000); 
            
            while (pinchar && !vPeticiones.empty()) {
                // Sacar una canción y reproducirla
                semaforo.lock();
                // Coge la que tenga más prioridad (última)
                int cancion = vPeticiones.back().getCod();
                vPeticiones.pop_back();
                semaforo.unlock();
                
                cout << "Reproduciendo canción " << 
                        vCanciones[cancion - 1].GetTitle() << 
                        " de " << vCanciones[cancion - 1].GetArtist() <<
                        "... (" << vCanciones[cancion - 1].GetCode() << ")" << endl;
                // Simular el tiempo de reproducción de la canción (entre 2 y 12 seg.)
                
                millisleep(2000 + 1000 * (rand() % 10));
            }
            
        } while (pinchar);
    }

    void solicitarCanciones() {
        int cancion; //Código de la canción que se añadirá a vPeticiones
        
        cout << "¡Bienvenido a Radionauta v4!" << endl;
        cout << "Introduce el código de la canción que quieres reproducir:" 
                << endl;        
        
        // Pedir canciones hasta que se introduce "0"
        do {
            cin >> cancion;

            //Asegurarse de que la canción introducida es válida
            while (cancion < 0 || cancion > vCanciones.size()) {
                cin.clear();
                cin.ignore(100, '\n');
                cout << "Lo sentimos, no disponemos de esa canción." << endl;
                cin >> cancion;
            }
            
            semaforo.lock();
            //Código fuente para añadir canciones a vPeticiones
            Request peticion(cancion); 
            vPeticiones.push_back(peticion);            
            semaforo.unlock();

        } while (cancion != 0);
        
        pinchar = false;
        threadReproducirCanciones.join();
    }
};

/**
 * 
 * @param lSongs    Lista con las canciones (por referencia)
 * @return          void
 * @description     Este procedimiento carga en la lista de canciones todas las
 *                  canciones que se encuentran en el archivo de canciones para
 *                  tal fin ("canciones.txt") en el directorio del proyecto.
 */
void CargarListaCaciones(map<int, Song> &vCanciones2) {
    try {
        // Opens a file
        fstream fi("canciones.txt");
        string line, atribute[3];

        while (!fi.eof()) {
            getline(fi, line);
            stringstream lineStream(line);

            int i = 0;
            while (getline(lineStream, atribute[i], '|')) {
                i++;
            };
            
            int cod = atoi(atribute[0].c_str());
            pair<int, Song> pSong(cod, Song(cod, atribute[1], atribute[2]));
            
            vCanciones2.insert(pSong);
        }
        fi.close();
        vCanciones2.erase(0);
    } catch (exception &e) {
        cout << "The file could not be open";
    }
}

long djb2 (char *str) {
    unsigned long hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c;
    return hash;
}

int main(int argc, char** argv) {
    RadioApp app;
    app.solicitarCanciones();

    
    /* Pruebas varias, NO BORRAR
     * 
     * 
    // # de palabras en Autores 410
    // # de palabras en Titulos 821
    cout << "70%" << endl;
    cout << "El primo más cercano a 410*1.7=" << 410*1.7 << " es: " << getPrime(410*1.7) << endl;
    cout << "El primo más cercano a 821*1.7=" << 821*1.7 << " es: " << getPrime(821*1.7) << endl;
    cout << "75%" << endl;
    cout << "El primo más cercano a 410*1.75=" << 410*1.75 << " es: " << getPrime(410*1.75) << endl;
    cout << "El primo más cercano a 821*1.75=" << 821*1.75 << " es: " << getPrime(821*1.75) << endl;
    cout << "80%" << endl;
    cout << "El primo más cercano a 410*1.80=" << 410*1.8 << " es: " << getPrime(410*1.8) << endl;
    cout << "El primo más cercano a 821*1.80=" << 821*1.8 << " es: " << getPrime(821*1.8) << endl;
    cout << "85%" << endl;
    cout << "El primo más cercano a 410*1.85=" << 410*1.85 << " es: " << getPrime(410*1.85) << endl;
    cout << "El primo más cercano a 821*1.85=" << 821*1.85 << " es: " << getPrime(821*1.85) << endl;
    cout << "90%" << endl;
    cout << "El primo más cercano a 410*1.9=" << 410*1.9 << " es: " << getPrime(410*1.9) << endl;
    cout << "El primo más cercano a 821*1.9=" << 821*1.9 << " es: " << getPrime(821*1.9) << endl;
    
    for (int i = 0; i < 1500; i++)
        cout << ((210728963387 % 1553) + i*(1069 + 210728963387 % 1069)) % 1553 << endl;
     * 
     * 
     */
    
    return 0;
}

