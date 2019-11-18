/**
 * 
 */

var request = new XMLHttpRequest();
request.open("GET", "/bleeper", false); // false for synchronous request
request.send(null);
var allconfig = JSON.parse(request.responseText);

var configitems = document.getElementsByClassName("bleeper");

for (var c = 0; c != configitems.length; c++) {
	var value = allconfig[configitems[c].name];
	switch (configitems[c].tagName.toLowerCase()) {
	case "input":
	case "textarea":
		var value = allconfig[configitems[c].name];
		configitems[c].value = value;
		break;
	case "select":
		var value = allconfig[configitems[c].name + "_list"];
		var options = JSON.parse(value);
		for(var key in options) {
			var optionNode = new Option(options[key], key);
			if(key == allconfig[configitems[c].name]) 
				optionNode.selected = true;
			configitems[c].appendChild(optionNode);			
		}
		break;
	default:
		console.log("Unrecognized element type:");
		console.log(configitems[c]);
		break;
	}
}
