#pragma once

class Draw2D
{
private:
	Stepper& x;
	Stepper& y;
	int16_t posX;
	int16_t posY;
	bool drawing;
	void (*switchFunc)(bool);

	void move(int16_t dx, int16_t dy)
	{
		bool xReverse = dx < 0 ? true : false;
		bool yReverse = dy < 0 ? true : false;

		posX += dx;
		posY += dy;

		if(dx < 0)
			dx = -dx;
		if(dy < 0)
			dy = -dy;

		//TODO evenly distribute the smaller amount steps along the larger amount

		while(dx > 0 || dy > 0)
		{
			if(dx > 0)
			{
				x.singleStep(xReverse);
				dx--;
			}
			if(dy > 0)
			{
				y.singleStep(yReverse);
				dy--;
			}
		}
	}

public:
	Draw2D(Stepper& _x, Stepper& _y,
		int16_t _posX, int16_t _posY,
		void (*_switch)(bool)
	)
		: x(_x), y(_y), posX(_posX), posY(_posY), switchFunc(_switch)
	{

	}

	inline void setDrawing(bool state)
	{
		if(drawing != state)
		{
			switchFunc(state);
			drawing = state;
		}
	}

	void moveTo(int16_t x, int16_t y)
	{
		setDrawing(false);
		move(x - posX, y - posY);
	}
	void moveToRel(int16_t dx, int16_t dy)
	{
		setDrawing(false);
		move(dx, dy);
	}

	void lineTo(int16_t x, int16_t y)
	{
		setDrawing(true);
		move(x - posX, y - posY);
	}
	void lineToRel(int16_t dx, int16_t dy)
	{
		setDrawing(true);
		move(dx, dy);
	}

	//TODO arcTo

	//TODO curveTo
};
