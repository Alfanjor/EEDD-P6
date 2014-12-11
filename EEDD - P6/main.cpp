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
        
        //Mostrar lista de canciones
//        map<int, Song>::iterator it = vCanciones2.begin();
//        while (it != vCanciones2.end()) {
//            cout << it->first << " - " << it->second.GetTitle() << " - " << it->second.GetArtist() << endl;
//            it++;
//        }
        
//        int pArtista = 0, pTitulo = 0;
//        
//        // Construcción de las tablas de dispersión
//        for (int i = 0; i < vCanciones.size(); ++i) {
//            
//            string artista, titulo;
//            string lineArtist = vCanciones[i].GetArtist();
//            string lineTittle = vCanciones[i].GetTitle();
//            stringstream lineStreamArtist(lineArtist);
//            stringstream lineStreamTittle(lineTittle);
//            
//            while (getline(lineStreamArtist, artista, ' ')) {
//                for (int x = 0; x < artista.size(); x++)
//                    artista[x] = tolower(artista[x]);
//                
//                long key = djb2((char*) artista.c_str());
//                ItemCancion *p = tablaAutores.search(key);
//                if (!p) {
//                    if (!tablaAutores.insert(key, ItemCancion(artista, &vCanciones[i])))
//                        cout << "Demasiadas colisiones para pArtista: " << artista << " en la canción " << i << endl;
//                    pArtista++;
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
//        };
//        
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
                
                
                cout << "Reproduciendo canción " << 
                        vCanciones2[cancion].GetTitle() << 
                        " de " << vCanciones2[cancion].GetArtist() <<
                        "... (" << vCanciones2[cancion].GetCode() << ")" << endl;
                
                // Simular el tiempo de reproducción de la canción (entre 2 y 12 seg.)
                millisleep(2000 + 1000 * (rand() % 10));
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
                string palabra_buscada;
                
                //=================ESTO ES LO INTERESANTE===============//
                // Forma de buscar palabras (SIEMPRE EN MINUSCULA)

                //Pasar lo que buscamos a minúscula
                cin >> palabra_buscada;
                for (int x = 0; x < palabra_buscada.size(); x++)
                            palabra_buscada[x] = tolower(palabra_buscada[x]);
                ItemCancion *p;
                
                
                if (letra == "A")
                    p = tablaAutores.search(djb2((char*) palabra_buscada.c_str()));
                else
                    p = tablaTitulos.search(djb2((char*) palabra_buscada.c_str()));
                
                if (p) {
                    vector<Song*> *songs = p->getSongs();
                    cout << endl;
                    cout << "Canciones con la palabra " << palabra_buscada << endl;
                    for (int i = 0; i < songs->size(); i++)
                        cout << songs->at(i)->GetCode() << " - " <<
                                songs->at(i)->GetArtist() << " - " <<
                                songs->at(i)->GetTitle() << endl;
                    cout << "Introduce el código de la canción deseada: ";
                    cin >> cancion;
                } else {
                    cout << "\n No hay canciones con la palabra " << palabra_buscada << endl;;
                    
                    //Para que no la de por repetida al no introducir nada
                    cancion = 0; 
                }
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
            
                //Comprobar que se ha introducido alguna canción
                if (cancion > 1 && cancion < 500) {
                    //Comprobar si es la primera petición que introducimos (vacío)
                    if (!vPeticiones.empty()) {
                        vPeticiones.push_back(peticion);
                    } else 
                        vPeticiones.push_back(peticion);
                }

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

