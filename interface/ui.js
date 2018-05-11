var statusEl = document.getElementById("status");

function loadFile()
{
	var fileInput = document.getElementById("fileInput");
	var file = fileInput.files[0];

	var reader = new FileReader();
	reader.onload = function(e)
	{
		document.getElementById("image").innerHTML = e.target.result;

		let attributes = document.getElementsByTagName("svg")[0].attributes;
		document.getElementById("svgDimensions").innerHTML = attributes.width.value + "x" + attributes.height.value;
	};
	reader.onerror = function()
	{
		statusEl.innerHTML = "Failed reading the file";
	};

	reader.readAsText(file);
}

function genGcode()
{
	let svg = document.getElementsByTagName("svg")[0];

	let ops;
	try
	{
		ops = svg2gcode(null, Array.from(svg.children));
		statusEl.innerHTML = "Generated " + ops.length + " instructions";

		optimizePaths(ops);

		transformCoordinates(ops, svg.attributes.width.value, svg.attributes.height.value);
		console.dir(ops);
		ops = opsToBytecode(ops);
	}
	catch(e)
	{
		statusEl.innerHTML = e.toString();
	}

	let a = document.createElement("a");
	a.style = "display: none";
	document.body.appendChild(a);
	let url = window.URL.createObjectURL(new Blob([ops]));
	a.href = url;
	a.download = "eggbot.command";
	a.click();
	window.URL.revokeObjectURL(url);
}
