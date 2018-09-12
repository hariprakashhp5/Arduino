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
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", getNodeInfo());
    if (request->hasArg("BROADCAST")){
      String msg = request->arg("BROADCAST");
      mesh.sendBroadcast(msg);
    }
  });

  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
  	String message;
  	if(request->hasParam("targetNodeId", true) && request->hasParam("state", true)){
  		targetNodeId = atoi (request->getParam("targetNodeId", true)->value());
  		state = request->getParam("state", true)->value().toInt();
  		message = sendSwitchSignal();
  	}else{
  		int paramCount = request->params();
  		for(int i=0;i<paramCount;i++){
		 message += request->argName(i).c_str()+" "+request->arg(i).c_str()+"\n";
		}
  		message += "Found the above params only";
  	}
  	request->send(200, "text/html", "Hello, POST: " + message);
  });

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  server.begin();
}

void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}

String getNodeInfo(){
	char* htmlString = "<table><thead><tr><td>Parameter</td><td>Value</td></tr></thead>"+
						"<tbody><tr><td>NodeId</td></tr><tr><td>"+String(myNodeId)+"</td></tr>"+
						"<tr><td>NodeIp</td></tr><tr><td>"+String(myAPIP)+"</td></tr>"+
						"<tr><td>NodeTime</td></tr><tr><td>"+String(mesh.getNodeTime())+"</td></tr>"+
						"</tbody></table><table><thead><tr><td>NodeList</td></thead><tbody>";
		for(uint32_t n : mesh.getNodeList()){
			htmlString += "<tr><td>"+n+"</td></tr>";
		}
		htmlString += "</tbody></table>";
		return htmlString;
}

String sendSwitchSignal(){
	String message="Default Message";
	if(targetNodeId != myNodeId){
		Serial.printf("Sending Switch signal %d to nodeId: %u, from nodeId: %u", state, targetNodeId, myNodeId);
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& jsonObj = jsonBuffer.createObject();
		jsonObj["command"] = "switch";
		jsonObj["state"] = state;
		String jsonStr;
		jsonObj.printTo(jsonStr);
		jsonObj.printTo(Serial);
		message = mesh.sendSingle(targetNodeId, jsonStr);
	}else{
		println("myNode is the targetNode");
		if(digitalRead(myControlPin) != state){
			digitalWrite(myControlPin, state);
			message = "ControlPin on node :"targetNodeId+" has been set to "+String(state)+" state.";
		}else{
			message = "ControlPin on node :"targetNodeId+" already in "+String(state)+" state.";
		}
	}
	println(message);
	return message;
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

void sendMessage() {
  String msg = "HeartBeat from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval(TASK_SECOND * 10);
}	

void handleReceivedSignal(uint32_t from, String &msg){
	String mess;
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& rjsonObj = jsonBuffer.parseObject(msg);
	int tempState = rjsonObj['state'].toInt()
	if(rjsonObj['command'] == 'switch'){
		println("Received 'Switch' Command from node: "+String(from));
		if(digitalRead(myControlPin) != tempState){
			digitalWrite(myControlPin, tempState);
			mess = "ControlPin on node :"targetNodeId+" has been set to "+String(tempState)+" state.";
		}else{
			mess = "ControlPin on node :"targetNodeId+" already in "+String(tempState)+" state.";
		}
	}else{
		mess = "Unknown Command Received";
	}
	println(mess);
}