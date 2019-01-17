/*
 Name:		ver1.ino
 Created:	07.05.2018 22:38:15
 Author:	Ksiw
*/

// the setup function runs once when you press reset or power the board

#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Utf8Rus.h>
#include <string.h>             // библиотека расширенной работы со строками
#include <avr/pgmspace.h>


#define ERROR_UPTIME 0      // 1 - сбрасывать uptime при потере связи, 0 - нет
#define DHTPIN 19

// pin 3 - Serial clock out (SCLK)
// pin 4 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 6 - LCD chip select (CS)
// pin 7 - LCD reset (RST)

Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 6, 7);
/*
// правый край полосы загрузки
byte right_empty[8] = { 0b11111,  0b00001,  0b00001,  0b00001,  0b00001,  0b00001,  0b00001,  0b11111 };
// левый край полосы загрузки
byte left_empty[8] = { 0b11111,  0b10000,  0b10000,  0b10000,  0b10000,  0b10000,  0b10000,  0b11111 };
// центр полосы загрузки
byte center_empty[8] = { 0b11111, 0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111 };
*/

char inData[82];       // массив входных значений (СИМВОЛЫ)
int PCdata[20];			// массив численных значений показаний с компьютера
//byte PLOTmem[6][16];   // массив для хранения данных для построения графика (16 значений для 6 параметров)
byte blocks, halfs;
byte index = 0;
String string_convert;
unsigned long timeout, uptime_timer, plot_timer;
boolean lightState, reDraw_flag = 1, updateDisplay_flag, updateTemp_flag, timeOut_flag = 1;

unsigned long sec, mins, hrs;

// Названия для легенды графиков
const char plot_0[] = "CPU ";
const char plot_1[] = "GPU ";
const char plot_2[] = "RAM ";
const char plot_3[] = "\xB0\C ";  //T градусов
const char plot_4[] = "%";  

DHT dht(DHTPIN, DHT22); //Инициация датчика
float humidity, temperature;

