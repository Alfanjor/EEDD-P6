/* 
 * File:   request.h
 * Author: zackreynolt
 *
 * Created on October 22, 2014, 8:15 PM
 */

#ifndef REQUEST_H
#define	REQUEST_H

using namespace std;

class Request {
private:
    int _cod;           // Song code
    int _nRequest;      // Number of requests
    
public:
    Request             ();
    Request             (int code);
    Request             (const Request &orig);
    ~Request            () {};
    
    void addRequest     (int code);
    void setNRequest    (int _nRequest);
    int getNRequest     () const;
    void setCod         (int _cod);
    int getCod          () const;
    bool operator==     (const Request &orig);
    bool operator<      (const Request &orig) const;
    void operator=      (const Request &orig);
};

Request::Request() {
    _cod = -1;
    _nRequest = -1;
}

Request::Request(int code) {
    _cod = code;
    _nRequest = 1;
}

Request::Request(const Request& orig) {
    _cod = orig._cod;
    _nRequest = orig._nRequest;
}

void Request::setNRequest(int _nRequest) {
    this->_nRequest += _nRequest;
}

int Request::getNRequest() const {
    return _nRequest;
}

void Request::setCod(int _cod) {
    this->_cod = _cod;
}

int Request::getCod() const {
    return _cod;
}

void Request::addRequest(int code) {
    _nRequest++;
}

bool Request::operator ==(const Request& orig) {
    if (_cod == orig._cod)
        return true;
    return false;
}

bool Request::operator  <(const Request& orig) const {
    if (_nRequest < orig._nRequest)
        return true;
    return false;
}

void Request::operator  =(const Request& orig) {
    this->_cod      = orig._cod;
    this->_nRequest = orig._nRequest;
}

#endif	/* REQUEST_H */

