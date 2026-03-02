#include <SPI.h>
#include <math.h>

//Pin initialization for Array 
int CSB_ARRAY = 11;
int RST_ARRAY = 12;
int LNASW_ARRAY = 10;
int VEN_ARRAY = 6;
int HEN_ARRAY = 8;

//Pin initialization for UDC
int CSB_UDC = 7;
int RST_UDC = 13;
int STDBY_UDC = 9;

// Pin initialization for Trigger
int TRIG = 14;

// Trigger flag
int trigFlag = 0;

byte buf[128]; // Used for header related functions
byte data[128]; // Used for data related functions
byte ret[128]; // Used for MATLAB feedback functions 

// trackerLUT keeps track of the LUT states; Each element corresponds to the one BF chip.
// The code is written such that when the value at an index is a multiple of 4 then all the four channels correponding to that chip are programmed
// floor(<value at an index> / 4) gives the LUT address filled in the corresponding chip
int trackerLUT[8] = {0, 0, 0, 0, 0, 0, 0, 0}; 

// Codebook size
int CBSize = 0;

// Return value after operation
int val = 0;

SPISettings setting(50000000, MSBFIRST, SPI_MODE0);

void setup() {
  //Initializing chip select pins
  pinMode(CSB_ARRAY, OUTPUT);
  pinMode(CSB_UDC, OUTPUT);

  //Initializing control pins
  pinMode(RST_ARRAY, OUTPUT);
  pinMode(LNASW_ARRAY, OUTPUT);
  pinMode(VEN_ARRAY, OUTPUT);
  pinMode(HEN_ARRAY, OUTPUT);
  pinMode(STDBY_UDC, OUTPUT);
  pinMode(RST_UDC, OUTPUT);

  digitalWrite(RST_ARRAY, LOW);
  digitalWrite(LNASW_ARRAY, LOW);
  digitalWrite(VEN_ARRAY, LOW);
  digitalWrite(HEN_ARRAY, LOW);
  digitalWrite(STDBY_UDC, LOW);
  digitalWrite(RST_UDC, LOW);

  //Initializing Trigger Pin
  pinMode(TRIG, INPUT);

  //Setting D4 and D5 as input - if using FSW port for switching between Tx/Rx, D4 and D5 as input
  pinMode(4, INPUT); //D4
  pinMode(5, INPUT); //D5

  //Disabling internal MCU power and control 
  pinMode(2, OUTPUT); //D2
  pinMode(3, OUTPUT); //D3
  
  digitalWrite(2, HIGH);
  digitalWrite(3, LOW);
  
  Serial.begin(115200);
  SPI.begin();
}

void loop() {
  if (Serial.available() >= 1){
    // Read two bytes from buffer which contain operation type and operation related values
    Serial.readBytes(buf, 2);
    decodeCommand(buf[0]);
  }

  // if (val == 0){
  //   Serial.flush();
  // }
  // if (Serial.available() >= 1){
  //   // Read two bytes from buffer which contain operation type and operation related values
  //   Serial.readBytes(buf, 2);
  //   val = decodeCommand(buf[0]);
  // }

  if (digitalRead(TRIG)){
    trigFlag = 1;
  }

}

int decodeCommand(int com){
  switch (com) {
    case 1:
      // Sets a pin high
      configPinHigh(buf[1]); // buf[1] contains the pin number
      //return 1;
      break;
    case 2:
      // Sets a pin low
      configPinLow(buf[1]); // buf[1] contains the pin number
      //return 1;
      break;
    case 3:
      // Command for static mode writing or reading from registers of BF chip
      regCommandBF(buf[1]); // buf[1] contains the length of the following data bytes (the length will be different for read and write commands)
      //return 1;
      break;
    case 4:
      // Command for static mode writing or reading from registers of UDC chip
      regCommandUDC(buf[1]); // buf[1] contains the length of the following data bytes (the length will be different for read and write commands)
      //return 1;
      break;
    case 5:
      // Command to load the values into the LUT
      loadLUT(buf[1]); // buf[1] contains the length of the following data bytes (the value is 5 for writing LUT)
      //return 1;
      break;
    case 6:
      //Execute fast beam scan once Trigger pulse is received
      if (trigFlag){
        // Command for performing Fast Beam Scan mode 1: Local 
        fastBeamScanLocal(((buf[1] & 128) >> 7), ((buf[1] & 64) >> 6)) ; // buf[1] contains polarization and mode values in the MSBs
      }
      //return 1;
      break;
    case 7:
      // Command for resetting LUTs; the code does not keep track multiple LUT positions, it scans the LUT beginning from 0 to the size of codebook
      resetLUTs();
      //return 1; 
      break;
  }
  return 0;
}

