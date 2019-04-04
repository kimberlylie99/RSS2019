/*
  Langster's LC Meter Code - 19-07-2013
  Uses the LCD library, Frequency Library
  Button libary and various other bits and piecee
 
  V1.0
 
  This code borrows from Kerry Wongs LC Meter
  code!
 
  To Calibrate press both the Frequency and
  Calibrate buttons together!
 
  To measure frequency press the frequency
  button
 
  Enjoy!
  
 */

// Include the Frequency counter library
#include <FreqCounter.h>

// Include the Button Library
#include <Button.h>

//part of the switch position check
enum meterMode {
    L,
    C,
    F
};

unsigned long indFreq = 23896;   //rough frequency from oscilloscope measurements
unsigned long capFreq = 23885;   //rough frequency from oscilloscope measurements

long measureComponentFreq = 0;   //variable to store component measurement frequency

float cMeasured;                 //Th measured capacitance
double lMeasured;                //The measured inductance

float cMeasuredZero;             //The zero factor for capacitance
double lMeasuredZero;            //The zero factor for inductance

//Some temporary variables for calculations

long temp1;
long temp2;
double temp3;

const float Cth = 4.6 * 1e-9;    //measured 4.7nF - calibration capacitor value
const float Lth = 99.6 * 1e-6;  //measured 91.14uH - calibration inductor value

int i=0;                         // count variable
int switchState;                 // variable for storing the state of the switch

unsigned long frq;               //The frequency measurement
long capFreqArray[10];           //An array for storing frequencies
long indFreqArray[10];           //An array for storing frequencies

long frqAverage = 0;             //An variable for averaging
long average = 0;               

const int frequencyButtonPin = 17;     // the number of the pushbutton pin
const int calibrateButtonPin = 18;     // the number of the pushbutton pin
const int relayPin = 19;               // the number of the relay pin
const int componentSelectPin = 2;      // the number of the component select pin
const float pi2=6.283185307;

Button calibrationButton = Button(calibrateButtonPin, BUTTON_PULLDOWN);  //Calibration button on pin 15 or A1
Button frequencyButton = Button(frequencyButtonPin, BUTTON_PULLDOWN);    //Frequency button on pin 16 or A2


// Spulen Werte
const float maxforce = 10.00; // [N]
const float maxlength = 59.6; // [mm]
const float ic1min = 0.002374; // [H] Induktion ohne Kern
const float ic1half = 0.003409; // [H] Induktion ohne Kern
const float ic1max = 0.00505; // [H] Induktion mit Kern



meterMode currentMode;                  //check which mode the switch is in

// include the LCD display library:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

//Check the switch position

void checkLCMode() { 
        currentMode = L;
        measureInductance();
}

//setup the arduino
void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  pinMode(relayPin, OUTPUT);
 
  pinMode(calibrateButtonPin, INPUT);
  pinMode(frequencyButtonPin, INPUT);
  pinMode(componentSelectPin, INPUT);
 
  digitalWrite(componentSelectPin, HIGH);
 
  Serial.begin(57600);        // connect to the serial port
  digitalWrite(relayPin, LOW);
//  Serial.print("Relais:");
//  Serial.println("1");  

  lcd.setCursor(0, 0);
  lcd.print("* Strain Meter *");
  lcd.setCursor(0, 1);
  lcd.print(" -=< S.E.S. >=- ");  
 
  //measureCalibrationCapacitance();
  //measureCalibrationInductance();
   
  delay(2000);
 
  lcd.clear();
  
} 

