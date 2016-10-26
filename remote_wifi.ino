/*
  WiFi Web Server LED Blink

  A simple web server that lets you blink an LED via the web.
  This sketch will create a new access point (with no password).
  It will then launch a new server and print out the IP address
  to the Serial monitor. From there, you can open that address in a web browser
  to turn on and off the LED on pin 13.

  If the IP address of your shield is yourAddress:
    http://yourAddress/H turns the LED on
    http://yourAddress/L turns it off

  created 25 Nov 2012
  by Tom Igoe
  adapted to WiFi AP by Adafruit
 */

#include <SPI.h>
#include <WiFi101.h>

int led =  LED_BUILTIN;

char ssid[] = "wifi101-network"; // created AP name
char pass[] = "1234567890";      // AP password (needed only for WEP, must be exactly 10 or 26 characters in length)
int keyIndex = 0;                // your network key Index number (needed only for WEP)

String newSSID = "";
String newPASS = "";

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Access Point Web Server");

  pinMode(led, OUTPUT);      // set the LED pin mode

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // print the network name (SSID);
  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssid);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }

  // wait 10 seconds for connection:
  delay(10000);

  // start the web server on port 80
  server.begin();

  // you're connected now, so print out the status
  printWifiStatus();
}


void loop() {
  // compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    } 
  }
  
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
//            client.print("Click <a href=\"/H\">here</a> turn the LED on<br>");
//            client.print("Click <a href=\"/L\">here</a> turn the LED off<br>");
            client.print("<form>");
            client.print("<input type=\"text\" placeholder=\"ssid\" id=\"ssid\" />");
            client.print("<input type=\"text\" placeholder=\"password\" id=\"password\" />");
            client.print("<button type=\"button\" onclick=\"changeUrl()\">submit</button>");
            client.print("</form>");
            client.print("<script>");
            client.print("var originalURL = document.referrer;");
            client.print("function changeUrl() {");
            client.print("var ssid = document.getElementById('ssid').value;");
            client.print("var password = document.getElementById('password').value;");
            client.print("var redirect2url = \"\";");
//            client.print("console.log(ssid, password);");
            client.print("if (ssid.length == 0) {  return; }");
            client.print("redirect2url += '?ssid=' + ssid;");
            client.print("if (password.length > 0) { redirect2url += '&password=' + password; }");
            client.print("window.location = originalURL + redirect2url + '&done'; }");
            client.print("</script>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(led, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(led, LOW);                // GET /L turns the LED off
        }

        // get input ssid and pass
        if (currentLine.indexOf("&done") != -1) {
          int index = currentLine.indexOf("ssid=");
          if (index != -1) {
            index += 5;
            newSSID = "";
            while (currentLine.charAt(index) != '&') {
              newSSID += currentLine.charAt(index++);
            }
            Serial.println("ssid="+newSSID);
            index = currentLine.indexOf("password=");
            newPASS = "";
            if (index != -1) {
              index += 9;
              while (currentLine.charAt(index) != '&') {
                newPASS += currentLine.charAt(index++);
              }
              Serial.println("password="+newPASS);
            }
          } 
        }
      }
    }
    // close the connection:
//    if (newSSID.length() > 0) {
//      char* wifissid;
//      for (int i = 0; i < newSSID.length(); i++) {
//        wifissid[i] = newSSID.charAt(i);
//      }
//      char* wifipass;
//      for (int i = 0; i < newPASS.length(); i++) {
//        wifipass[i] = newPASS.charAt(i);
//      }
//      Serial.println(wifissid);
//      Serial.println(wifipass);
////      connectToWiFi(wifissid, wifipass);
//    }
    client.stop();
    Serial.println("client disconnected");
  }
}

void connectToWiFi(char* ssid_wifi, char* pass_wifi) {
//  char ssid_wifi[] = newSSID;
//  char pass_wifi[] = newPASS;
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to open SSID: ");
    Serial.println(ssid);
    status = newPASS.length() > 0 ? WiFi.begin(ssid_wifi, pass_wifi) : WiFi.begin(ssid_wifi);
    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();
}

void printWifiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

  // print your subnet mask:
  IPAddress subnet = WiFi.subnetMask();
  Serial.print("NetMask: ");
  Serial.println(subnet);

  // print your gateway address:
  IPAddress gateway = WiFi.gatewayIP();
  Serial.print("Gateway: ");
  Serial.println(gateway);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