const uint8_t  logo[] PROGMEM = {
	0XFF,0XFC,0X7A,0XAA,0XAA,0XAA,0XAA,0XB0,0X00,0X00,0X00,0XFF,0XFC,0X70,0X00,0X00,
	0X00,0X00,0X38,0X00,0X00,0X0A,0XF0,0X7C,0XFA,0XAA,0XAA,0XAA,0XAA,0XBC,0X00,0X00,
	0X03,0XF7,0XFD,0XF0,0X00,0X00,0X00,0X00,0X0C,0X00,0X00,0X01,0XFF,0XFD,0XEA,0XAA,
	0XAA,0XAE,0XAA,0XAF,0X00,0X00,0X0A,0XF0,0X7D,0XD0,0X00,0X00,0X7F,0XC0,0X07,0X80,
	0X00,0X01,0XFF,0XFD,0XEA,0XAA,0XAA,0XFF,0XEA,0XAB,0X80,0X00,0X0F,0XFF,0X7D,0XF4,
	0X00,0X07,0XE0,0X78,0X01,0XC0,0X00,0X0C,0XF0,0X7D,0XFA,0XAA,0XAF,0X80,0X3E,0XAA,
	0XE0,0X00,0X02,0XFF,0XFD,0XFC,0X00,0X3F,0XFC,0X0E,0X00,0X70,0X00,0X0A,0XFF,0XFC,
	0XFE,0XAA,0XBB,0XFF,0X07,0XAA,0XB8,0X00,0X00,0XF7,0XFC,0XF5,0X00,0XFF,0X83,0XC1,
	0XC0,0X18,0X00,0X0E,0XF7,0XFC,0XFA,0XAA,0XEE,0X00,0XE1,0XEA,0XBC,0X00,0X00,0XF7,
	0XFC,0X75,0X43,0XDE,0X00,0XE1,0XE0,0X0E,0X00,0X07,0XF0,0X7C,0XFF,0XAF,0X78,0X00,
	0X38,0X7A,0XAF,0X00,0X0C,0XFF,0X7C,0X7F,0XFF,0XF0,0X00,0X38,0X70,0X07,0X00,0X0E,
	0XF7,0X7C,0X3E,0XB9,0XF0,0X03,0X38,0X3A,0XAB,0XC0,0X0A,0XF7,0X7C,0X1D,0X7B,0XF0,
	0X07,0X18,0X1C,0X00,0XC0,0X00,0XFF,0XFC,0X0E,0XFF,0XF0,0X07,0XB8,0X1E,0XAA,0XE0,
	0X08,0XFF,0XFC,0X07,0XEF,0XF0,0X03,0X38,0X0C,0X00,0X70,0X00,0XFD,0XFC,0X07,0XFF,
	0XF0,0X00,0X38,0X0E,0XAA,0XB8,0X00,0XFF,0XFC,0X07,0X7F,0XF0,0X00,0X70,0X3C,0X00,
	0X1C,0X0A,0XFD,0XFC,0X0F,0XFA,0X30,0X00,0XE0,0X3A,0XAA,0XBE,0X0B,0XFF,0XFC,0X0E,
	0XE0,0X1C,0X01,0XC0,0XF0,0X00,0X06,0X00,0XFD,0XFC,0X3F,0XE0,0X1C,0X03,0X81,0XEA,
	0XAA,0XAF,0X8A,0XFD,0XFC,0X3F,0XC0,0X0E,0X07,0X01,0XC0,0X00,0X07,0X8F,0XFD,0XFC,
	0X7F,0X80,0X0F,0XFE,0X07,0XAA,0XAA,0XAB,0XCC,0XFF,0XFC,0X77,0X00,0X03,0XFC,0X0F,
	0X00,0X7F,0X01,0XC8,0XFF,0XFC,0XEF,0X00,0X80,0X78,0X3E,0XAB,0XFF,0XEA,0XF7,0XFF,
	0XFD,0XDF,0X01,0XC0,0X70,0X78,0X07,0XDF,0XE0,0X7E,0XF4,0XFF,0XBF,0X83,0XC1,0XE0,
	0XEA,0XBE,0X3E,0X3A,0XBF,0XF3,0X7D,0XFF,0X03,0XC1,0XC1,0XC0,0X3C,0X7F,0X38,0X7F,
	0XF7,0X7F,0XFF,0X81,0X83,0X83,0XAA,0XBF,0XE3,0X3A,0XFA,0XFF,0XFF,0XFF,0XC0,0X0E,
	0X07,0X01,0XE3,0X81,0XF9,0XFF,0XF7,0X7F,0XFF,0XE0,0X0E,0X2E,0XAB,0XCF,0X81,0XCB,
	0XF7,0XF7,0X7F,0XFF,0XF0,0X3C,0X5C,0X07,0X9C,0XE7,0X7D,0XFE,0XFF,0XFF,0XFF,0XFE,
	0XFA,0XBE,0XAF,0XB8,0X7F,0X3B,0XFF,0XFF,0XFD,0XFF,0XFF,0XF4,0X7F,0X0F,0XF0,0X7C,
	0X3F,0XFF,0XFF,0XFD,0XFF,0XFF,0XEA,0XEA,0XBC,0XF0,0X78,0XFF,0XF8,0XF7,0X7D,0XBF,
	0XFF,0XD1,0XFD,0X79,0XF0,0X7C,0XF7,0XF0,0XF7,0X7C,0XFF,0XFF,0XAB,0XEA,0XFB,0XBB,
	0X8F,0XEF,0XFF,0XFA,0XFC,0XFF,0XFF,0X1F,0XDF,0XFF,0X1F,0X87,0X87,0XFF,0XFD,0XFC,
	0XFB,0XFE,0XFF,0XFE,0XFE,0X0E,0X1E,0XAF,0XFF,0XFF,0XFC,0X7D,0XFF,0XFF,0XFD,0XFE,
	0X0F,0X1C,0X1F,0XFF,0XFD,0XFC,0X3F,0XBF,0XFA,0XEE,0XFE,0X3F,0XBA,0XBF,0XFF,0XFD,
	0XFC,0X07,0XFF,0X00,0X7F,0XF3,0X71,0XF5,0X5F,0XFF,0XFF,0XFC,0X03,0XF8,0X00,0X3F,
	0XE3,0XF1,0XFF,0XFF,0XF8,0XFF,0XFC,0X00,0X00,0X00,0X1F,0X01,0XE3,0X9F,0XFF,0XF1,
};
void setup() 
{
	Serial.begin(9600);
	display.begin();              // Инициализация дисплея
	display.cp437(true);
	display.setContrast(60);      // Устанавливаем контраст
	display.setTextColor(BLACK);  // Устанавливаем цвет текста
	display.setTextSize(1);       // Устанавливаем размер текста
	display.clearDisplay();       // Очищаем дисплей
	display.display();
	delay(10);
	dht.begin();
	display.drawBitmap(0, 0, logo, 84, 48, BLACK);
	display.display();
    delay(3000);

}

