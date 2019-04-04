/*

Credits: 

* Langster's LC Meter code 
 
* Kerry Wongs LC Meter code

* Stefan E. Schausberger code

 Uses the LCD library, Frequency Library
 Button libary and various other bits and piecee
  
*/

// Include the Frequency counter library
#include "FreqCounter.h"

// Include the Button Library
#include "Button.h"

// Include the Math Library
#include <math.h>

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

const int frequencyButtonPin = 17;     // the number of the pushbutton pin -> button on pin 16 or A2
const int calibrateButtonPin = 18;     
const int okButtonPin = 16;     
const int zweiButtonPin = 15;    
const int einsButtonPin = 14;  

const int relayPin = 19;               // the number of the relay pin
const int componentSelectPin = 2;      // the number of the component select pin
const float pi2=6.283185307;

Button calibrationButton = Button(calibrateButtonPin, BUTTON_PULLDOWN);  //Calibration button on pin 15 or A1
Button frequencyButton = Button(frequencyButtonPin, BUTTON_PULLDOWN);    //Frequency button on pin 16 or A2
Button okButton = Button(okButtonPin, BUTTON_PULLDOWN);                  //OK button on pin 17 or A3


// Spulen Werte
const float maxforce = 10.00; // [N]
const float maxlength = 59.6; // [mm]
const float ic1min = 0.002374; // [H] Induction without core
const float ic1half = 0.003409; // [H] Induction without core
const float ic1max = 0.00505; // [H] Induction with core

float mind[]={0.00,0.00,0.00};

float I;
float Beins;
float Bzwei;
       

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
  Serial.println(" =====================================");
  Serial.println("   Radial Extensor V2  - SES 06.2014");  
  Serial.println(" =====================================");
  Serial.println("");    
  digitalWrite(relayPin, LOW);
//  Serial.print("Relais:");
//  Serial.println("1");  

  lcd.setCursor(0, 0);
  lcd.print("*   R.S.S.    *");
  lcd.setCursor(0, 1);
  lcd.print(" -=< S.E.S. >=- ");  
 
  //measureCalibrationCapacitance();
  //measureCalibrationInductance();
   
  delay(2000);
 
  lcd.clear();
  
} 

