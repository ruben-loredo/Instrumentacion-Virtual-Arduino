/* Control de "Nivel de agua en un tanque"

*/
#include <OneWire.h>
#include <DallasTemperature.h>
//------------puente H L298N----------------
const int ENA = 11;
const int IN1 = 12;
const int IN2 = 13;
//--------------ultrasonico -----------------
const int PinTrig = 10;
const int PinEcho = 9;
const float VelSon = 34000.0;
float distancia;
//--------------Caudalimetro-----------------
unsigned char flowsensor = 2;
//--------------Electrovalvula---------------
const int valvula = 8;
//-------------------------------------------
String StatusValvula = "";
int duty;
int datoSerie;
String inString = "";         // string to hold input
volatile int flow_frequency;  // Measures flow sensor pulses
unsigned int l_hour;          // Calculated litres/hour
unsigned long currentTime;
unsigned long cloopTime;

OneWire ourWire(3);                //Se establece el pin 2  como bus OneWire
DallasTemperature sensors(&ourWire); //Se declara una variable u objeto para nuestro sensor
/*Función que hace el conteo del flujo de los pulsos del sensor
   de agua esta función es un interrupción del programa principal
*/
void flow () // Interrupt function
{
  flow_frequency++;
}

//-----------------------------------------------------------------------
/*Funcion que controla el puente H L298N de
  la bomba de agua
*/
void bomba(int vel)
{
  //------ CW ----------
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  //---------------------
  analogWrite(ENA, vel);
}

/*Función que lee datos numericos enteros por el puerto serie
   y los retorna en la función
*/
int readNum() {
  int intNum = 0;
  while (true)
  {
    if (Serial.available() > 0) {
      int inChar = Serial.read();
      if (isDigit(inChar)) {

        inString += (char)inChar;
      }
      if (inChar == '\n') {
        intNum = inString.toInt();
        inString = "";
        return (intNum);
        break;
      }
    }
  }
}
//-----------------------------------------------------------------------
void WaterFlowSensorYF_S201() {
  currentTime = millis();
  // Every second, calculate and print litres/hour
  if (currentTime >= (cloopTime + 1000))
  {
    cloopTime = currentTime; // Updates cloopTime
    // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
    l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
    flow_frequency = 0; // Reset Counter
    Serial.print("Flujo = ");
    Serial.println(l_hour, DEC); // Print litres/hour
  }
}
//-----------------------------------------------------------------------
void SensorUltrasonico()
{
  // Ponemos el Triiger en estado bajo y esperamos 2 ms
  digitalWrite(PinTrig, LOW);
  delayMicroseconds(2);

  // Ponemos el pin Trigger a estado alto y esperamos 10 ms
  digitalWrite(PinTrig, HIGH);
  delayMicroseconds(10);

  // Comenzamos poniendo el pin Trigger en estado bajo
  digitalWrite(PinTrig, LOW);


  unsigned long tiempo = pulseIn(PinEcho, HIGH);  // La función pulseIn obtiene el tiempo entre estados, en este caso a HIGH
  distancia = tiempo * 0.000001 * VelSon / 2.0;   // Obtenemos la distancia en cm
  Serial.print("d = ");
  Serial.println(distancia);
}

void SensorTempDS1820() {
  sensors.requestTemperatures();   //Se envía el comando para leer la temperatura
  float temp = sensors.getTempCByIndex(0); //Se obtiene la temperatura en ºC

  Serial.print("Temp = ");
  Serial.println(temp);
}

//-----------------------------------------------------------------------
void setup() {
  // Ponemos el pin Trig en modo salida
  pinMode(PinTrig, OUTPUT);
  // Ponemos el pin Echo en modo entrada
  pinMode(PinEcho, INPUT);
  //--------- CONFIGURA PUENTE H ----------
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  //--------- CONFIGURA CAUDALIMETRO----------
  pinMode(flowsensor, INPUT);
  //--------- CONFIGURA ELECTROVALVULA----------
  pinMode(valvula, OUTPUT);
  digitalWrite(valvula, LOW);   //inicializa en estado bajo
  StatusValvula = "OFF";
  //-------inicializa Sensor Temperatura----------------
  sensors.begin();
  //---------------------------------------------------
  digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
  attachInterrupt(0, flow, RISING); // Setup Interrupt
  sei(); // Enable interrupts
  currentTime = millis();
  cloopTime = currentTime;

  // configura el puerto Serie
  Serial.begin(9600);

  while (!Serial) {
    ; // Espera a conectarse.
  }
}

void loop() {
  Serial.println("-----------------------------------");
  if (Serial.available() > 0)
  {
    datoSerie = Serial.read();
    switch (datoSerie) {
      case 'A':
        digitalWrite(valvula, HIGH);
        StatusValvula = "ON";
        break;
      case 'a':
        digitalWrite(valvula, LOW);
        StatusValvula = "OFF";
        break;
      case 'b':
        duty = readNum();
        bomba(duty);
        Serial.print("b");
        Serial.println(duty);
        break;

    }
  }
  Serial.print("Bomba = ");
  Serial.println(duty);
  Serial.print("Valvula = ");
  Serial.println(StatusValvula);
  WaterFlowSensorYF_S201();   //funcion que envia datos del sensor de flujo
  SensorUltrasonico();        //funcion que envia datos del sensor ultrasonico
  SensorTempDS1820();         //funcion que envia datos del sensor de temperatura


  delay(500);

}
