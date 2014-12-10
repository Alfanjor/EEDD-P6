/* 
 * File:   Song.h
 * Author: zackreynolt
 *
 * Created on 22 de octubre de 2014, 8:12 PM
 */

#ifndef SONG_H
#define	SONG_H
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>

using namespace std;


class Song {
private:
    int _code;
    string _artist;
    string _title;
public:
    Song();
    Song(int code, string title, string artist) : _code(code), _artist(artist),
        _title(title) {};
    ~Song() {};

    void SetTitle(string _title) {
        this->_title = _title;
    }

    string GetTitle() const {
        return _title;
    }

    void SetArtist(string _artist) {
        this->_artist = _artist;
    }

    string GetArtist() const {
        return _artist;
    }

    void SetCode(int _code) {
        this->_code = _code;
    }

    int GetCode() const {
        return _code;
    }
    
    bool operator==(const Song &orig) {
        if (_code == orig._code)
            return true;
        return false;
    }
};

Song::Song() {
    _code = -1;
    _title = "Unknown";
    _artist = "Unknown";
}

#endif	/* SONG_H */

