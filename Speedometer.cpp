#include "Speedometer.h"

Speedometer :: Speedometer( const int8_t sensorPin, const int8_t ledPin ) :
  _SensorPin( sensorPin ), _LedPin( ledPin ) {

  threshold = SPEEDOMETER_DEFAULT_THRESHOLD;
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
    /* wheel was not rotating - recalibrate - for biased data set */
    resetData(); 
    return;
  }

  runSpeedometer();

}

void Speedometer :: runSpeedometer() {
  static bool inPeak = false;   /* currLight surpassed Z_Score threshold */
  static int32_t revolutions = 0; /* current tire revolutions */
  static int32_t prevLight = 0; /* last valid light reading */
  int32_t currLight = 0; //analogRead( _SensorPin ); /* current light reading */
  double Z_Score = 0            /* current light standing */
  
  /* Noise control -- avoid currLight to bounce between two adjacent values */

  /*
  if( abs(currLight - prevLight) > NOISE_DEVIATION ) {
    * current light reading is valid - update new prevLight *
    prevLight = currLight;
  }
  else {
    * reset currLight to last valid reading - light in noise range *            
    currLight = prevLight;
  }
  */

  Z_Score = (currLight - mean) / stanDev;
   
  if( inPeak == false && Z_Score > threshold ) {
    /* 1 revolution noted, beginning of light peak */
    revolutions++;
    inPeak = true;
  }
  else if( inPeak == true && Z_Score <= theshold ) {
    /* end of light peak -- ready for the next revolution */
    inPeak = false;
  }

  uint64_t currentMillis = 0;//millis(); TODO

  if( revolutions ) {
    /* calculate current rpm */
    double dt = currentMillis - previousMillis;
    dt = dt * (1.0 / 1000); //* (1.0 / 60); TODO currently rps
    rpm = revolutions / dt;
    /* reset deltas */
    previousMillis = currentMillis; 
    revolutions = 0; 

    if( rpm > maxRPM ) {
      maxRPM = rpm;
    }

  }


}

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

bool Speedometer :: populateData() {
  uint32_t sampleSize = SPEEDOMETER_SAMPLE_SIZE;
  uint64_t currentMillis = 0; //= millis();TODO
  
  if( currentMillis - previousMillis > 50 ) {
    /* delay implemented to emulate a random sample */
    previousMillis = currentMillis;
    dataIndex = dataIndex % sampleSize; /* wrap overloaded index */
    //data[ dataIndex ] = analogRead( _SensorPin );TODO
    dataIndex++;                        /* wrap avoided - for valid return */
  }

  /* dataIndex will be off by one, to indicate end of sample collection */
  return (dataIndex == sampleSize);

}

double Speedometer :: calcMean() {
  uint32_t sampleSize = SPEEDOMETER_SAMPLE_SIZE;
  uint64_t total = 0;

  for( int index = 0; index < sampleSize; index++ ) {
    /* accumulate data point values */
    total += data[ index ];
  }

  return total / sampleSize; 
}

double Speedometer :: calcStanDev( double sampleMean ) {
  uint64_t sampleSize = SPEEDOMETER_SAMPLE_SIZE;
  uint64_t deviations = 0; /* current deviations */
  
  for( int index = 0; index < sampleSize; index++ ) {
    /* squaring deviations for treating all differences equally */
    //deviations += sq( data[index] - sampleMean );
  }

  /* unbiased estimation of population standard deviation */
  return 0;//sqrt( deviations / (sampleSize - 1) ); TODO
}

void Speedometer :: resetData() {
  mean = 0;
  stanDev = 0;
  dataIndex = 0;
  //previousMillis = millis(); TODO
  maxRPM = 0;
}