void loop()
{
  //check if calibration is required
 
  if(calibrationButton.isPressed() && frequencyButton.isPressed())
  {
    Serial.println("beide Tasten gedrückt");
    switchState = digitalRead(componentSelectPin);
   
    if (switchState==LOW)
      {
    Serial.println("SelectPin: LOW -> L messen");

          currentMode = L;
          lcd.setCursor(0, 0);
          lcd.print("Short Terminals");
          lcd.setCursor(0, 1);
          lcd.print("Press Cal Button");
         
          if(calibrationButton.isPressed())
            {
    Serial.println("L Calibr Taste gedrueckt");              
              measureCalibrationInductance();
            }
 
      }
   
    if (switchState==HIGH)
 
      {
          currentMode = C;
          lcd.setCursor(0, 0);
          lcd.print("Clear Terminals");
          lcd.setCursor(0, 1);
          lcd.print("Press Cal Button");
         
          if(calibrationButton.isPressed())
            {
    Serial.println("C Calibr Taste gedrueckt");                
              measureCalibrationCapacitance();
            }
      }
     
  }
 
  //check if frequency measurement is required
 
  if(frequencyButton.isPressed())
      {
    Serial.println("Freq Taste gedrueckt");          
        measureFrequency();
        digitalWrite(relayPin, LOW);
      }

  if(calibrationButton.isPressed(){
    
  }


checkLCMode();    //check switch position

 
}

//Measure the frequency

void measureFrequency()
{
    Serial.println("misst freq");  
    digitalWrite(relayPin, HIGH);
    delay(500);
 
    FreqCounter::f_comp=10;   // Cal Value / Calibrate with professional Freq Counter
    FreqCounter::start(100);  // 100 ms Gate Time
    while (FreqCounter::f_ready == 0)
    frq=FreqCounter::f_freq;  
   
    lcd.setCursor(0, 0);
    lcd.print("Frequency ");
   
    lcd.setCursor(0, 1);
    lcd.print("F: ");
    lcd.print(frq);
    lcd.print("      ");
    
    delay(4000);
   
}

//Calibrate for inductance measurements

void measureCalibrationInductance(){
    Serial.println("L calibr. messung");  
    for (int i=0; i<100; i++)
       
        {
          FreqCounter::f_comp=10;   // Cal Value / Calibrate with professional Freq Counter
          FreqCounter::start(100);  // 100 ms Gate Time
          while (FreqCounter::f_ready == 0)
          frq=FreqCounter::f_freq;
          indFreq=frq;
         
          temp1 = sq(indFreq);
          temp2 = sq(measureComponentFreq);
          temp3 = float(temp1)/float(temp2);
          lMeasured = Lth*(temp3-1);
          lMeasuredZero = lMeasured;
          i++;
         
        }
     
      lcd.setCursor(0, 0);
      lcd.print("Calibration     ");
      lcd.setCursor(0, 1);
      lcd.print("Complete        ");
      delay(4000);
     
      lcd.clear();
   
}

//Measure the inductance

void measureInductance()

{
    Serial.println("misst Induction");    
      FreqCounter::f_comp=10;   // Cal Value / Calibrate with professional Freq Counter
      FreqCounter::start(100);  // 100 ms Gate Time
      while (FreqCounter::f_ready == 0)
      frq=FreqCounter::f_freq;
      measureComponentFreq=frq;
     
      delay(200);
     
      calcIndData();

}



//Calculate and Display the Inductance

void calcIndData(){
   
    Serial.println("berechnet induktion");    
    temp1 = sq(indFreq);
    temp2 = sq(measureComponentFreq);
    temp3 = float(temp1)/float(temp2);
    lMeasured = Lth*(temp3-1) - lMeasuredZero;
    Serial.println(lMeasured,12);

    // Umrechnen auf Weg und Kraft
 
    float deflection = (1-((lMeasured-ic1min)/(ic1max-ic1min)))*maxlength;
    float force = (deflection/maxlength)*maxforce;
    Serial.println(deflection,4);
    Serial.println(force,4);

    lcd.setCursor(0, 0);
    lcd.print("L: ");   
       
     if (lMeasured >= 1e-9 && lMeasured < 1e-6)
            {
                lMeasured = lMeasured * 1e9; // nano
                lcd.print(lMeasured);
                lcd.print(" ");
                lcd.print("nH");
                lcd.print("        ");
            }
           
            if (lMeasured > 1e-6 && lMeasured < 1e-3)
        
            {
                lMeasured = lMeasured * 1e6; // micro
                lcd.print(lMeasured);
                lcd.print(" ");
                lcd.print("uH");
                lcd.print("        ");
            }
           
           
            if (lMeasured > 1e-3)
           
            {
                lMeasured = lMeasured * 1e3; // milli
                lcd.print(lMeasured);
                lcd.print(" ");
                lcd.print("mH");
                lcd.print("        ");
            }
   
    lcd.setCursor(0, 1);
    lcd.print("S:");
    lcd.print(deflection,1);
    lcd.print("mm F:");
    lcd.print(force,1);
    lcd.print("N   ");    
 
} 

//Measure the Calibration Capacitance

void measureCalibrationCapacitance()
{
  
  for (int i=0; i<100; i++)
        {
          FreqCounter::f_comp=10;   // Cal Value / Calibrate with professional Freq Counter
          FreqCounter::start(100);  // 100 ms Gate Time
          while (FreqCounter::f_ready == 0)
          frq=FreqCounter::f_freq;
          capFreq=frq;
         
          temp1 = sq(capFreq);
          temp2 = sq(measureComponentFreq);
          temp3 = float(temp1)/float(temp2);
          cMeasured = Cth *(temp3-1);
          cMeasuredZero = cMeasured;
          i++;
        }
    
     lcd.setCursor(0, 0);
     lcd.print("Calibration     ");
     lcd.setCursor(0, 1);
     lcd.print("Complete        ");
     delay(4000);
    
     lcd.clear();
    
}


//Measure the capacitance

void measureCapacitance(){
   
      FreqCounter::f_comp=10;   // Cal Value / Calibrate with professional Freq Counter
      FreqCounter::start(100);  // 100 ms Gate Time
      while (FreqCounter::f_ready == 0)
      frq=FreqCounter::f_freq;
      measureComponentFreq=frq;
     
      delay(200);
     
      calcCapData();
   
}

//Calculate and display the capacitance
 
void calcCapData(){
   
    temp1 = sq(capFreq);
    temp2 = sq(measureComponentFreq);
   
    temp3 = float(temp1)/float(temp2);
    cMeasured = Cth*(temp3-1)-cMeasuredZero;
   
    Serial.print("Capacitor Oscillator Frequency: ");
    Serial.print(capFreq);
    Serial.println();
    Serial.print("Component Oscillator Frequency: ");
    Serial.print(measureComponentFreq);
    Serial.println();
    Serial.print("Cap Osc Squared: ");
    Serial.print(temp1);
    Serial.println();
    Serial.print("Msr Osc Squared: ");
    Serial.print(temp2);
    Serial.println();
    Serial.print("Division       : ");
    Serial.print(temp3);
    Serial.println();
    Serial.print("Component Value: ");
    Serial.print(cMeasured);
    Serial.println();
   
      
    lcd.setCursor(0, 1);
    lcd.print("C: ");
       
     if (cMeasured < 1e-9)
            {
                cMeasured = cMeasured * 1e12; // pico
                lcd.print(cMeasured);
                lcd.print(" ");
                lcd.print("pF");
                lcd.print("       ");
            }
           
     if (cMeasured >= 1e-9 && cMeasured < 1e-6)
           
            {
                cMeasured = cMeasured * 1e9; // n
                lcd.print(cMeasured);
                lcd.print(" ");
                lcd.print("nF");
                lcd.print("       ");
            } 
           
      if (cMeasured > 1e-6)     
            {
                lcd.print("Out of Range");
                lcd.print("        ");
            }
   
}  
