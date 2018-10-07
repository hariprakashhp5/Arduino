void setupWifiParameters(AsyncWebServerRequest *request){
	bool status=false;
	if(request->hasParam("MeshSSID", true) && request->hasParam("MeshPSWD", true)){
		String meshSSID = request->getParam("MeshSSID", true)->value();
		String meshPSWD = request->getParam("MeshPSWD", true)->value();
		String wifiSSID = request->getParam("WiFiSSID", true)->value();
		String wifiPSWD = request->getParam("WiFiPSWD", true)->value();
		if(wifiSSID!="" && wifiPSWD!=""){
			setWiFiSSID(wifiSSID);
			setWiFiPSWD(wifiPSWD);
		}
		status = setMeshSSID(meshSSID) && setMeshPSWD(meshPSWD);
	}
	if(status){
		setCredsState(1);
		request->send(200, "text/html", configSuccess());
	}else{
		setCredsState(0);
		request->send(200, "text/html", configFailed());
	}
}

void sendResponse(AsyncWebServerRequest *request){
  String message;
    if(request->hasParam("targetNodeId", true) && request->hasParam("state", true)){
      targetNodeId = atoi (request->getParam("targetNodeId", true)->value().c_str());
      state = request->getParam("state", true)->value().toInt();
      message = sendSignal("switch");
    }else if(request->hasParam("targetNodeId", true)){
      targetNodeId = atoi (request->getParam("targetNodeId", true)->value().c_str());
      message = sendSignal("restart");
    }else{
      message = debugRequest(request);
    }
    request->send(200, "text/json", message);
}

String sendSignal(String cmd){
	String strResponse;
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& jsonObj = jsonBuffer.createObject();
  jsonObj["targetNodeId"] = targetNodeId;
	jsonObj["command"] = cmd;
	jsonObj["state"] = state;
	jsonObj.printTo(strResponse);
  if(targetNodeId != myNodeId){
    Serial.printf("Sending Switch signal %d to nodeId: %u, from nodeId: %u", state, targetNodeId, myNodeId);
    jsonObj["trigger"] = mesh.sendSingle(targetNodeId, strResponse);
  }else{
    println("myNode is the targetNode");
    jsonObj["success"] = executeCommand(cmd);
  }
  strResponse="";
  jsonObj.printTo(strResponse);
	jsonObj.printTo(Serial);
	return strResponse;
}

String executeCommand(String cmd){
  String returnVal;
  if(cmd == "switch"){
    if(digitalRead(myControlPin) != state){
      digitalWrite(myControlPin, state);
      returnVal = "true";
    }else{
      returnVal = "NSCN";   // No State Change Needed
    }
  }else if(cmd == "resetmem"){
    returnVal = String(resetMemory());
  }else if(cmd == "restart"){
    ESP.restart();
    returnVal = "true";
  }
  return returnVal;
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
  deviceJson["topology"] = mesh.subConnectionJson();
  JsonArray& nodeList = deviceJson.createNestedArray("Node_LIST");
  for(uint32_t nId : mesh.getNodeList()){
    nodeList.add(nId);
  }
  deviceJson.printTo(*response);
  request->send(response);
}

void fireBroadCastMessage(AsyncWebServerRequest *request, int receivedState){
  String message;
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["targetNodeId"] = "all";
  root["command"] = "switch";
  root["state"] = receivedState;
  root.printTo(message);
  root["success"] = mesh.sendBroadcast(message, true);
  root.printTo(*response);
  request->send(response);
}

// void sendMessage() {
//   String msg = "HeartBeat from node ";
//   msg += mesh.getNodeId();
//   mesh.sendBroadcast( msg );
//   taskSendMessage.setInterval(TASK_SECOND * 10);
// }	

void handleReceivedSignal(uint32_t from, String &msg){
  String returnVal;
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& reqJsonObj = jsonBuffer.parseObject(msg);
	int tempState = reqJsonObj["state"];
	if(reqJsonObj["command"] == "switch"){
		if(digitalRead(myControlPin) != tempState){
      returnVal = "true";
			digitalWrite(myControlPin, tempState);
		}else{
      returnVal = "NSCN";
		}
    mesh.sendSingle(from, "{\"targetNodeId\":"+String(myNodeId)+",\"command\":signal_response,\"state\":"+String(tempState)+",\"success\":"+returnVal+"}");
	}else if(reqJsonObj["command"] == "signal_response"){
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& resJsonObj = jsonBuffer.parseObject(msg);
      resJsonObj.printTo(Serial); 
      // Need to enhance this block to send back http response.;
  }else if(reqJsonObj["command"] == "resetmem"){
      returnVal = String(resetMemory());
      mesh.sendSingle(from, "{\"targetNodeId\":"+String(myNodeId)+",\"command\":signal_response,\"state\":ResetMem,\"success\":"+returnVal+"}");
  }else if(reqJsonObj["command"] == "restart"){
      ESP.restart();
  }else{
		println("Unknown Command Received");
	}
}


void println(String content){
  if(DEBUG_MODE)
    Serial.println(content);
}
