function loadFile()
{
	var fileInput = document.getElementById("fileInput");
	var file = fileInput.files[0];

	var reader = new FileReader();
	reader.onload = function(e)
	{
		document.getElementById("image").innerHTML = e.target.result;
	};

	reader.readAsText(file);
}

function genGcode()
{
	var nodes = Array.from(document.getElementsByTagName("svg")[0].children);
	var ops = svg2gcode(null, nodes);

	//TODO send to esp
	console.dir(ops);
}
