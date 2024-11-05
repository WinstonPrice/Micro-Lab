
// Proportional Control Setup
// Only Kp is active, Ki and Kd are set to zero.

float Kp = 0.5; // Adjust this value for proportional control testing
float Ki = 0.0;
float Kd = 0.0;

// Function to compute control signal with Proportional Only
float computeProportionalControl(float error) {
    float proportional = Kp * error;
    return proportional;
}


// Integral Control Setup
// Only Ki is active, Kp and Kd are set to zero.

float Kp = 0.0;
float Ki = 0.2; // Adjust this value for integral control testing
float Kd = 0.0;

// Function to compute control signal with Integral Only
float computeIntegralControl(float error, float &integralSum) {
    integralSum += error; // Accumulate error
    float integral = Ki * integralSum;
    return integral;
}


// Derivative Control Setup
// Only Kd is active, Kp and Ki are set to zero.

float Kp = 0.0;
float Ki = 0.0;
float Kd = 0.1; // Adjust this value for derivative control testing

// Function to compute control signal with Derivative Only
float computeDerivativeControl(float error, float lastError) {
    float derivative = Kd * (error - lastError);
    return derivative;
}


// Full PID Calibration Setup
// All three terms Kp, Ki, and Kd are active for testing PID response.

float Kp = 0.5;  // Adjust these values for optimal PID control
float Ki = 0.1;
float Kd = 0.05;

// Function to compute control signal with full PID
float computePIDControl(float error, float &integralSum, float lastError) {
    integralSum += error; // Integral term
    float derivative = (error - lastError); // Derivative term
    float output = (Kp * error) + (Ki * integralSum) + (Kd * derivative);
    return output;
}


// IMU-Based PID Control Setup
// This PID controller maintains an angle balance using IMU readings

float Kp = 0.5;
float Ki = 0.1;
float Kd = 0.05;

// Function to compute control signal with PID for IMU angle
float computeIMUPIDControl(float angleError, float &integralSum, float lastAngleError) {
    integralSum += angleError; // Integral term
    float derivative = (angleError - lastAngleError); // Derivative term
    float output = (Kp * angleError) + (Ki * integralSum) + (Kd * derivative);
    return output;
}

// Additional setup for IMU readings
void setupIMU() {
    // IMU initialization code, assuming IMU setup is done here
}

void loopIMUControl() {
    // Placeholder loop for IMU control logic
    float angleError = readIMUAngle() - targetAngle; // Calculate angle error
    float output = computeIMUPIDControl(angleError, integralSum, lastAngleError);
    applyMotorOutput(output); // Use the output to control motors
    lastAngleError = angleError;
}