void loop()
{
  
        currentMode = L;
        measureInductance();
        
  //check if calibration is required
 
  if(calibrationButton.isPressed() && frequencyButton.isPressed()){
    Serial.println("both buttons pressed");
    switchState = digitalRead(componentSelectPin);
   
    if (switchState==LOW)
      {
    Serial.println("SelectPin: LOW -> L measurement");

          currentMode = L;
          lcd.setCursor(0, 0);
          lcd.print("Short Terminals");
          lcd.setCursor(0, 1);
          lcd.print("Press Cal Button");
         
          if(calibrationButton.isPressed())
            {
    Serial.println("L Calibrate button pressed");              
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
    Serial.println("C Calibrate button pressed");                
              measureCalibrationCapacitance();
            }
      }
     
  }
 
  //check if frequency measurement is required
 
  if(frequencyButton.isPressed()){
    Serial.println("Freq button pressed");          
        measureFrequency();
        digitalWrite(relayPin, LOW);
  }

  // When CALIBRATE Button is pressed OR no I Value
  if(calibrationButton.isPressed() ){ //|| !I){
        int mval[]={1,5,10};
        
    
    
        // save three Points START    
        for (int i=0; i<3; i++){
          lcd.setCursor(0, 0);
          lcd.print("Calibration S1");
          lcd.setCursor(0, 1);
          lcd.print(mval[i]); 
          lcd.print("N -> press OK"); 
          
            while (!okButton.uniquePress()){ 
              delay(50);
              /*lcd.setCursor(0, 1);
              lcd.print(mval[i]); 
              lcd.print("  change     ");             */
            }
              delay(200);
              for (int j=0; j<10; j++){
                FreqCounter::f_comp=10;   // Cal Value / Calibrate with professional Freq Counter
                FreqCounter::start(100);  // 100 ms Gate Time
                while (FreqCounter::f_ready == 0)
                frq=FreqCounter::f_freq;
                measureComponentFreq=frq;
              }
             
              delay(200);      
        
           
            //Serial.println("berechnet induktion");    
            temp1 = sq(indFreq);
            temp2 = sq(measureComponentFreq);
            temp3 = float(temp1)/float(temp2);
            lMeasured = Lth*(temp3-1) - lMeasuredZero;
            Serial.print(mval[i]);
            Serial.print(" N -> ");
            Serial.println(lMeasured,12);
            mind[i]=lMeasured;
        
            // Umrechnen auf Weg und Kraft
         
            //float deflection = (1-((lMeasured-ic1min)/(ic1max-ic1min)))*maxlength;
            //float force = (deflection/maxlength)*maxforce;
            //Serial.println(deflection,4);
            //Serial.println(force,4);
        
            lcd.setCursor(0, 0);
            lcd.print("L: ");   
               
  
            lMeasured = lMeasured * 1e3; // milli
            lcd.print(lMeasured);
            lcd.print(" ");
            lcd.print("mH");
            lcd.print("        ");
           
            lcd.setCursor(0, 1);
            lcd.print("SUCCESS       ");
            
            delay(1000);
            
            
       }  
       
       // save three Points END
       Serial.print("measured points: ");
       Serial.print(mind[0],6);
       Serial.print(" - ");
       Serial.print(mind[1],6);
       Serial.print(" - ");       
       Serial.println(mind[2],6);
       
       
       // Calc Binominal Values: I=mind[i]=Y und F=mval[i]=X
       float X1 = mval[0];
       float X2 = mval[1];
       float X3 = mval[2];

       float Y1 = mind[0];
       float Y2 = mind[1];
       float Y3 = mind[2];       
       
       I = (X1 * X3 *(-X1 + X3) * Y2 + (X2 * X2) * (X3 * Y1 - X1 * Y3) + X2 *(-(X3 * X3) * Y1 + (X1 * X1) * Y3)) / ((X1 - X2) * (X1 - X3) * (X2 - X3));
       Beins = ((X3 * X3) * (Y1 - Y2) + (X1 * X1) * (Y2 - Y3) + (X2 * X2) * (-Y1 + Y3))/((X1 - X2) * (X1 - X3) * (X2 - X3));
       Bzwei = (X3 * (-Y1 + Y2) + X2 * (Y1 - Y3) + X1 * (-Y2 + Y3))/((X1 - X2) * (X1 - X3) * (X2 - X3));
      Serial.print("Value I: ");
      Serial.println(I,9);
      Serial.print("Value B1: ");
      Serial.println(Beins,9);
      Serial.print("Value B2: ");
      Serial.println(Bzwei,9);
  }


// Measure


delay(500); 
}


//Measure the frequency

void measureFrequency()
{
    Serial.println("measure freq");  
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
    Serial.println("L calibr. masurement");  
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
    Serial.print("measure Frequenz: ");    
      FreqCounter::f_comp=10;   // Cal Value / Calibrate with professional Freq Counter
      FreqCounter::start(100);  // 100 ms Gate Time
      while (FreqCounter::f_ready == 0)
      frq=FreqCounter::f_freq;
      measureComponentFreq=frq;
      Serial.println(frq);         
      delay(200);
     
      calcIndData();

}



//Calculate and Display the Inductance

void calcIndData(){
   
    Serial.print("calculat Induction: ");    
    temp1 = sq(indFreq);
    temp2 = sq(measureComponentFreq);
    temp3 = float(temp1)/float(temp2);
    lMeasured = Lth*(temp3-1) - lMeasuredZero;
    Serial.println(lMeasured,6);

    // Umrechnen auf Weg und Kraft
    
    
    float force = -( Beins + sqrt((Beins * Beins) - 4 * Bzwei * I + 4 * Bzwei * lMeasured))/(2*Bzwei);
    Serial.print("Kraft: ");
    Serial.println(force,6);
    float deflection = (1-((lMeasured-ic1min)/(ic1max-ic1min)))*maxlength;
    
    //float force = (deflection/maxlength)*maxforce;
    //Serial.println(deflection,4);
    //Serial.println(force,4);

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
