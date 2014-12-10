/* 
 * File:   tableHash.h
 * Author: alf
 *
 * Created on 21 de noviembre de 2014, 18:16
 */

#ifndef TABLEHASH_H
#define	TABLEHASH_H

#include <vector>
#include <bitset>
#include <math.h>

using namespace std;

// Declaracion de funciones hechas al final de la clase
long getPrime(long num);

template<class U>
class Entry {
public:
    long key;
    U data;
    
    Entry   (long aKey, U aData) : key(aKey), data(aData) {};
    Entry   () : key(0), data() {};
};

template<class T, long L>
class TableHash {
private:
    unsigned long       prime;
    unsigned long       size;
    vector<Entry<T> >   table;
    bitset<L>           check;       // Estructura de datos para llevar las posiciones "ocupadas"/"vacías"/"libres"
    
    // Función de dispersión de la tabla
    unsigned long doubleDispersion (unsigned long key, int numColi) {
        unsigned long _hash = 0;
        _hash = ((key % size) + numColi*(prime + key % prime)) % size;
        return _hash;
    }
    
public:
    TableHash<T, L> () {
        size = L;
        table.resize(L);
        check.reset();
        prime = getPrime(L*0.7);
    };
    
    bool insert     (const long key, const T &data);
    T*   search     (const long key);
    int  tableSize  ();
};



/**
 * 
 * @return devuelve el tamaño ocupado en la tabla
 */
template<class T, long L>
int TableHash<T, L>::tableSize() {
    return check.count();
}

/**
 * 
 * @param key clave de búsqueda del objeto
 * @return T* devuelbe un puntero al objeto encontrado
 * @description 
 */
template<class T, long L>
T* TableHash<T, L>::search(const long key) {
    int i = 0;
    unsigned long pos;
    do {
        pos = doubleDispersion(key, i++);
        if (table[pos].key == key)
            return &table[pos].data;
    } while (check.test(pos));
    return 0;
}

/**
 * 
 * @param key clave de búsqueda para el dato
 * @param data
 * @return devuelbe true si lo ha insertado y false en caso de que el número de colisiones sea superior a 6
 */
template<class T, long L>
bool TableHash<T, L>::insert(const long key, const T& data) {
    int numColisions = 0;
    unsigned long pos;
    do {
        pos = doubleDispersion(key, numColisions++);
        if (!check.test(pos)) {
            table[pos].key = key;
            table[pos].data = data;
            check.flip(pos);
            return true;
        }
    } while (numColisions < 5);
    
    return false;
}

/**
 * 
 * @param num
 * @return primer número primo inferior al num
 * @description calcula el siguiente número primo inferior comprobando por todos
 *              los números inferiores al resultado de su raíz
 */
long getPrime(long num) {
    
    long n = num - 1;
    int m = sqrt(n);
    
    while (m > 1) 
    {
        if (n%m == 0) 
        {            
            n -= 1;
            m = sqrt(n);
        } 
        else
            --m;
        
        if (m == 1)
            return n;
    }
}
#endif	/* TABLEHASH_H */

