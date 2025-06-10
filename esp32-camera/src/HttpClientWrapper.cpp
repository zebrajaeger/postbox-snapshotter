#include "HttpClientWrapper.h"

#include <WiFi.h>

HttpClientWrapper::HttpClientWrapper(const char* host, uint16_t port, const char* endpoint)
    : _host(host), _port(port), _endpoint(endpoint) {
  generateBoundary();
}

void HttpClientWrapper::generateBoundary() {
  // einfache Zufalls-Boundary
  _boundary = "----ESP32FormBoundary";
  for (int i = 0; i < 8; ++i) {
    char c = 'A' + (random(0, 26));
    _boundary += c;
  }
}

bool HttpClientWrapper::uploadImage(const uint8_t* data, size_t length, const char* filename) {
  // Verbindung aufbauen
  WiFiClient client;
  if (!client.connect(_host, _port)) {
    Serial.println("Verbindung zum HTTP-Server fehlgeschlagen");
    return false;
  }

  // Multipart-Headerteile erstellen
  String part1 = "--" + _boundary + "\r\n";
  part1 += "Content-Disposition: form-data; name=\"filename\"\r\n\r\n";
  part1 += filename;
  part1 += "\r\n";

  String part2 = "--" + _boundary + "\r\n";
  part2 += "Content-Disposition: form-data; name=\"image\"; filename=\"" + String(filename) + "\"\r\n";
  part2 += "Content-Type: application/octet-stream\r\n\r\n";

  String closing = "\r\n--" + _boundary + "--\r\n";

  // Gesamtlänge berechnen
  size_t contentLength = part1.length() + part2.length() + length + closing.length();

  // HTTP-Request senden
  client.print(String("POST ") + _endpoint + " HTTP/1.1\r\n");
  client.print(String("Host: ") + _host + ":" + _port + "\r\n");
  client.print("Content-Type: multipart/form-data; boundary=" + _boundary + "\r\n");
  client.print("Content-Length: " + String(contentLength) + "\r\n");
  client.print("Connection: close\r\n\r\n");

  // Body senden
  client.print(part1);
  client.print(part2);
  client.write(data, length);
  client.print(closing);

  // Antwort lesen (Status-Code prüfen)
  // Warte bis mindestens die erste Zeile da ist
  unsigned long timeout = millis() + 5000;
  while (!client.available() && millis() < timeout) {
    delay(10);
  }
  if (!client.available()) {
    Serial.println("Timeout beim Warten auf HTTP-Antwort");
    client.stop();
    return false;
  }

  // HTTP-Statuszeile
  String statusLine = client.readStringUntil('\n');
  client.stop();
  // Beispiel: "HTTP/1.1 200 OK"
  Serial.println("Status: " + statusLine);
  if (statusLine.indexOf("200") != -1) {
    return true;
  }
  return false;
}
