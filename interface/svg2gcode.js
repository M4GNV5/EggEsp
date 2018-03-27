const OP_MOVE = 1;
const OP_MOVE_REL = 2;
const OP_LINE = 3;
const OP_LINE_REL = 4;
//...

const xyOps = {
	"m": OP_MOVE,
	"M": OP_MOVE_REL,
	"l": OP_LINE,
	"L": OP_LINE_REL
};

function svg2gcode(transform, elements)
{
	let ret = [];
	for(var curr of elements)
	{
		switch(curr.tagName)
		{
			case "g":
				let _transform;
				if(curr.attributes && curr.attributes.transform)
				{
					let matrix = parseTransform(curr.attributes.transform.value);
					_transform = {matrix: matrix, calculated: null, next: transform};
				}
				else
				{
					_transform = transform;
				}

				ret.push(svg2gcode(_transform, Array.from(curr.children)));
				break;

			case "path":
				ret.push(path2gcode(transform, curr.attributes.d.value));
				break;

			//TODO rect, polygon, ...

			default:
				//ignore
				//TODO handle child nodes?
				break;
		}
	}

	let ret2 = [];
	ret.forEach(x => ret2.push(...x));
	return ret2;
}

function path2gcode(transform, path)
{
	let split = path.split(/ |,/);
	let ret = [];

	for(let i = 0; i < 100 && split.length > 0; i++)
	{
		let curr = split.shift();
		if(xyOps.hasOwnProperty(curr))
		{
			let op = {
				op: xyOps[curr],
				x: parseFloat(split.shift()),
				y: parseFloat(split.shift())
			};

			//TODO apply transform to x,y

			ret.push(op);
		}
		//TODO other ops (see https://svgwg.org/specs/paths/#PathElement)
		else if(!isNaN(curr)) //assume L by default
		{
			let op = {
				op: OP_LINE_REL,
				x: parseFloat(curr),
				y: parseFloat(split.shift())
			};

			//TODO apply transform to x,y

			ret.push(op);
		}
		else
		{
			throw "Invalid/Unsupported svg path character " + curr;
		}
	}

	return ret;
}

function parseTransform(text)
{
	//TODO
	return [0, 0, 0, 0, 0, 0];
}
