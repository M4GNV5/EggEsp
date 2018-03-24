#pragma once

const static uint8_t controls[4] = {
	0b1010,
	0b0110,
	0b0101,
	0b1001
};

class Stepper
{
private:
	uint8_t pins[4];
	uint8_t currStep = 0;

	uint32_t lastStep = 0;
	uint32_t stepInterval = 1000;

	void doStep(bool reverse)
	{
		if(reverse)
		{
			if(currStep == 0)
				currStep = 4;
			currStep--;
		}
		else
		{
			currStep++;
			if(currStep >= 4)
				currStep = 0;
		}

		uint8_t control = controls[currStep];

		for(uint8_t i = 0; i < 4; i++)
			digitalWrite(pins[i], control & (1 << i));
	}

public:

	Stepper(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3)
	{
		pins[0] = pin0;
		pins[1] = pin1;
		pins[2] = pin2;
		pins[3] = pin3;
	}

	void init()
	{
		uint8_t control = controls[currStep];
		for(uint8_t i = 0; i < 4; i++)
		{
			pinMode(pins[i], OUTPUT);
			digitalWrite(pins[i], control & (1 << i));
		}
	}

	void setSpeed(uint16_t stepsPerSecond)
	{
		stepInterval = (uint32_t)1000 / stepsPerSecond;
	}

	void step(int32_t n)
	{
		bool reverse = false;
		if(n < 0)
		{
			n = -n;
			reverse = true;
		}

		while(n > 0)
		{
			uint32_t now = millis();
			while(lastStep + stepInterval > now)
			{
				yield();
				now = millis();
			}

			doStep(reverse);
			lastStep = now;
			n--;
		}
	}

	void singleStep(bool reverse)
	{
		uint32_t now = millis();
		while(lastStep + stepInterval > now)
		{
			yield();
			now = millis();
		}

		doStep(reverse);
		lastStep = now;
	}
};
