#include "Arduino.h"
#include "Memory.h"


Memory::Memory(): EEPROMClass(){
	begin(512);
	println("Initializing EEPROM");
}


bool Memory::isCredsAvailable(){
	// 0th position in memory will hold data which describes whether ssid is configured or not.
	return read(0)==1;
}

bool Memory::commitToEEPROM(){
	bool commitStatus = commit();
	if(commitStatus){
		println("Commit Success");
	}else{
		println("Commit Failed");
	}
	return commitStatus;
}

bool Memory::setCredsState(char state){
	println("Commiting Credentials Status: "+String(int(state)));
	put(0, state);
	return commitToEEPROM();
}
bool Memory::setMeshSSID(String ssid){
	println("Commiting Mesh SSID: "+ssid);
	for(int i=0; i<32; i++){
		put(i+1, ssid[i]);
	}
	return commitToEEPROM();
}

bool Memory::setMeshPSWD(String pswd){
	println("Commiting Mesh Password: "+pswd);
	for(int i=0; i<32; i++){
		put(i+33, pswd[i]);
	}
	return commitToEEPROM();
}

bool Memory::setWiFiSSID(String ssid){
	println("Commiting WiFi SSID: "+ssid);
	for(int i=0; i<32; i++){
		put(i+65, ssid[i]);
	}
	return commitToEEPROM();
}

bool Memory::setWiFiPSWD(String pswd){
	println("Commiting WiFi Password: "+pswd);
	for(int i=0; i<32; i++){
		put(i+97, pswd[i]);
	}
	return commitToEEPROM();
}

String Memory::getMeshSSID(){
	String ssid;
	for(int i=1; i<33; i++){
		ssid +=char(read(i));
	}
	println("Mesh SSID: "+ssid);
	return ssid.c_str();
}

String Memory::getMeshPSWD(){
	String pswd;
	for(int i=33; i<65; i++){
		pswd +=char(read(i));
	}
	println("Mesh Password: "+pswd);
	return pswd.c_str();
}

String Memory::getWiFiSSID(){
	String ssid;
	for(int i=65; i<97; i++){
		ssid +=char(read(i));
	}
	println("WiFi SSID: "+ssid);
	return ssid.c_str();
}

String Memory::getWiFiPSWD(){
	String pswd;
	for(int i=97; i<129; i++){
		pswd += char(read(i));
	}
	println("WiFi Password: "+pswd);
	return pswd.c_str();
}

bool Memory::resetMemory(){
	println("Resetting Memory");
	for(int i=0; i<length(); i++){
		put(i, 0);
	}
	return commitToEEPROM();
}

bool Memory::isBridge(){
	return read(131)==1;
}

bool Memory::setBridge(char state){
	println("Commiting Bridge Status: "+String(int(state)));
	put(131, state);
	return commitToEEPROM();
}

String Memory::getCredentials(){
	// Need  to implement json string which returns credentials
}

void Memory::println(String content){
	Serial.println(content);
}