void configPinHigh(int pin){
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

void configPinLow(int pin){
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void regCommandBF(int len){
  // Read the data bytes
  Serial.readBytes(data, len);

  // Transfer SPI data
  SPI.beginTransaction(setting);
  digitalWrite(CSB_ARRAY, LOW);
  for(int i = 0; i < len; i++){
    ret[i] = SPI.transfer(data[i]);
  }
  digitalWrite(CSB_ARRAY, HIGH);
  SPI.endTransaction();

  // Returning the data read to MATLAB
  for(int i = 0; i < len; i++){
    Serial.write(ret[i]);
  }  
}

void regCommandUDC(int len){
  // Read the data bytes
  Serial.readBytes(data, len);

  // Transfer SPI data
  SPI.beginTransaction(setting);
  digitalWrite(CSB_UDC, LOW);
  for(int i = 0; i < len; i++){
    ret[i] = SPI.transfer(data[i]);
  }
  digitalWrite(CSB_UDC, HIGH);
  SPI.endTransaction();

  // Returning the data read to MATLAB
  for(int i = 0; i < len; i++){
    Serial.write(ret[i]);
  }
}

void loadLUT(int len){
  // Read the data bytes
  Serial.readBytes(data, len);

  //Extract the chip address 
  int chipAddress = (data[0] & 60) >> 2; 
  // Calculate the LUT address
  int LUTAddress = floor(trackerLUT[chipAddress] / 4);

  // Insert the LUT address to the received data
  data[1] = LUTAddress >> 3;
  data[2] = (data[2] | (LUTAddress << 5));

  // Increment tracker to notify that the corresonding chip's LUT is filled
  trackerLUT[chipAddress] = trackerLUT[chipAddress] + 1;

  //Writing to the LUT
  SPI.beginTransaction(setting);
  digitalWrite(CSB_ARRAY, LOW);
  for(int i = 0; i < len; i++){
    SPI.transfer(data[i]);
  }
  digitalWrite(CSB_ARRAY, HIGH);
  SPI.endTransaction();

  // Reading back from the LUT
  data[0] = (data[0] & 253); // Last two bits should be (01)
  data[3] = 0;
  data[4] = 0;
  SPI.beginTransaction(setting);
  digitalWrite(CSB_ARRAY, LOW);
  for(int i = 0; i < 5; i++){
    ret[i] = SPI.transfer(data[i]);
  }
  digitalWrite(CSB_ARRAY, HIGH);
  SPI.endTransaction();
  ret[2] = LUTAddress;
  for(int i = 0; i < 5; i++){
    Serial.write(ret[i]);
  }  
  
  // Calculation of the codebook size
  int trackerSum;
  for(int i = 0; i < 8; i++)
    trackerSum += trackerLUT[i];

  // The code fills LUTs in order starting from 0 and programs each channel of a chip
  // The codebook size is incremented by 1, if sum of values in the trackerLUT is a multiple of 32 (8 chips x 4 channels = 32 values stored in LUT)
  if (!(trackerSum % 32))
    CBSize += 1;
}

void fastBeamScanLocal(int pol, int mode){
  int latchEN = 1;
  int tmp_pol = pol;
  int numCBrep;

  // Read the number of beamscan repetitions to perform 
  Serial.readBytes(&buf[2], 1);

  // Beamscan repetition is taken to multiples of 5
  numCBrep = buf[2] * 5;

  // Returning the codebook size to MATLAB
  Serial.write(CBSize);
  SPI.beginTransaction(setting);
  // Repetition of the codebook 
  for(int rep = 0; rep < numCBrep; rep++){
    // Iterate over each LUT address; each LUT address contains 1 beam
    for(int addressLUT = 0; addressLUT < CBSize; addressLUT++){
      latchEN = 1; // Latch enable for beam scanning; Only set to one if both 'H' and 'V' polarizations are programmed
      tmp_pol = !pol; 
      // Two iterations of the same codebook to ensure both 'H' and 'V' are programmed
      for(int iter = 0; iter < 2; iter++){
        latchEN = !latchEN; // latchEN = 0 during the first iteration, so that latching happens after both polarizations are programmed
        tmp_pol = !tmp_pol; // Switching between polarizations to program
          // Program each chip
          for(int addressChip = 0; addressChip < 8; addressChip++){
            // Fast beam scan local SPI command
            data[0] = (1 << 6)|(addressChip << 2)|(pol << 1)|(mode);
            data[1] = (addressLUT >> 3);
            data[2] = (addressLUT << 5) | (latchEN << 4);

            // Send Fast Beam Scan command
            
            digitalWrite(CSB_ARRAY, LOW);
            for(int i = 0; i < 3; i++){
              SPI.transfer(data[i]);
            }
            digitalWrite(CSB_ARRAY, HIGH);
            
//            for(int i = 0; i < 3; i++){
//              Serial.write(data[i]);
//            }
          }
      }
//      delay(20);
    }
  }
  SPI.endTransaction();
}

void resetLUTs(){
  // Resetting the trackerLUT array
  for(int i = 0; i < 8; i++){
    trackerLUT[i] = 0;
  }
  // Setting codebook size to 0
  CBSize = 0;
  // Feedback to MATLAB
  Serial.write(1);
}
