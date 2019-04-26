
#include <ModbusMaster.h> // RS485 Sensor 
#include <SoftwareSerial.h> // TX/RX communications TTL -> A/B RS4855
#include <Bounce2.h> // debounces reset button
// #include <SdFat.h> //sd card FAT16/FAT32 file card system 



ModbusMaster sensor; //Sensor instance for modbus 
SoftwareSerial softSerial(2, 3); // RX, TX assignment to Uno board
Bounce button = Bounce(); //button gnd at A0 with 2k resistor , vcc 5V
//SdFat sd;


#define DRIVER_ENABLE 7 //Set sensor driver enable to digital pin 7
#define SENSOR_PWR    8 //Set sendor reciver enable to digital pin 8

#define LED1 A1  //green led for operational 
#define LED2 6   //Red led for error, blinking if error

void preTransmission() {
  digitalWrite(DRIVER_ENABLE, HIGH);
   digitalWrite(SENSOR_PWR, HIGH);
}

void postTransmission() {
  digitalWrite(DRIVER_ENABLE, LOW);
  digitalWrite(SENSOR_PWR, LOW);
}

uint16_t data[6]; //data unassigned short int


void setup() {
  // Serial
  Serial.begin(9600);
  softSerial.begin(19200); // RS485 sensor serial 

  //Sensor 
  pinMode(DRIVER_ENABLE, OUTPUT);
  pinMode(SENSOR_PWR, OUTPUT);
  digitalWrite(SENSOR_PWR, HIGH);
  //Modbus slave ID 1
  sensor.begin(1, softSerial);
  //callbacks allow us to configure the RS483 properly
  sensor.preTransmission(preTransmission);
  sensor.postTransmission(postTransmission);
  
  // Reset Button 
  pinMode(A0, INPUT_PULLUP);
  button.attach(A0);
  button.interval(5);

  //LEDs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED1, HIGH);
  delay(1000);
  digitalWrite(LED2, LOW);
  digitalWrite(LED1, LOW);

  pinMode(A2, OUTPUT);
  digitalWrite(A2,HIGH); //shows all loaded
  Serial.println("#Begins...");
  //


}

void loop() {
  
 
    if(button.fell()) {
    
   
      for (int i = 0; i < 5; i++)
     {
     uint8_t r = sensor.readHoldingRegisters(0, 5);
     uint8_t s = sensor.setSlave(2);
     uint16_t Out = sensor.getResponseBuffer(i); //reading address of sensor
     Serial.print("i is ");
     Serial.println(i);
     Serial.print("Out is: ");
     Serial.println(Out);
      
      }
    
      
      digitalWrite(A2,LOW); //button call made successfully green light turns off 
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      delay(100);
      uint8_t tests = 0;
      uint16_t moisture=0;
      int16_t temperature=0;
      uint16_t ver=0;
      uint8_t err = 0;
      //Check sensor IDS
      
      while(tests < 3 && err == 0) {
        uint8_t r = sensor.readInputRegisters(0, 3);
  
        if(0 != r) {
          Serial.print("#Error: ");
          Serial.println(r);
          
          digitalWrite(LED1, LOW);
          digitalWrite(LED2, HIGH);
          delay(100);
          err = 1;
        } 
        else
        {
          moisture = sensor.getResponseBuffer(0);
          temperature = sensor.getResponseBuffer(1);
          ver = sensor.getResponseBuffer(2);
          
          tests++;
          //Set dry and wet rates as below, but activate relay until sensor_value hits acceptable range
          if(moisture < 0 || moisture > 500)
          {
          Serial.println(moisture);
          Serial.println("#MOISTURE OUT OF BOUNDS!");
          err = 1;
          }
          //record temperature, trigger a fault LED if temperature is below 10C or above 30C (returned values in 1/10 degree so 300 = 30C with 2% variation)
          if(temperature < 100 || temperature > 300) 
          { 
           Serial.println(temperature);
           Serial.println("#TEMPERATURE OUT OF BOUNDS!");
           err = 1;
          }
  
          if(err) 
          {
            digitalWrite(LED1, LOW);
            digitalWrite(LED2, HIGH);
            delay(200);
          } else 
          {
            digitalWrite(LED1, HIGH);
            digitalWrite(LED2, LOW);
            delay(50);
          }
        }
      }
      
      if(err == 0)
      {
        Serial.print("The moisture is: ");
        Serial.print(moisture);
        Serial.print(", ");
        Serial.print(temperature/10);
        Serial.println(" C");
      
        //File results = sd.open("results.txt", FILE_WRITE);
        //results.print(moisture);
        //results.print(",");
        //results.println(temperature);
        //results.close();
      }
      
      while(LOW == button.read())
      {
        button.update();
        
      }
    delay(100);
    digitalWrite(LED2, LOW);
    digitalWrite(LED1, LOW);

  } else if(button.rose()){
    digitalWrite(LED2, LOW);
    digitalWrite(LED1, LOW);
  }
  
  button.update();

  
}
