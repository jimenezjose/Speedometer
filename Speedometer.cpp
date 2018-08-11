/****************************************************************************
                                                Jose Jorge Jimenez-Olivas
                                                August 6, 2018

File Name:       Speedometer.cpp
Description:     Emulate a speedometer by calculating the number of 
                 revolutions per unit time on an RC-Car.
****************************************************************************/
#include "Speedometer.h"

/***************************************************************************
% Constructor :  Speedometer
% File :         Speedometer.cpp 
% Parameters:    sensorPin -- Arduino socket connected to the photoresistor
%                ledPin    -- Pin for the backlight led 
***************************************************************************/
Speedometer :: Speedometer( const int8_t sensorPin, const int8_t ledPin ) :
  _SensorPin( sensorPin ), _LedPin( ledPin ) {

  threshold = SPEEDOMETER_DEFAULT_THRESHOLD;
  resetData();
 
  pinMode( sensorPin, INPUT );
  pinMode( ledPin, OUTPUT );
  digitalWrite( ledPin, HIGH );

}

/***************************************************************************
% Routine Name : task
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  Main function that handles when to start and stop sampling.
%                After the sample has been collected, rpm logic is invoked.
% Return:        Nothing
***************************************************************************/
void Speedometer :: task() {

  if( stanDev == 0 ) {
    /* sampling light intensity values */
    calibrate();
    return;
  }
  else if( stanDev < NOISE_DEVIATION + 1 ) {
    /* wheel was not rotating - biased data set, + 1 to ignore decimals */
    Serial.println( "reseting" );
    resetData(); 
    return;
  }

  runSpeedometer();

}

/***************************************************************************
% Routine Name : runSpeedometer
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  Calculates the instantaneous RPM based on the current light 
%                intensity.
% Return:        Nothing
***************************************************************************/
void Speedometer :: runSpeedometer() {
  static bool inPeak = false;        /* currLight surpassed Z_Score threshold */
  static int32_t revolutions = 0;                 /* current tire revolutions */
  static int16_t prevLight = 0;                   /* last valid light reading */
  int16_t currLight = analogRead( _SensorPin );      /* current light reading */
  double Z_Score = 0;                               /* current light standing */
  
  /* Noise control -- avoid currLight to bounce between two adjacent values */ 
  if( abs(currLight - prevLight) > NOISE_DEVIATION ) {
    /* current light reading is valid - update new prevLight */
    prevLight = currLight;
  }
  else {
    /* reset currLight to last valid reading - light in noise range */
    currLight = prevLight;
  }

  Z_Score = (currLight - mean) / stanDev;
   
  if( inPeak == false && Z_Score > threshold ) {
    /* 1 revolution noted, beginning of light peak */
    revolutions++;
    totalRevs++;
    inPeak = true;
  }
  else if( inPeak == true && Z_Score <= threshold ) {
    /* end of light peak -- ready for the next revolution */
    inPeak = false;
  }

  uint32_t currentMillis = millis(); 

  if( revolutions || currentMillis - previousMillis > 7500 ) {
    /* calculate current rpm -- time interupt of 7.5 seconds*/
    double dt = currentMillis - previousMillis;
    dt = dt * (1.0 / 1000) * (1.0 / 60);
    rpm = revolutions / dt;
    /* reset deltas */
    previousMillis = currentMillis; 
    revolutions = 0; 

    if( rpm > maxRPM ) {
      maxRPM = rpm;
    }

  }

  Serial.print( totalRevs );
  Serial.print( "\t" );
  Serial.print( currLight );
  Serial.print( "\t" );
  Serial.print( mean + threshold * stanDev );
  Serial.print( "\t" );
  Serial.println( rpm );
  //Serial.print( "\t" );
  //Serial.println( currentMillis - previousMillis );

}

/***************************************************************************
% Routine Name : calibrate
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  Samples real time light intensity values to calculate and 
%                approximate the population mean and standard deviation for
%                future program reference.
% Return:        Nothing
***************************************************************************/
void Speedometer :: calibrate() {

  if( populateData() ) {
    /* data sample has successfully been collected */
    mean = calcMean();
    stanDev = calcStanDev( mean );
  }
  else {
    /* new data is being collected - necessary for forced recalibration */
    mean = 0; 
    stanDev = 0;
  }

}

