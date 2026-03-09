#pragma once 

#include "pubSysCls.h"

using namespace sFnd; 

class MotorManager{
    public:  
        MotorManager() = default;
        int inititalize(); 
        int findAndOpenPorts(); 
        int enableMotors(); 
        int disableMotors();
        int homeMotors(); 
        int setSetpoint(int motorIdx, double setpoint); 

        int goToSetpoint(); 
        int atSetpoint(); 
        double getPosition(int motorIdx); 
        int setMotorLimits(double accLimRpmPerSec = 1000, double velLimRpm = 100, double softLimit1 = 0, double softLimit2 = 0);
        int isReady(); 
        int getNodeCount(); 
        int shutdown();
        void testHello();
    private: 
        SysManager *myMgr = nullptr; 
        IPort *myPort = nullptr; 
        INode *getNode(int motorIdx);
        INode *Motor1; 
        INode *Motor2; 
        int motor1Setpoint; 
        int motor2Setpoint; 
};