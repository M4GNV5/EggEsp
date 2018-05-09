#pragma once

class Draw2D
{
private:
	Stepper& x;
	Stepper& y;
	int16_t posX;
	int16_t posY;
	void (*setDrawing)(bool);

	void rawMove(int16_t dx, int16_t dy)
	{
		if(dx == 0 && dy == 0)
		{
			return;
		}
		/*else if(dx == 0)
		{
			y.step(dy);
			posY += dy;
			return;
		}
		else if(dy == 0)
		{
			x.step(dx);
			posX += dx;
			return;
		}*/

		posX += dx;
		posY += dy;

		bool xReverse = dx < 0 ? true : false;
		bool yReverse = dy < 0 ? true : false;
		if(dx < 0)
			dx = -dx;
		if(dy < 0)
			dy = -dy;

		Stepper *a;
		Stepper *b;
		int16_t da;
		int16_t db;
		bool aReverse;
		bool bReverse;

		if(dx < dy)
		{
			a = &y;
			b = &x;
			da = dy;
			db = dx;
			aReverse = yReverse;
			bReverse = xReverse;
		}
		else
		{
			a = &x;
			b = &y;
			da = dx;
			db = dy;
			aReverse = xReverse;
			bReverse = yReverse;
		}

		//Bresenham algorithm
		//constrains: 0 <= db/da <= 1 and da > 0 (thats why we put the smaller one in 'b' above)

		int16_t D = 2 * db - da;
		int16_t deltaE = 2 * db;
		int16_t deltaNE = 2 * (db - da);
		while(da > 0)
		{
			if(D < 0)
			{
				D += deltaE;
			}
			else
			{
				D += deltaNE;
				b->singleStep(bReverse);
			}

			a->singleStep(aReverse);
			da--;
		}
	}

public:
	Draw2D(Stepper& _x, Stepper& _y,
		int16_t _posX, int16_t _posY,
		void (*switchFunc)(bool)
	)
		: x(_x), y(_y), posX(_posX), posY(_posY), setDrawing(switchFunc)
	{

	}

	void moveTo(int16_t x, int16_t y)
	{
		setDrawing(false);
		rawMove(x - posX, y - posY);
	}
	void move(int16_t dx, int16_t dy)
	{
		setDrawing(false);
		rawMove(dx, dy);
	}

	void lineTo(int16_t x, int16_t y)
	{
		setDrawing(true);
		rawMove(x - posX, y - posY);
	}
	void line(int16_t dx, int16_t dy)
	{
		setDrawing(true);
		rawMove(dx, dy);
	}

	//TODO arcTo

	//TODO curveTo
};
