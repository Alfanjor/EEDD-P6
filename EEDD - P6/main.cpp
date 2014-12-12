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
#include <stdlib.h>

#include "tinythread.h"
#include "millisleep.h"
#include "Song.h"
#include "Request.h"
#include "ItemCancion.h"

using namespace std;
using namespace tthread;

void CargarListaCaciones(map<int, Song> &map_Canciones);
long djb2 (char *str);

class RadioApp {
    map<int, Song> map_Canciones;
    
    vector<Request>  vPeticiones;
    vector<int> historico;       //Historico con los códigos de las canciones que ya se han reproducido

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
    //unordered_map<class _Key,class _Tp,class _Hash=hash<_Key>,class _Pred=std::equal_to<_Key>,class _Alloc=std::allocator<std::pair<const _Key,_Tp> >>
    unordered_map<long, ItemCancion> tablaAutores;
    unordered_map<long, ItemCancion> tablaTitulos;
   
    thread  threadReproducirCanciones;
    mutex   semaforo;
    bool    pinchar;

    static void hebraReproducirCanciones(void *arg) {
        RadioApp *radioApp = static_cast<RadioApp *> (arg);
        radioApp->reproducirCanciones();
    }

public:

    RadioApp() : threadReproducirCanciones(hebraReproducirCanciones, this), tablaAutores(691), tablaTitulos(1381) {
        pinchar = true;
        CargarListaCaciones(map_Canciones);
        
        // Construcción de las tablas de dispersión
        for (map<int, Song>::iterator it = map_Canciones.begin(); it != map_Canciones.end(); ++it) {
            
            string artista, titulo;
            string lineArtist = it->second.GetArtist();
            string lineTittle = it->second.GetTitle();
            stringstream lineStreamArtist(lineArtist);
            stringstream lineStreamTittle(lineTittle);
            
            while (getline(lineStreamArtist, artista, ' ')) {
                for (int x = 0; x < artista.size(); x++)
                    artista[x] = tolower(artista[x]);
                
                long key = djb2((char*) artista.c_str());
                unordered_map<long, ItemCancion>::iterator it_aux = tablaAutores.find(key);
                if (it_aux == tablaAutores.end()) {
                    pair<long, ItemCancion> p(key, ItemCancion(artista, &it->second));
                    tablaAutores.insert(p);
                } else
                    it_aux->second.addSong(&it->second);
            };
            
            while (getline(lineStreamTittle, titulo, ' ')) {
                for (int x = 0; x < titulo.size(); x++)
                    titulo[x] = tolower(titulo[x]);
                
                long key = djb2((char*) titulo.c_str());
                unordered_map<long, ItemCancion>::iterator it_aux = tablaTitulos.find(key);
                if (it_aux == tablaTitulos.end()) {
                    pair<long, ItemCancion> p(key, ItemCancion(titulo, &it->second));
                    tablaTitulos.insert(p);
                } else
                    it_aux->second.addSong(&it->second);
            };
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
                historico.push_back(cancion);
                vPeticiones.pop_back();
                semaforo.unlock();
                
                
                cout << "\nReproduciendo canción " << 
                        map_Canciones[cancion].GetTitle() << 
                        " de " << map_Canciones[cancion].GetArtist() <<
                        "... (" << map_Canciones[cancion].GetCode() << ")" << endl;
                
                // Simular el tiempo de reproducción de la canción (entre 2 y 12 seg.)
                millisleep(2000 + 1000 * (rand() % 10));
                
            }
            
            //Reproducir al azar si la lista de peticiones se vacía
            if (vPeticiones.size() < 5) {
                Request peticion((rand()%500)+1);
                vPeticiones.push_back(peticion);
            }
            
        } while (pinchar);
    }

    void solicitarCanciones() {
        int cancion;
        
        cout << "¡Bienvenido a Radionauta v6!" << endl;
        
        string letra;
        // Pedir canciones hasta que se introduce "S"
        do {
            cout << "\nIntroduce C para código de canción." << endl;
            cout << "Introduce A ó T para buscar la canción deseada" << endl;
            cout << "Introduce S para Salir." << endl;
            cin >> letra;

            while (letra != "C" && letra != "A" && letra != "T" && letra != "S") {
                cin.clear();
                cin.ignore(100, '\n');
                cout << "\nPor favor, 'C' para Código, 'A' o 'T' para buscar."
                        << endl;
                cout << "'S' para salir: ";
                cin >> letra;
            }

            if (letra == "C") {
                cout << "Introduzca el código de la canción que quiere introducir" 
                        << endl;
                cin >> cancion;
            } else if (letra == "A" || letra == "T"){ 
                cout << "Introuce la palabra que quieres buscar: ";
                string frase;
                
                std::cin.ignore();
                getline(cin,frase);
                
                //Pasar lo que buscamos a minúscula
                for (int x = 0; x < frase.size(); x++)
                            frase[x] = tolower(frase[x]);
                
                // Comienza la búsqueda
                unordered_map<long, ItemCancion>::iterator it_aux;
                string palabra;
                stringstream ls_frase(frase);
                
                if (letra == "A") {
                    while (getline(ls_frase, palabra, ' ')) {
                        long key = djb2((char*) palabra.c_str());
                        it_aux = tablaAutores.find(key);
                        if (it_aux != tablaAutores.end()) {
                            cout << "Canciones con la palabra: " << palabra << endl;
                            map<int, Song*> *map_canciones = it_aux->second.getSongs();
                            for (map<int, Song*>::iterator it_canciones = map_canciones->begin(); it_canciones != map_canciones->end(); ++it_canciones)
                                cout << it_canciones->second->GetCode() << " - " << it_canciones->second->GetTitle() << " - " << it_canciones->second->GetArtist() << endl;
                        } else {
                            cout << "No se ha encontrado ninguna canción "
                                    "para el Artista " << palabra << endl;
                        }
                    }

                } else {
                    while (getline(ls_frase, palabra, ' ')) {
                        long key = djb2((char*) palabra.c_str());
                        it_aux = tablaTitulos.find(key);
                        if (it_aux != tablaTitulos.end()) {
                            cout << "Canciones con la palabra: " << palabra << endl;
                            map<int, Song*> *map_canciones = it_aux->second.getSongs();
                            for (map<int, Song*>::iterator it_canciones = map_canciones->begin(); it_canciones != map_canciones->end(); ++it_canciones) 
                                cout << it_canciones->second->GetCode() << " - " << it_canciones->second->GetTitle() << " - " << it_canciones->second->GetArtist() << endl;
                        } else {
                            cout << "No se ha encontrado ninguna canción "
                                    "para el Título " << palabra << endl;
                        }
                    }
                }
                
                cout << "Introduce el código de la cancion que te interesa: ";
                cin >> cancion;
            } 

            Request peticion(cancion);
            
            // Comprobamos que no haya sido de las 100 últimas reproducidas
            int j = 0;
            bool reproducida = false;

            while (j < historico.size() && j < 100 && !reproducida) {
                if (historico[j] == cancion) {
                    cout << "La canción " << cancion 
                            <<  " fue de las últimas 100 reproducidas" << endl;
                    reproducida=true;
                }
                j++;
            }
            
            //Si no lo ha sido, podemos continuar
            if (!reproducida) {
                semaforo.lock();
            
                //Comprobar que realmente se ha introducido alguna canción
                if (cancion > 1 && cancion < 500) 
                    vPeticiones.push_back(peticion);

                semaforo.unlock();
            }
            
        } while (letra != "S");
        
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
void CargarListaCaciones(map<int, Song> &map_Canciones) {
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
            
            map_Canciones.insert(pSong);
        }
        fi.close();
        map_Canciones.erase(0);
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
    
    return 0;
}

