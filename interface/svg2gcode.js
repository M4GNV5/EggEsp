const OP_MOVE = 1;
const OP_MOVE_REL = 2;
const OP_LINE = 3;
const OP_LINE_REL = 4;
//...
function isRelative(op)
{
	return op % 2 == 0;
}

const xyOps = {
	"M": OP_MOVE,
	"m": OP_MOVE_REL,
	"L": OP_LINE,
	"l": OP_LINE_REL
};
const xy0Ops = {
	"V": {value: "y", unchanged: "x", op: OP_LINE},
	"v": {value: "y", unchanged: "x", op: OP_LINE_REL},
	"H": {value: "x", unchanged: "y", op: OP_LINE},
	"h": {value: "x", unchanged: "y", op: OP_LINE_REL}
};

function svg2gcode(transform, elements)
{
	let ret = [];
	for(var curr of elements)
	{
		let _transform;
		if(curr.attributes && curr.attributes.transform)
			_transform = parseTransform(transform, curr.attributes.transform.value);
		else
			_transform = transform;

		switch(curr.tagName)
		{
			case "g":
				ret.push(svg2gcode(_transform, Array.from(curr.children)));
				break;

			case "path":
				ret.push(path2gcode(_transform, curr.attributes.d.value));
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
	if(!path)
		return [];

	let split = path.split(/ |,/);
	let ret = [];

	let startX = 0;
	let startY = 0;
	let x = 0;
	let y = 0;
	let wasRelative = false;

	while(split.length > 0)
	{
		let curr = split.shift();
		let op;
		if(xyOps.hasOwnProperty(curr))
		{
			op = {
				op: xyOps[curr],
				x: parseFloat(split.shift()),
				y: parseFloat(split.shift())
			};
		}
		else if(xy0Ops.hasOwnProperty(curr))
		{
			let xy0Op = xy0Ops[curr];
			op = {op: xy0Op.op};
			op[xy0Op.value] = parseFloat(split.shift());

			if(isRelative(op.op))
				op[xy0Op.unchanged] = 0;
			else
				op[xy0Op.unchanged] = xy0Op.unchanged == "x" ? x : y;
		}
		else if(curr == "z" || curr == "Z")
		{
			op = {
				op: OP_LINE,
				x: startX,
				y: startY
			};
		}
		//TODO other ops (see https://svgwg.org/specs/paths/#PathElement)
		else if(!isNaN(curr)) //assume L/l by default
		{
			op = {
				op: wasRelative ? OP_LINE_REL : OP_LINE,
				x: parseFloat(curr),
				y: parseFloat(split.shift())
			};
		}
		else
		{
			throw "Invalid/Unsupported svg path character " + curr;
		}

		wasRelative = isRelative(op.op);

		if(ret.length == 0 && wasRelative)
		{
			op.op = op.op == OP_LINE_REL ? OP_LINE : OP_MOVE;
		}

		if(wasRelative)
		{
			x += op.x;
			y += op.y;

			op.op = op.op == OP_LINE_REL ? OP_LINE : OP_MOVE;
			op.x = x;
			op.y = y;
		}
		else
		{
			x = op.x;
			y = op.y;
		}

		if(op.op == OP_MOVE)
		{
			startX = x;
			startY = y;
		}

		applyTransformation(transform, op);
		ret.push(op);
	}

	return ret;
}

function parseTransform(transform, text)
{
	let split = text.split(/ |,|\(|\)/);
	split = split.filter(x => x.trim() != "");

	while(split.length > 0)
	{
		//array in the form of [a, b, c, d, e, f] corresponding to the transformation matrix
		// a b c
		// d e f
		// 0 0 1
		let matrix = null;
		let curr = split.shift();

		//see https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/transform
		let x, y, a;
		switch(curr)
		{
			case "matrix":
				matrix = split.splice(0, 6);
				matrix = matrix.map(parseFloat);
				matrix = [matrix[0], matrix[2], matrix[4], matrix[1], matrix[3], matrix[5]];
				break;

			case "translate":
				x = parseFloat(split.shift());
				y = isNaN(split[0]) ? 0 : parseFloat(split.shift());
				matrix = [1, 0, x, 0, 1, y];
				break;

			case "scale":
				x = parseFloat(split.shift());
				y = isNaN(split[0]) ? x : parseFloat(split.shift());
				matrix = [x, 0, 0, 0, y, 0];
				break;

			case "rotate":
				a = parseFloat(split.shift());
				if(!isNaN(split[0]))
					throw "Rotation around a point not supported";
				matrix = [Math.cos(a), -Math.sin(a), 0, Math.sin(a), Math.cos(a), 0];
				break;

			case "skewX":
				a = parseFloat(split.shift());
				matrix = [1, Math.tan(a), 0, 0, 1, 0];
				break;

			case "skewY":
				a = parseFloat(split.shift());
				matrix = [1, 0, 0, Math.tan(a), 1, 0];
				break;

			default:
				throw "Unknown transformation " + curr;
		}

		transform = {matrix: matrix, calculated: null, prev: transform};
	}

	return transform;
}

//not a generic implementation, specific for 2 3x3 matrices with 0, 0, 1 as the last row
function matmul(a, b)
{
	let ret = [];
	for(let i = 0; i < 6; i += 3)
	{
		for(let j = 0; j < 3; j++)
		{
			ret[i + j] = a[i] * b[j] + a[i + 1] * b[j + 3] + a[i + 2] * (j == 2 ? 1 : 0);
		}
	}

	return ret;
}
function applyTransformation(transform, op)
{
	if(!transform)
		return;

	if(!transform.calculated)
	{
		let result = [1, 0, 0, 0, 1, 0];
		let curr = transform;
		while(curr)
		{
			result = matmul(curr.matrix, result);
			curr = curr.prev;
		}

		transform.calculated = result;
	}

	op.x = transform.calculated[0] * op.x + transform.calculated[1] * op.y + transform.calculated[2];
	op.y = transform.calculated[3] * op.x + transform.calculated[4] * op.y + transform.calculated[5];
}
