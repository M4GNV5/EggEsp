#include <ESP8266WiFi.h>
#include <Servo.h>
#include "./Stepper.hpp"
#include "./Draw2D.hpp"
#include "./config.h"

#ifdef DEBUG
#define LOG(msg) Serial.print(msg)
#define LOGLN(msg) Serial.println(msg)
#else
#define LOG(msg)
#define LOGLN(msg)
#endif

#ifndef USE_WIFI
#define input Serial
#endif

#define OP_MOVE 1
#define OP_MOVE_REL 2
#define OP_LINE 3
#define OP_LINE_REL 4

struct header
{
	uint16_t instruction_count;
	//...
} __attribute__((packed));

struct instruction
{
	uint8_t op;
	uint8_t spacing1; //reserved
	int16_t x;
	int16_t y;
	uint16_t spacing2; //reserved
} __attribute__((packed));

void togglePen(bool);

#define PEN_PIN D10
#define PEN_DRAWING 120
#define PEN_MOVING 140
bool penStatus;
Servo pen;

Stepper x(D0, D2, D1, D3);
Stepper y(D5, D7, D6, D8);
Draw2D draw(x, y, 0, 256, togglePen);

struct instruction *instructions = NULL;
uint16_t instruction_index;
uint16_t instruction_count = 0;

#ifdef USE_WIFI
WiFiServer server(1337);
#endif

void togglePen(bool status)
{
	if(status != penStatus)
	{
		if(penStatus)
		{
			pen.write(PEN_MOVING);
		}
		else
		{
			uint8_t angle = PEN_MOVING;
			while(angle > PEN_DRAWING)
			{
				angle -= 2;
				pen.write(angle);
				delay(50);
			}
		}

		penStatus = status;
	}
}

void setup()
{
#if defined(USE_WIFI) || defined(DEBUG)
	Serial.begin(115200);
#endif

	LOGLN("Initializing servo");
	pen.attach(PEN_PIN, 1000, 2000);
	pen.write(PEN_MOVING);
	penStatus = false;

	LOGLN("Initializing steppers");
	x.init();
	x.setSpeed(100);
	y.init();
	y.setSpeed(100);

	WiFi.persistent(false);
	WiFi.mode(WIFI_OFF);
#ifdef USE_WIFI
	LOGLN("Initializing WiFi");
	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PSK);

	server.begin();
#endif

	togglePen(true);
	delay(500);
	togglePen(false);
}

void loop()
{
	for(; instruction_index < instruction_count; instruction_index++)
	{
		int16_t x = instructions[instruction_index].x;
		int16_t y = instructions[instruction_index].y;

		LOG("Instruction: op = ");
		LOG(instructions[instruction_index].op);
		LOG(" x = ");
		LOG(x);
		LOG(" y = ");
		LOGLN(y);

		switch(instructions[instruction_index].op)
		{
			case OP_MOVE:
				draw.moveTo(x, y);
				break;

			case OP_MOVE_REL:
				draw.move(x, y);
				break;

			case OP_LINE:
				draw.lineTo(x, y);
				break;

			case OP_LINE_REL:
				draw.line(x, y);
				break;
		}
	}

	if(instructions != NULL)
	{
		togglePen(false);

		free(instructions);
		instructions = NULL;
	}

#ifdef USE_WIFI
	WiFiClient input = server.available();
	if(input)
	{
		LOGLN("Client connected");
#else
	if(Serial.available())
	{
		LOGLN("Data available...");
#endif
		struct header header;
		if(input.readBytes((char *)&header, sizeof(struct header)) == sizeof(struct header))
		{
			LOG("Received header with instruction count ");
			LOGLN(header.instruction_count);

			size_t size = sizeof(struct instruction) * header.instruction_count;
			instructions = (struct instruction *)malloc(size);

			if(instructions != NULL)
			{
				instruction_index = 0;
				instruction_count = header.instruction_count;

				LOG("Transferring ");
				LOG(size);
				LOGLN(" bytes...");

				input.write((uint8_t)0);
				input.setTimeout(5000);

				if(input.readBytes((char *)instructions, size) != size)
				{
					LOGLN("Transmission error!");

					free(instructions);
					instructions = NULL;
					instruction_count = 0;
				}
				else
				{
					LOGLN("Successfully recived instructions");
				}
			}
		}

#ifdef USE_WIFI
		input.stop();
#endif
	}
}
