
bool isCredsAvailable(){
	// 0th position in memory will hold data which describes whether ssid is configured or not.
	return EEPROM.read(0)==1;
}


bool commitToEEPROM(){
	bool commitStatus = EEPROM.commit();
	if(commitStatus){
		println("Commit Success");
	}else{
		println("Commit Failed");
	}
	return commitStatus;
}

bool setCredsState(char state){
	println("Commiting Credentials Status: "+String(state));
	EEPROM.write(0, state);
	return commitToEEPROM();
}
bool setMeshSSID(String ssid){
	println("Commiting Mesh SSID: "+ssid);
	for(int i=0; i<32; i++){
		EEPROM.write(i+1, ssid[i]);
	}
	return commitToEEPROM();
}

bool setMeshPSWD(String pswd){
	println("Commiting Mesh Password: "+pswd);
	for(int i=0; i<32; i++){
		EEPROM.write(i+33, pswd[i]);
	}
	return commitToEEPROM();
}

bool setWiFiSSID(String ssid){
	println("Commiting WiFi SSID: "+ssid);
	for(int i=0; i<32; i++){
		EEPROM.write(i+65, ssid[i]);
	}
	return commitToEEPROM();
}

bool setWiFiPSWD(String pswd){
	println("Commiting WiFi Password: "+pswd);
	for(int i=0; i<32; i++){
		EEPROM.write(i+97, pswd[i]);
	}
	return commitToEEPROM();
}

String getMeshSSID(){
	String ssid;
	for(int i=1; i<33; i++){
		ssid +=char(EEPROM.read(i));
	}
	println("Mesh SSID: "+ssid);
	return ssid.c_str();
}

String getMeshPSWD(){
	String pswd;
	for(int i=33; i<65; i++){
		pswd +=char(EEPROM.read(i));
	}
	println("Mesh Password: "+pswd);
	return pswd.c_str();
}

String getWiFiSSID(){
	String ssid;
	for(int i=65; i<97; i++){
		ssid +=char(EEPROM.read(i));
	}
	println("WiFi SSID: "+ssid);
	return ssid.c_str();
}

String getWiFiPSWD(){
	String pswd;
	for(int i=97; i<129; i++){
		pswd += char(EEPROM.read(i));
	}
	println("WiFi Password: "+pswd);
	return pswd.c_str();
}

bool resetMemory(){
	println("Resetting Memory on Node: "+String(myNodeId));
	for(int i=0; i<EEPROM.length(); i++){
		EEPROM.write(i, 0);
	}
	return commitToEEPROM();
}
