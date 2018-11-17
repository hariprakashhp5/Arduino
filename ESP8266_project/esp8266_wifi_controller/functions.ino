void handleRequest(AsyncWebServerRequest *request){
  if(request->url().endsWith(".css")){
    request->send(SPIFFS, "/generic.css");
  }else if(request->url().endsWith(".js")){
    request->send(SPIFFS, "/jsfunctions.js");
  }else if(request->method() == HTTP_OPTIONS){
    request->send(200);
  }else{
    request->send(404, "text/html", "<h1>404: Not found</h1>");
  }
}

void setupWifiParameters(AsyncWebServerRequest *request){
	bool status=false;
	if(request->hasParam("MeshSSID", true) && request->hasParam("MeshPSWD", true)){
		String meshSSID = request->getParam("MeshSSID", true)->value();
		String meshPSWD = request->getParam("MeshPSWD", true)->value();
		String wifiSSID = request->getParam("WiFiSSID", true)->value();
		String wifiPSWD = request->getParam("WiFiPSWD", true)->value();
    String bridge = request->getParam("bridge", true)->value();
    if(bridge=="true"){
      Mem.setBridge(1);
    }else{
      Mem.setBridge(0);
    }
    if(wifiSSID!="" && wifiPSWD!=""){
      Mem.setWiFiSSID(wifiSSID);
      Mem.setWiFiPSWD(wifiPSWD); 
    }
		status = Mem.setMeshSSID(meshSSID) && Mem.setMeshPSWD(meshPSWD);
	}
	if(status){
		Mem.setCredsState(1);
		request->send(SPIFFS, "/configSuccess.html");
	}else{
		Mem.setCredsState(0);
		request->send(SPIFFS, "/configFailed.html");
	}
}

// Need to improvise this method
void sendResponse(AsyncWebServerRequest *request, String cmd){
  String message;
    if(cmd=="switch" && request->hasParam("targetNodeId", true) && request->hasParam("state", true)){
      targetNode.id = atoi (request->getParam("targetNodeId", true)->value().c_str());
      targetNode.pinState = request->getParam("state", true)->value().toInt();
      message = sendSignal(cmd);
    }else if((cmd=="restart" ||cmd=="resetmem") && request->hasParam("targetNodeId", true)){
      targetNode.id = atoi (request->getParam("targetNodeId", true)->value().c_str());
      message = sendSignal(cmd);
    }else{
      message = debugRequest(request);
    }
    request->send(200, "text/json", message);
}

String sendSignal(String cmd){
	String strResponse;
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& jsonObj = jsonBuffer.createObject();
  jsonObj["sourceNodeId"] = thisNode.id;
  jsonObj["targetNodeId"] = targetNode.id;
	jsonObj["command"] = cmd;
	jsonObj["state"] = targetNode.pinState;
	jsonObj.printTo(strResponse);
  if(targetNode.id != thisNode.id){
    jsonObj["trigger"] = mesh.sendSingle(targetNode.id, strResponse);
  }else{
    println("thisNode is the targetNode");
    thisNode.pinState = targetNode.pinState;
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
    if(digitalRead(thisNode.controlPin) != thisNode.pinState){
      digitalWrite(thisNode.controlPin, thisNode.pinState);
      returnVal = "true";
    }else{
      returnVal = "NSCN";   // No State Change Needed
    }
  }else if(cmd == "resetmem"){
    returnVal = String(Mem.resetMemory());
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
  deviceJson["NodeId"] = thisNode.id;
  deviceJson["NodeIp"] = thisNode.apIp.toString();
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


void handleReceivedSignal(uint32_t from, String &msg){
  String returnVal;
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& reqJsonObj = jsonBuffer.parseObject(msg);
  if(reqJsonObj["command"] == "switch"){
	  thisNode.pinState = reqJsonObj["state"];
		if(digitalRead(thisNode.controlPin) != thisNode.pinState){
			digitalWrite(thisNode.controlPin, thisNode.pinState);
      returnVal = "true";
		}else{
      returnVal = "NSCN";
		}
    mesh.sendSingle(from, "{\"targetNodeId\":"+String(thisNode.id)+",\"command\":signal_response,\"state\":"+String(thisNode.pinState)+",\"success\":"+returnVal+"}");
	}else if(reqJsonObj["command"] == "signal_response"){
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& resJsonObj = jsonBuffer.parseObject(msg);
      resJsonObj.printTo(Serial); 
      // Need to enhance this block to send back http response.;
  }else if(reqJsonObj["command"] == "resetmem"){
      returnVal = String(Mem.resetMemory());
      mesh.sendSingle(from, "{\"targetNodeId\":"+String(thisNode.id)+",\"command\":signal_response,\"state\":ResetMem,\"success\":"+returnVal+"}");
  }else if(reqJsonObj["command"] == "restart"){
      ESP.restart();
  }else{
		println("Unknown Command Received");
	}
}


void println(String content){
  if(DEBUG_MODE)
    Serial.println("["+String(thisNode.id)+"] - "+content);
}