/***************************************************************************
% Routine Name : populateData
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  At every 50 millisecond interval, a data element is 
%                collected from the reading of the light sensor pin.
% Return:        true  - sample has finished being collected
%                false - sample is in the process of being collected
***************************************************************************/
bool Speedometer :: populateData() {
  uint8_t sampleSize = SPEEDOMETER_SAMPLE_SIZE;
  uint32_t currentMillis = millis();
  
  if( currentMillis - previousMillis > 75 ) {
    /* delay implemented to emulate a random sample */
    previousMillis = currentMillis;
    dataIndex = dataIndex % sampleSize; /* wrap overloaded index */
    data[ dataIndex ] = analogRead( _SensorPin );
    dataIndex++;                        /* wrap avoided - for valid return */

    Serial.println( data[ dataIndex - 1 ] );
  }

  /* dataIndex will be off by one, to indicate end of sample collection */
  return (dataIndex == sampleSize);

}

/***************************************************************************
% Routine Name : calcMean
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  Calculate the sample mean from the data set collected
% Return:        Sample mean.
***************************************************************************/
double Speedometer :: calcMean() {
  uint8_t sampleSize = SPEEDOMETER_SAMPLE_SIZE;
  uint32_t total = 0;

  for( uint8_t index = 0; index < sampleSize; index++ ) {
    /* accumulate data point values */
    total += data[ index ];
  }

  return total / sampleSize; 
}

/***************************************************************************
% Routine Name : calcStanDev
% File :         Speedometer.cpp 
% Parameters:    sampleMean -- mean of collected sample
% Description :  Calculated an unbiased estimation of the population 
%                standard deviation.
% Return:        Unbiased standard deviation.
***************************************************************************/
double Speedometer :: calcStanDev( double sampleMean ) {
  uint32_t sampleSize = SPEEDOMETER_SAMPLE_SIZE;
  uint32_t deviations = 0; /* current deviations */
  
  for( uint8_t index = 0; index < sampleSize; index++ ) {
    /* squaring deviations for treating all differences equally */
    deviations += sq( data[index] - sampleMean );
  }

  /* unbiased estimation of population standard deviation */
  return sqrt( deviations / (sampleSize - 1) ); 
}

/***************************************************************************
% Routine Name : resetData
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  Resets all sample influenced data.
% Return:        Nothing
***************************************************************************/
void Speedometer :: resetData() {
  mean = 0;
  stanDev = 0;
  dataIndex = 0;
  previousMillis = millis(); 
  totalRevs = 0;
  maxRPM = 0;
}

/***************************************************************************
% Routine Name : isCalibrating
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  sample set is currently being collected. 
% Return:        true - sample is being collected. false - otherwise.
***************************************************************************/
bool Speedometer :: isCalibrating() {
  return dataIndex != SPEEDOMETER_SAMPLE_SIZE;
}

/***************************************************************************
% Routine Name : setThreshold
% File :         Speedometer.cpp 
% Parameters:    newThreshold -- new revolutuion threshold, in terms of 
%                                standard deviations from sample mean.
% Description :  Mutator method to alter the threshold value.
% Return:        Nothing
***************************************************************************/
void Speedometer :: setThreshold( double newThreshold ) {
  threshold = newThreshold;
}

/***************************************************************************
% Routine Name : getRPM
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  Accessor method for instantaeous RPM 
% Return:        Current revolutions per minute rate
***************************************************************************/
int32_t Speedometer :: getRPM() {
  return rpm;
}

/***************************************************************************
% Routine Name : getMaxRPM
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  Accessor method to get the current max rpm value.
% Return:        Current max rpm.
***************************************************************************/
int32_t Speedometer :: getMaxRPM() {
  return maxRPM;
}

/***************************************************************************
% Routine Name : getThreshold
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  Accessor method to the revolution threshold.
% Return:        Revolution threshold 
%                (in terms of standard deviations from the mean)
***************************************************************************/
double Speedometer :: getThreshold() {
  return threshold;
}

/***************************************************************************
% Routine Name : getMean
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  Accessor method to the sample mean
% Return:        Sample mean
***************************************************************************/
double Speedometer :: getMean() {
  return mean;
}

/***************************************************************************
% Routine Name : getStanDev
% File :         Speedometer.cpp 
% Parameters:    None
% Description :  Accessor method to the unbiased standard deviation
% Return:        Sample standard deviation
***************************************************************************/
double Speedometer :: getStanDev() {
  return stanDev;
}

