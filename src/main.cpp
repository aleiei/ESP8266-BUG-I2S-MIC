/*
   MIT License

   Copyright (c) 2021 Alessandro Orlando

   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute,
   sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
   TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVEN
   SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
   OR OTHER DEALINGS IN THE SOFTWARE.
   
   Description:
   Simple I2S MEMS microphone Audio Streaming via UDP transmitter
   Needs a UDP listener like netcat on port 16500 on listener PC
   Needs a SoX with mp3 handler for Recorder
   Under Linux for listener:
   netcat -u -p 16500 -l | play -t raw -r 16000 -b 16 -c 2 -e signed-integer -
   Under Linux for recorder (give for file.mp3 the name you prefer) : 
   netcat -u -p 16500 -l | rec -t raw -r 16000 -b 16 -c 2 -e signed-integer - file.mp3
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <i2s.h>

#ifndef STASSID
#define STASSID "YOUR_SSID"
#define STAPSK  "YOUR_PASS"
#endif

const char *SSID = STASSID;
const char *PASS = STAPSK;

WiFiUDP udp;
// Set your listener PC's IP here in according with your DHCP network. In my case is 192.168.1.40:
const IPAddress listener = { 192, 168, 1, 40 };
const int port = 16500;

int16_t buffer[100][2]; // Determine the Sampling time

void setup() {
  Serial.begin(115200);

  // Connect to WiFi network
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

  i2s_rxtx_begin(true, false); // Enable I2S RX
  i2s_set_rate(16000);

  Serial.print("\nStart the listener on ");
  Serial.print(listener);
  Serial.print(":");
  Serial.println(port);
  Serial.println("Under Linux for listener: netcat -u -p 16500 -l | play -t raw -r 16000 -b 16 -c 2 -e signed-integer -");
  Serial.println("Under Linux for recorder: netcat -u -p 16500 -l | play -t raw -r 16000 -b 16 -c 2 -e signed-integer - file.mp3");
  udp.beginPacket(listener, port);
  udp.write("I2S Receiver\r\n");
  udp.endPacket();

}

void loop() {
  static int cnt = 0;
  // Each loop will send 100 raw samples (400 bytes)
  // UDP needs to be < TCP_MSS which can be 500 bytes in LWIP V2
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
