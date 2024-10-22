import matplotlib.pyplot as plt

# Data for the calibration curve
sensor_output = [ 47, 76,  150, 157, 176]
measured_distance = [  45, 76.2, 142.8, 161, 171]

# Plot the data
plt.figure(figsize=(8,6))
plt.plot(sensor_output, measured_distance, 'o-', label='Calibration Data')
plt.xlabel('Sensor Output (mm)')
plt.ylabel('Measured Distance (mm)')
plt.title('Calibration Curve for VL6180X Distance Sensor')
plt.grid(True)
plt.legend()
plt.show()
