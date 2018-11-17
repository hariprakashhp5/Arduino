baseUrl=window.origin!="null" ? window.origin+"/" : "http://192.168.43.208:3030/"
console.log(baseUrl);
window.onload = function getTopology() {
	getRequest(baseUrl+"getdeviceinfo", getNodes);
}

function getNodes(response){
	var jsObj =JSON.parse(response);
	var parent = jsObj['NodeId'];
	createRow(parent);
	var nodes = jsObj['Node_LIST'];
	for(var i=0;i<nodes.length;i++){
		createRow(nodes[i]);
	}
}

function createRow(nodeId){
	var table = document.getElementById('devices')
	var row = document.createElement('tr')
	for(i=0; i<6; i++){
		var cell = document.createElement('td')
		switch(i){
			case 0:
		  		cell.innerHTML=nodeId;
				row.append(cell);
				break;
			case 2:
		  		cell.appendChild(createButton("switchOn", nodeId));
				row.append(cell);
				break;
			case 3:
		  		cell.appendChild(createButton("switchOFF", nodeId));
				row.append(cell);
				break;
			case 4:
				cell.appendChild(createButton("restart", nodeId));
				row.append(cell);
				break;
			case 5:
				cell.appendChild(createButton("resetMem", nodeId));
				row.append(cell);
				break;
			case 1:
				cell.innerHTML="STATUS";
				row.append(cell);
				break;
		}
	}
	table.append(row)
}

var btnVal={'switchOn': "ON",
		'switchOFF' : 'OFF',
		'resetMem' : 'Reset Mem',
		'restart' : 'Restart'
}

var clsVal={'switchOn': "butn on",
		'switchOFF' : 'butn off',
		'resetMem' : 'butn reset',
		'restart' : 'butn restart'
}

function createButton(callback, data=""){
	var id = document.createAttribute('id');
	id.value='btn';
	var cl = document.createAttribute('class');
	cl.value = clsVal[callback];
	var onclick = document.createAttribute('onClick');
	onclick.value=callback+"("+data+")";
	var btn = document.createElement('button');
	btn.innerHTML=btnVal[callback];
	btn.setAttributeNode(id);
	btn.setAttributeNode(cl);
	btn.setAttributeNode(onclick);
	return btn
}

function switchOn(nodeId){
	data="targetNodeId="+nodeId+"&state=1"
	console.log("Switch On Node: "+nodeId);
	postRequest(baseUrl+"switch", data, handleRes);
}

function switchOFF(nodeId){
	data="targetNodeId="+nodeId+"&state=0"
	console.log("Switch OFF Node: "+nodeId);
	postRequest(baseUrl+"switch", data, handleRes);
}

function switchAll(state){
	console.log("Switching "+state);
	getRequest(baseUrl+state, handleRes);
}

function restart(nodeId){
	if(window.confirm('Are you sure to Restart Node?')){
		data="targetNodeId="+nodeId
		console.log("Restarting node: "+nodeId);
		postRequest(baseUrl+"restart", data, handleRes);
	}
}

function resetMem(nodeId){
	if(window.confirm('Are you sure to reset memory?')){
		data="targetNodeId="+nodeId
		console.log("Resetting Memory on node: "+nodeId);
		postRequest(baseUrl+"resetmem", data, handleRes);
	}
}

function handleRes(response){
	console.log(response)
}

function getRequest(url,callback){
	console.log("Requesting for resource...")
	var xhttp = new XMLHttpRequest();
	xhttp.open("GET", url, true);
	xhttp.setRequestHeader( 'Access-Control-Allow-Origin', '*');
	xhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			console.log("Done...")
	       	callback(xhttp.responseText);
	    }
	};
	xhttp.send();
}

function postRequest(url, data, callback){
	console.log("Sending post request...")
	var xhttp = new XMLHttpRequest();
	xhttp.open("POST", url, true);
	xhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
	xhttp.setRequestHeader( 'Access-Control-Allow-Origin', '*');
	xhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			console.log("Done...")
	       	callback(xhttp.responseText);
	    }
	};
	xhttp.send(data);
}