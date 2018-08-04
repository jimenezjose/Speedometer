#include "Speedometer.h"

Speedometer :: Speedometer( const int8_t sensorPin, const int8_t ledPin ) :
  _SensorPin( sensorPin ), _LedPin( ledPin ) {

  resetData();
  /*
  pinMode( sensorPin, INPUT );
  pinMode( ledPin, OUTPUT );
  digitalWrite( ledPin, HIGH );
  */

}

void Speedometer :: task() {

  if( stanDev == 0 ) {
    /* sampling light intensity values */
    calibrate();
    return;
  }
  else if( stanDev == NOISE_DEVIATION ) {
    /* wheel was not rotating - recalibrate - biased data set */
    resetData();
    calibrate();
    return;
  }

}

void Speedometer :: calibrate() {

  if( populateData( SPEEDOMETER_SAMPLE_SIZE ) ) {
    /* data sample has successfully been collected */
    mean = calcMean();
    stanDev = calcStanDev( mean );
  }

}

bool Speedometer :: populateData( uint32_t sampleSize ) {

  uint64_t currentMillis = 0; //= millis();
  
  if( currentMillis - previousMillis > 50 ) {
    /* delay implemented to emulate a random sample */
    previousMillis = currentMillis;
    dataIndex = dataIndex % sampleSize; /* wrap overloaded index */
    //data[ dataIndex ] = analogRead( lightSensor );
    dataIndex++;                        /* wrap avoided - for valid return */
  }

  /* dataIndex will be off by one, to indicate end of sample collection */
  return (dataIndex == sampleSize);

}

double Speedometer :: calcMean() {

  mean = 0;

  return mean;
}

double Speedometer :: calcStanDev( double sampleMean ) {

  stanDev = 0;

  return stanDev;
}

void Speedometer :: resetData() {
  mean = 0;
  stanDev = 0;
  dataIndex = 0;
  //previousMillis = millis();
}