void loop() 
{
	while (true)
	{
		parsing();
		timeoutTick();
		updateDisplay();                    // обновить показания на дисплее
		//debug();
	
		humidity = dht.readHumidity(); //Измеряем влажность
		temperature = dht.readTemperature(); //Измеряем температуру
		display.display();
		delay(250);
		timeOut_flag = false;
	}
}

void parsing()
{
	while (Serial.available() > 0)
	{
		char aChar = Serial.read();
		if (aChar != 'E')
		{
			inData[index] = aChar;
			index++;
			inData[index] = '\0';
		}
		else
		{
			char *p = inData;
			char *str;
			index = 0;
			String value = "";
			while ((str = strtok_r(p, ";", &p)) != NULL)
			{
				string_convert = str;
				PCdata[index] = string_convert.toInt();
				index++;
			}
			index = 0;
		}
		timeout = millis();
		timeOut_flag = false;
	}
}
void updateDisplay()
{
	if (!timeOut_flag)
	{
	display.clearDisplay();                                          //очистить буфер экрана
	
	display.setCursor(0, 0);                                        //cpu load
	display.print(plot_0); display.print(PCdata[4]); display.print(plot_4);

	display.setCursor(0, 10);											//gpu load
	display.print(plot_1); display.print(PCdata[5]); display.print(plot_4);

	display.setCursor(0, 20);										//ram load
	display.print(plot_2); display.print(PCdata[6]); display.print(plot_4);

	display.setCursor(0, 30);											//CPU t
	display.print(plot_0); 	 display.print(PCdata[0]); display.print(plot_3);

	display.setCursor(0, 40);											//gpu t
	display.print(plot_1);  display.print(PCdata[1]); display.print(plot_3);
	
	graph();
	}
}
void timeoutTick()
{
	if ((millis() - timeout > 5000))
	{
		display.clearDisplay();
		display.setCursor(10, 0);
		display.print(utf8rus("СОЕДИНЕНИЕ"));
		display.setCursor(15, 10);
		display.print(utf8rus("ПОТЕРЯНО"));
		timeOut_flag = true;
		
		if (isnan(temperature) || isnan(humidity))
		{
			display.setCursor(25, 30);
			display.print(utf8rus("И DHT"));
			display.setCursor(11, 40);
			display.print(utf8rus("не работает"));
		}
		else
		{
		display.setCursor(0, 23);
		display.print(utf8rus("Темпер~ ")); display.print(temperature, 1); display.print(plot_3);
		display.setCursor(0, 32);
		display.print(utf8rus("Влажно~ ")); display.print(humidity, 1); display.print(plot_4);
		}
		
	}
}
void debug()
{
	display.clearDisplay();
	display.setCursor(0, 0);
	for (int j = 0; j < 2; j++)
	{
		display.print(PCdata[j]); display.print(" ");
	}
	display.print(" 0 ");
	display.setCursor(50, 0);
	for (int j = 3; j < 5; j++)
	{
		display.print(PCdata[j]); display.print(" ");
	}
			
	display.setCursor(0, 10);
	for (int j = 5; j < 10; j++)
	{
		display.print(PCdata[j]); display.print(" ");
	}
	display.setCursor(0, 20);
	for (int j = 10; j < 15; j++)
	{
		display.print(PCdata[j]); display.print(" ");
	}
	display.setCursor(0, 30);
	for (int j = 15; j < 20; j++)
	{
		display.print(PCdata[j]); display.print(" ");
	}
	display.setCursor(0, 40);
	display.print(utf8rus("Жо ")); 
	display.println(utf8rus("19\xB0\C")); //19 град С


}

void graph()
{
	display.fillRect(54, 0, 29, 47, BLACK);					//черный прямоугольник
	display.setRotation(1);									//повернули экран	
	display.setCursor(6, 60);
	display.setTextColor(WHITE);							//белый текст
	if (isnan(temperature) || isnan(humidity))
	{
		display.print(utf8rus("DHT не   робит"));
	}

	else
	{
	display.print(temperature, 1); display.print(plot_3);
	display.setCursor(6, 71);
	display.print(humidity, 1); display.print(plot_4);
	}
	
	display.setTextColor(BLACK);							//обратно черный
	display.setRotation(0);
}