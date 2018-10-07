#include "painlessMesh.h"
#include "IPAddress.h"
#include <EEPROM.h>
#ifdef ESP8266
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

// #define   MESH_PREFIX     "whateverYouLike"
// #define   MESH_PASSWORD   "somethingSneaky"
#define   DEBUG_MODE  true
#define   MESH_PORT       5555
String MESH_PREFIX;
String MESH_PASSWORD;
String WIFI_PREFIX;
String WIFI_PASSWORD;

painlessMesh  mesh;

AsyncWebServer server(80);
IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);
int state = 0;
uint32_t myNodeId;
uint32_t targetNodeId;
int myControlPin = 2;
bool isConfigAvailable;
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  isConfigAvailable = isCredsAvailable();
  if(isConfigAvailable){
    MESH_PREFIX = getMeshSSID();
    MESH_PASSWORD = getMeshPSWD();
    WIFI_PREFIX = getWiFiSSID();
    WIFI_PASSWORD = getWiFiPSWD();
    pinMode(myControlPin, OUTPUT);
    println("ControlPin State :"+String(digitalRead(myControlPin)));
    mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
//     mesh.setDebugMsgTypes( ERROR | STARTUP | MESH_STATUS | CONNECTION);  // set before init() so that you can see startup messages
    // mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
    mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
    myAPIP = IPAddress(mesh.getAPIP());
    myNodeId = mesh.getNodeId();
    println("My AP IP :" + myAPIP.toString()+ " My NodeId :"+String(myNodeId));
    initMainRoutes();
  }else{
    println("Starting in configuration mode");
    println(WiFi.softAP("eSwitch")? "Ready" : "Failed!");
    initConfigRoutes();
  }

  initBasicRoutes();
  server.begin();
}

void loop() {
  mesh.update();
}
