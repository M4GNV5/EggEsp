//eggbot dimensions
const targetWidth = 2048;
const targetHeight = 512;

function transformCoordinates(ops, sourceWidth, sourceHeight)
{
	for(op of ops)
	{
		op.x = op.x * targetWidth / sourceWidth;
		op.y = op.y * targetHeight / sourceHeight;
	}
}

function opsToBytecode(ops)
{
	let buff = new ArrayBuffer(ops.length * 8 + 2);
	let view = new DataView(buff);

	view.setInt16(0, ops.length, true);

	for(let i = 0; i < ops.length; i++)
	{
		let off = 2 + i * 8;
		view.setUint8(off, ops[i].op, true);
		view.setInt16(off + 2, ops[i].x, true);
		view.setInt16(off + 4, ops[i].y, true);
	}

	return buff;
}
