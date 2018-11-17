#include "painlessMesh.h"
#include "IPAddress.h"
#include "Memory.h"
#include <FS.h>
#ifdef ESP8266
#include "Hash.h"
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#define   DEBUG_MODE  true
#define   MESH_PORT   5555
#define   HOSTNAME    "HTTP_BRIDGE"
String MESH_PREFIX;
String MESH_PASSWORD;
String WIFI_PREFIX;
String WIFI_PASSWORD;
IPAddress getlocalIP();
const char* filename = "/index.html";
painlessMesh  mesh;
Memory Mem;
AsyncWebServer server(3030);
IPAddress myIP(0,0,0,0);
// IPAddress myapIp(0,0,0,0);
struct Node{
  const int controlPin = 2;
  int pinState = 0;
  uint32_t id;
  IPAddress apIp;
};
Node thisNode;
Node targetNode;

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  if(Mem.isCredsAvailable()){
    MESH_PREFIX = Mem.getMeshSSID();
    MESH_PASSWORD = Mem.getMeshPSWD();
    WIFI_PREFIX = Mem.getWiFiSSID();
    WIFI_PASSWORD = Mem.getWiFiPSWD();
    pinMode(thisNode.controlPin, OUTPUT);
    println("ControlPin State :"+String(digitalRead(thisNode.controlPin)));
    mesh.setDebugMsgTypes( ERROR | STARTUP );
    mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
//    mesh.setName(Mem.getDeviceName());
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    thisNode.id = mesh.getNodeId();
    if(Mem.isBridge()){
      println("Starting Node in Bridge Mode");
      mesh.stationManual(WIFI_PREFIX, WIFI_PASSWORD);
      mesh.setHostname(HOSTNAME);
      mesh.setRoot(true);
      mesh.setContainsRoot(true);
    }
    thisNode.apIp = IPAddress(mesh.getAPIP());
    println("My NodeId :"+String(thisNode.id)+" My AP IP :" + thisNode.apIp.toString());
    initMainRoutes();
  }else{
    println("Starting in configuration mode");
    println(WiFi.softAP("eSwitch-configure-me","123456789")? "Ready" : "Failed!");
    initConfigRoutes();
  }
  initBasicRoutes();
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
  server.begin();
}

void loop() {
  mesh.update();
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
  }
}

void receivedCallback( uint32_t from, String &msg ) {
  println("Received from "+String(from)+" "+msg);
  handleReceivedSignal(from, msg);
}

void newConnectionCallback(uint32_t nodeId) {
  println("New Connection detected, nodeId = "+String(nodeId));
}

void changedConnectionCallback() {
  println("Changed connections "+String(mesh.subConnectionJson()));
}


IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
