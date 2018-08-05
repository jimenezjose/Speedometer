#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H

//#include <Arduino.h>
#include <inttypes.h>

#define SPEEDOMETER_SAMPLE_SIZE 100
#define NOISE_DEVIATION 1

class Speedometer {

  public:
    Speedometer( const int8_t sensorPin, const int8_t ledPin );

    void task( void );
    void calibrate( void );

  private:
    void resetData( void );
    bool populateData( void );
    double calcMean( void );
    double calcStanDev( double sampleMean );

    const int8_t _SensorPin;
    const int8_t _LedPin;

    int32_t data[ SPEEDOMETER_SAMPLE_SIZE ];
    int32_t dataIndex;
    double mean;
    double stanDev;

    uint64_t previousMillis; 

};

#endif
