const char * homePage =" <!DOCTYPE html>"
"<html>"
	"<body>"
		"<h1>HELLO WORLD !!</h1>"
    "</br>"
		"<a href='/getdeviceinfo'>Get Device Info</a>"
	"</body>"
"</html>";


const char * meshConfigForm = "<!DOCTYPE html>"
"<html>"
	"<body>"
		"<form method='POST' action='/config'>"
			"<input id='MeshSSID' name='MeshSSID' length=32 placeholder='Mesh SSID' required>"
			"<br/>"
			"<input id='MeshPSWD' name='MeshPSWD' length=32 type='password' placeholder='Mesh Password' required>"
			"<br/>"
			"<input id='WiFiSSID' name='WiFiSSID' length=32 placeholder='WiFi SSID'>"
			"<br/>"
			"<input id='WiFiPSWD' name='WiFiPSWD' length=32 type='password' placeholder='WiFi Password'>"
			"<br/>"
			"<br/>"
			"<button type='submit'>Save</button>"
		"</form>"
   "<br/>"
   "<input type='button' onClick=\"window.location.href='/resetmem'\" value='Reset Memory'>"
	"</body>"
"</html>";

String configSuccess(){
	const char * configSuccess = "<!DOCTYPE html>"
	"<html>"
		"<body>"
			"<p style='color:green;'>Configuration Success</p>"
			"<br/>"
			"<p>Restart this node to connect to your cluster</p>"
			"<br/>"
      "<form action='/restart' method='POST'>"
        "<input type='submit' value='Restart' />"
      "</form>"
		"</body>"
	"</html>";
	return String(configSuccess);
}


String configFailed(){
	const char * configFailed = "<!DOCTYPE html>"
	"<html>"
		"<body>"
			"<p style='color:red;'>Configuration Failed</p>"
			"<br/>"
			"<p>GoTo home Page to start configuration again.</p>"
			"<br/>"
			"<input type='button' onclick=\"location.href='/';\" value='Home' />"
		"</body>"
	"</html>";
	return String(configFailed);
}




