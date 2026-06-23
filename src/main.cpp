/*
   Copyright (C) 2021 Alessandro Orlando

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/>.
   
   Description:
   Simple Bug with I2S MEMS microphone and Audio Streaming via UDP
   Under Linux for listener:
   netcat -u -p 16500 -l | play -t raw -r 16000 -b 16 -c 2 -e signed-integer -
   Under Linux for recorder (give for file.mp3 the name you prefer) : 
   netcat -u -p 16500 -l | rec -t raw -r 16000 -b 16 -c 2 -e signed-integer - file.mp3
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <I2S.h>

#ifndef STASSID
#define STASSID "YOUR_SSID"
#define STAPSK  "YOUR_PASS"
#endif

const char *SSID = STASSID;
const char *PASS = STAPSK;

WiFiUDP udp;
const IPAddress listener(192, 168, 1, 40);
const int port = 16500;

int16_t buffer[100][2];

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("My IP: ");
  Serial.println(WiFi.localIP());

  i2s_rxtx_begin(true, false);
  i2s_set_rate(16000);

  Serial.print("\nStart the listener on ");
  Serial.print(listener);
  Serial.print(":");
  Serial.println(port);
  Serial.println("Under Linux for listener: netcat -u -p 16500 -l | play -t raw -r 16000 -b 16 -c 2 -e signed-integer -");
  Serial.println("Under Linux for recorder: netcat -u -p 16500 -l | rec -t raw -r 16000 -b 16 -c 2 -e signed-integer - file.mp3");
  udp.begin(port);
  udp.beginPacket(listener, port);
  udp.write("I2S Receiver\r\n");
  udp.endPacket();

}

void loop() {
  static int cnt = 0;
  for (int i = 0; i < 100; i++) {
    i2s_read_sample(&buffer[i][0], &buffer[i][1], true);
  }
  udp.beginPacket(listener, port);
  udp.write((uint8_t*)buffer, sizeof(buffer));
  udp.endPacket();
  cnt++;
  if ((cnt % 100) == 0) {
    Serial.printf("%d\n", cnt);
  }
}
