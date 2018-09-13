#include "painlessMesh.h"
#include "IPAddress.h"

#ifdef ESP8266
#include "Hash.h"
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555


Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
// void sendMessage() ; // Prototype so PlatformIO doesn't complain
// Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

AsyncWebServer server(80);
IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);
int state = 0;
uint32_t myNodeId;
uint32_t targetNodeId;
int myControlPin = 2;

void setup() {
  Serial.begin(115200);
  pinMode(myControlPin, OUTPUT);
  Serial.println("ControlPin State :"+String(digitalRead(myControlPin)));
//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  myAPIP = IPAddress(mesh.getAPIP());
  myNodeId = mesh.getNodeId();
  Serial.println("My AP IP :" + myAPIP.toString()+ " My NodeId :"+String(myNodeId));

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<h1>Hello World !!<h1>");
  });

  server.on("/getdeviceinfo", HTTP_GET, [](AsyncWebServerRequest *request){
      sendDeviceInfo(request);
  });

  server.on("/switch", HTTP_POST, [](AsyncWebServerRequest *request){
  	 sendSwitchResponse(request);
  });

  // userScheduler.addTask( taskSendMessage );
  // taskSendMessage.enable();
  server.begin();
}

void loop() {
//  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}

void sendSwitchResponse(AsyncWebServerRequest *request){
  String message;
    if(request->hasParam("targetNodeId", true) && request->hasParam("state", true)){
      targetNodeId = atoi (request->getParam("targetNodeId", true)->value().c_str());
      state = request->getParam("state", true)->value().toInt();
      message = sendSwitchSignal();
    }else{
      message = debugRequest(request);
    }
    request->send(200, "text/json", message);
}

String debugRequest(AsyncWebServerRequest *request){
    String strResponse;
    int paramCount = request->params();
    int headers = request->headers();
    println("Request headers count:"+String(headers));
    println("Request parameters count:"+String(paramCount));
    DynamicJsonBuffer jsonBuffer;
    JsonObject &responseJson = jsonBuffer.createObject();
    responseJson["header"] = "following are header"; 
    for(int j=0;j<headers;j++){
      AsyncWebHeader* h = request->getHeader(j);
      responseJson[h->name()] = h->value(); 
    }
    responseJson["parameters"] = "following are parameters";
    for(int i=0;i<paramCount;i++){
      responseJson[request->argName(i)] = request->arg(i); 
    }
    responseJson.prettyPrintTo(Serial);
    responseJson.printTo(strResponse);
    return strResponse;
}

void sendDeviceInfo(AsyncWebServerRequest *request){
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonBuffer jsonBuffer;
  JsonObject &deviceJson = jsonBuffer.createObject();
  deviceJson["NodeId"] = myNodeId;
  deviceJson["NodeIp"] = myAPIP.toString();
  deviceJson["NodeTime"] = mesh.getNodeTime();
  deviceJson["EspHeap"] = ESP.getFreeHeap();
  JsonArray& nodeList = deviceJson.createNestedArray("Node_LIST");
  for(uint32_t nId : mesh.getNodeList()){
    nodeList.add(nId);
  }
  deviceJson.printTo(*response);
  request->send(response);
}

String sendSwitchSignal(){
	String strResponse;
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& jsonObj = jsonBuffer.createObject();
  jsonObj["targetNodeId"] = targetNodeId;
	jsonObj["command"] = "switch";
	jsonObj["state"] = state;
	jsonObj.printTo(strResponse);
  if(targetNodeId != myNodeId){
    Serial.printf("Sending Switch signal %d to nodeId: %u, from nodeId: %u", state, targetNodeId, myNodeId);
    jsonObj["success"] = mesh.sendSingle(targetNodeId, strResponse);
  }else{
    println("myNode is the targetNode");
    if(digitalRead(myControlPin) != state){
      digitalWrite(myControlPin, state);
      jsonObj["success"] = true;
    }else{
      jsonObj["success"] = "NSCN";   // No State Change Needed
    }
  }
  strResponse="";
  jsonObj.printTo(strResponse);
	jsonObj.printTo(Serial);
	return strResponse;
}

void println(String content){
	Serial.println(content);
}

// Needed for painless library

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  handleReceivedSignal(from, msg);
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections %s\n",mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

// void sendMessage() {
//   String msg = "HeartBeat from node ";
//   msg += mesh.getNodeId();
//   mesh.sendBroadcast( msg );
//   taskSendMessage.setInterval(TASK_SECOND * 10);
// }	

void handleReceivedSignal(uint32_t from, String &msg){
	String mess;
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& rjsonObj = jsonBuffer.parseObject(msg);
	int tempState = rjsonObj["state"];
	if(rjsonObj["command"] == "switch"){
		println("Received 'Switch' Command from node: "+String(from));
		if(digitalRead(myControlPin) != tempState){
			digitalWrite(myControlPin, tempState);
			mess = "ControlPin on node :"+String(targetNodeId)+" has been set to "+String(tempState)+" state.";
		}else{
			mess = "ControlPin on node :"+String(targetNodeId)+" already in "+String(tempState)+" state.";
		}
	}else{
		mess = "Unknown Command Received";
	}
	println(mess);
}
