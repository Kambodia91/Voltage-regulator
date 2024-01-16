//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------ 
#include "defines.h"
#include "config.h"
#include "setup.h"
#include "moveTracks.h"

//------------------------------------------------------------------------
// objects
//------------------------------------------------------------------------ 
extern TrackSpeeds speeds;

//------------------------------------------------------------------------
// extern variables
//------------------------------------------------------------------------ 
extern uint32_t OutputLewa;
extern uint32_t OutputPrawa;

//------------------------------------------------------------------------
// variables
//------------------------------------------------------------------------ 
uint32_t axisValueY;
uint32_t axisValueX;

//------------------------------------------------------------------------
// procedures move tracks
//------------------------------------------------------------------------ 
void moveTracks(int leftStick, int rightStick) {
    int speed = map(leftStick, -1000, 1000, -1000, 1000);
    int rotation = map(rightStick, -1000, 1000, -1000, 1000);

    int leftSpeed = speed + rotation;
    int rightSpeed = speed - rotation;

    // Normalizacja wartości leftSpeed i rightSpeed
    if (leftSpeed > 1000) {
        leftSpeed = 1000;
    } else if (leftSpeed < -1000) {
        leftSpeed = -1000;
    }
    if (rightSpeed > 1000) {
        rightSpeed = 1000;
    } else if (rightSpeed < -1000) {
        rightSpeed = -1000;
    }

    speeds.leftSpeed = leftSpeed;
    speeds.rightSpeed = rightSpeed;
}

//------------------------------------------------------------------------
// procedures loop tracks
//------------------------------------------------------------------------ 
void loopTracks() {

moveTracks(axisValueY, axisValueX);

OutputLewa = speeds.leftSpeed;
OutputPrawa = speeds.rightSpeed;

}

//------------------------------------------------------------------------
// end file
//------------------------------------------------------------------------ 