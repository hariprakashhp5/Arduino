void initBasicRoutes(){
	server.onNotFound([](AsyncWebServerRequest *request){
		handleRequest(request);
	});

  server.on("/resetmem", HTTP_POST,[](AsyncWebServerRequest *request){
    println("[POST] - Received request to Reset Memory.");
    sendResponse(request, "resetmem");
  });

  server.on("/restart", HTTP_POST,[](AsyncWebServerRequest *request){
   	println("[POST] - Received request to Restart");
   	sendResponse(request,"restart");
  });
 
}

void initConfigRoutes(){
	server.on("/", HTTP_GET,[](AsyncWebServerRequest *request){
		println("[GET] - Received request for Config Home Page.");
		request->send(SPIFFS, "/config.html");
	});

	server.on("/config", HTTP_POST,[](AsyncWebServerRequest *request){
		println("[POST] - Received Data to Setup Node.");
		setupWifiParameters(request);
	});
}



void initMainRoutes(){
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		println("[GET] - Received request for Home Page");
		request->send(SPIFFS, "/index.html");
	});

  server.on("/devices", HTTP_GET, [](AsyncWebServerRequest *request){
    println("[GET] - Received request for Devices Page");
    request->send(SPIFFS, "/devices.html");
  });

	server.on("/getdeviceinfo", HTTP_GET, [](AsyncWebServerRequest *request){
	  println("[GET] - Received request to get device information");
	  sendDeviceInfo(request);
	});

	server.on("/switch", HTTP_POST, [](AsyncWebServerRequest *request){
		println("[POST] - Received request to change state of a Switch");
		sendResponse(request, "switch");
	});

	server.on("/allon", HTTP_GET, [](AsyncWebServerRequest *request){
		println("[GET] - Received request to change state of all connected switchs to HIGH");
		fireBroadCastMessage(request, 1);
	});

	server.on("/alloff", HTTP_GET, [](AsyncWebServerRequest *request){
		println("[GET] - Received request to change state of all connected switchs to LOW");
		fireBroadCastMessage(request, 0);
	});	
}


