#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H

#include <Arduino.h>
#include <inttypes.h>

#define SPEEDOMETER_SAMPLE_SIZE 100
#define NOISE_DEVIATION 1
#define SPEEDOMETER_DEFAULT_THRESHOLD 0.1
#define SPEEDOMETER_MAX_TIME 1000

class Speedometer {

  public:
    Speedometer( const int8_t sensorPin, const int8_t ledPin );

    void task( void );
    void calibrate( void );

    bool isCalibrating( void );

    int32_t getRPM( void );
    int32_t getMaxRPM( void );
    double getThreshold( void );

    double getMean( void );
    double getStanDev( void );

    void setThreshold( double newThreshold );

  private:
    void resetData( void );
    bool populateData( void );
    double calcMean( void );
    double calcStanDev( double sampleMean );
    void runSpeedometer( void );

    const int8_t _SensorPin;
    const int8_t _LedPin;

    int16_t data[ SPEEDOMETER_SAMPLE_SIZE ];
    int16_t dataIndex;
    double stanDev;
    double mean;

    uint32_t previousMillis; 
    uint32_t totalRevs;
    double threshold;
    int32_t maxRPM;
    int32_t rpm;

};

#endif
