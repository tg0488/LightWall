#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);   //Web server object. Will be listening in port 80 (default for HTTP)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN    5
#define NUMPIXELS 49

const int wallWidth = 7;
const int wallHeight = 7;

int wallState [wallHeight][wallWidth][3] = {
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
};

//int mappedWall [wallHeight][wallWidth] = {
//  {42, 43, 44, 45, 46, 47, 48},
//  {35, 36, 37, 38, 39, 40, 41},
//  {27, 28, 29, 30, 31, 32, 33},
//  {21, 22, 23, 24, 25, 26, 27},
//  {14, 15, 16, 17, 18, 19, 20},
//  {7, 8, 9, 10, 11, 12, 13},
//  {0, 1, 2, 3, 4, 5, 6}
//};


int actualWall [wallHeight][wallWidth] = {
  {42, 43, 44, 45, 46, 47, 48},
  {41, 40, 39, 38, 37, 36, 35},
  {28, 29, 30, 31, 32, 33, 34},
  {27, 26, 25, 24, 23, 22, 21},
  {14, 15, 16, 17, 18, 19, 20},
  {13, 12, 11, 10, 9, 8, 7},
  {0, 1, 2, 3, 4, 5, 6}
};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


void connectToWifi() {
  WiFi.begin("Klick Dev", "D3vw1f1z"); //Connect to the WiFi network

  while (WiFi.status() != WL_CONNECTED) { //Wait for connection
    delay(500);
    Serial.println("Waiting to connect…");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP to access the server
}

void httpHandler() {
  server.on("/lightwall", lightwall_handler); //Associate the handler function to the path
  server.on("/reset", reset_handler); //Associate the handler function to the path
  server.on("/state", state_handler); //Associate the handler function to the path

  server.begin(); //Start the server
  Serial.println("Server listening");
}

void setup() {
  pixels.begin();
  pixels.show();
  Serial.begin(115200);

  connectToWifi();
  httpHandler();
}

void loop() {
  server.handleClient(); //Handling of incoming requests
}


void lightwall_handler() { //Handler
  DynamicJsonBuffer jsonBuffer(200);
  //given that Strings are not char arrays in C we have to convert the argument(in json format) we get from the server in to a character array to be parsed
  int l = (int) server.arg(0).length(); //we find the length of the string
  char json[l + 1]; //we initialize the array
  server.arg(0).toCharArray(json, l + 1); //We used an arduino built in function to change the server argument in to json format
  Serial.println(json);
  JsonObject& light = jsonBuffer.parseObject(json);//the first step in parsing our json format charcter array is creating a json object which is the parsed charcter array
  if (!light.success()) {
    server.send(500, "text/plain", "error parsing json object!");
    return;
  }

  int arraySize = light["array"].size();//Since out argment will be an array of x and y locations as well as rgb pararmeters for each led to be turned on we find the length of the array
  for (int i = 0; i < arraySize; i++) { //we run through the array of led arguments
    int X = light["array"][i]["X"];//x location
    int Y = light["array"][i]["Y"];//y location
    int R = light["array"][i]["RGB"][0];//R value
    int G = light["array"][i]["RGB"][1];//G value
    int B = light["array"][i]["RGB"][2];//B value
    Serial.println(X);
    Serial.println(Y);
    Serial.println(R);
    Serial.println(G);
    Serial.println(B);
    ledON(X, Y, R, G, B);
  }
  reifyWallState();

  server.send(200, "text/plain", "ok");
}



void state_handler() {
  String response = String("{\"array\": [");
  response += "]}";
  server.send(200, "application/json ", response);
}


void reset() {
  for (int X = 0; X < wallWidth; X++) {
    for (int Y = 0; Y < wallHeight; Y++) {
      int R = wallState[X][Y][0];
      int G = wallState[X][Y][1];
      int B = wallState[X][Y][2];
    }
  }
  reifyWallState();
}

void reset_handler() {
  reset();
  server.send(200, "text/plain", "ok");
}


void ledON(int X, int Y, int R, int G, int B) {
  wallState[X][Y][0] = R;
  wallState[X][Y][0] = G;
  wallState[X][Y][0] = B;
}

void reifyWallState () {
  for (int X = 0; X < wallWidth; X++) {
    for (int Y = 0; Y < wallHeight; Y++) {
      int R = wallState[X][Y][0];
      int G = wallState[X][Y][1];
      int B = wallState[X][Y][2];

      pixels.setPixelColor(actualWall[Y][X], pixels.Color(R, G, B));
      pixels.show();
    }
  }
}

