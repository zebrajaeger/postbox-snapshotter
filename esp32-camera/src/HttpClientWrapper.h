#pragma once

#include <Arduino.h>

class HttpClientWrapper {
public:
    /**
     * Konstruktor
     * @param host       Server-IP oder Domain
     * @param port       Server-Port (z.B. 80 oder 3000)
     * @param endpoint   Upload-Pfad, z.B. "/img"
     */
    HttpClientWrapper(const char* host, uint16_t port, const char* endpoint);

    /**
     * Führt einen HTTP-Multipart-Upload eines Bildes durch
     * @param data       Pointer auf Bilddaten (JPEG/PNG...)
     * @param length     Anzahl Bytes
     * @param filename   Gewünschter Dateiname z.B. "bild.jpg"
     * @return           true bei Erfolg, false bei Fehler
     */
    bool uploadImage(const uint8_t* data, size_t length, const char* filename);

private:
    const char* _host;
    uint16_t    _port;
    const char* _endpoint;
    String      _boundary;

    /**
     * Erzeugt eine zufällige Boundary
     */
    void generateBoundary();
};
