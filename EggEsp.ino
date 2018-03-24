#include <Servo.h>
#include "./Stepper.hpp"
#include "./Draw2D.hpp"

void togglePen(bool);

#define PEN_PIN D10
#define PEN_DRAWING 110
#define PEN_MOVING 140
bool penStatus;
Servo pen;

Stepper x(D0, D2, D1, D3);
Stepper y(D5, D7, D6, D8);
Draw2D draw(x, y, 0, 0, togglePen);

void togglePen(bool status)
{
	if(status != penStatus)
	{
		uint8_t angle, target, step;
		if(penStatus)
		{
			angle = PEN_DRAWING;
			target = PEN_MOVING;
			step = 2;
		}
		else
		{
			angle = PEN_MOVING;
			target = PEN_DRAWING;
			step = -2;
		}

		while(angle != target)
		{
			angle += step;
			pen.write(angle);
			delay(50);
		}

		penStatus = status;
	}
}

void setup()
{
	Serial.begin(115200);

	Serial.println("Hello!");
	delay(5000);

	Serial.println("Initializing servo");
	pen.attach(PEN_PIN, 1000, 2000);
	pen.write(PEN_MOVING);
	penStatus = false;

	Serial.println("Initializing steppers");
	x.init();
	x.setSpeed(100);
	y.init();
	y.setSpeed(100);
}

void loop()
{
	draw.lineTo(666, -30);
	draw.moveTo(-100, 500);

	delay(3000);
}
