/* 
 * File:   ItemCancion.h
 * Author: alf
 *
 * Created on 20 de noviembre de 2014, 13:42
 */

#ifndef ITEMCANCION_H
#define	ITEMCANCION_H

#include "Song.h"
#include <map>

using namespace std;

class ItemCancion {
public:
    ItemCancion             () : palabra(), canciones() {};
    ItemCancion             (string word, Song *s) : palabra(word) {
        pair<int, Song*> p(s->GetCode(), s);
        canciones.insert(p);
    };
    
    ItemCancion                         (const ItemCancion& orig);
    virtual             ~ItemCancion    () {};
    bool                operator<       (const ItemCancion& orig) const;
    bool                operator>       (const ItemCancion& orig) const;
    bool                operator==      (const ItemCancion& orig) const;
    void                setPalabra      (string palabra);
    string              getPalabra      () const;
    void                addSong         (Song *s);
    map<int, Song*> *   getSongs        ();
    
private:
    string              palabra;
    map<int, Song*>     canciones;
};

ItemCancion::ItemCancion(const ItemCancion& orig) {
    palabra     = orig.palabra;
    canciones   = orig.canciones;
}

void ItemCancion::addSong (Song *s) {
    pair<int, Song*> p(s->GetCode(), s);
    canciones.insert(p);
}

map<int, Song*> * ItemCancion::getSongs() {
    return &canciones;
}

bool ItemCancion::operator< (const ItemCancion& orig) const {
    return palabra < orig.palabra;
    
}

bool ItemCancion::operator> (const ItemCancion& orig) const {
   return palabra > orig.palabra;
}

bool ItemCancion::operator ==(const ItemCancion& orig) const {
    return palabra == orig.palabra;
}

void ItemCancion::setPalabra(string palabra) {
    this->palabra = palabra;
}

string ItemCancion::getPalabra() const {
    return palabra;
}

#endif	/* ITEMCANCION_H */

