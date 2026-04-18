#                                             Fault Tolerance System for Aerial Vehicles

##  Team: Team Airborne
         
           MGIT
         > Saathvik Vardhan
         > Sai Vinay
         > Rahul


##  Problem Statement

Modern aerial systems like drones and eVTOL platforms rely on RTOS such as VxWorks, INTEGRITY, PikeOS, and Deos. These systems are:

Tightly coupled
Less flexible
Hard to scale for high-compute applications

Our Goal

To overcome these limitations by designing an open hardware, modular, fault-tolerant flight control system using QNX Neutrino RTOS on Raspberry Pi 4


##  Objective

Build a real-time fault tolerant flight control system
Ensure deterministic communication
Implement fault tolerance with self-healing
Analyze latency, jitter, and CPU usage
Demonstrate modular process-based design


##  Solution Overview

We developed a QNX RTOS-based flight controller that:

Uses microkernel architecture
Implements message passing (IPC)
Supports process-level fault isolation
Enables automatic recovery (restart mechanism)



##  Technologies Used

Controller
Raspberry Pi 4 running QNX Neutrino RTOS

Sensors
MPU9250 → IMU (orientation)
NEO-M8N → GPS
MS5611 → Barometer (altitude)
HMC5883L → Magnetometer (heading)

Actuators
BLDC Motors via ESC

Interfaces
I2C → Barometer, Magnetometer
SPI → IMU
UART → GPS
iBUS → Receiver



##  Workflow

1. System Initialization
The QNX RTOS image is built using a .build file.
The executables (read_i2c and control) are included in the image.
On boot, the control process is started automatically.
The control process then spawns the IMU process (read_i2c).


3. IMU Data Acquisition (read_i2c Process)
The IMU process continuously reads accelerometer and gyroscope data using I2C.

Before sending data, a timestamp is added using:

data.timestamp = ClockCycles();
The data is sent to the control process using QNX message passing (MsgSend).


3. Inter-Process Communication
The control process creates a channel using name_attach().
The IMU process connects to this channel using name_open().
Data transfer happens using:
MsgSend() from IMU
MsgReceive() in control
MsgReply() to complete the communication


4. Control Process Functionality

The control process performs multiple tasks:

a. Receiving Data
Receives IMU data using MsgReceive().
Prints accelerometer and gyroscope values.

b. Latency Measurement Calculates latency using:
latency = (current_time - data.timestamp) / cycles_per_sec;
Displays latency in milliseconds.

c. Jitter Calculation
Jitter is calculated as the variation in latency between consecutive samples.

d. Time Monitoring
The control process continuously tracks the last received timestamp.
Uses ClockCycles() to measure time difference.

5. Fault Detection

If no data is received for 5 seconds:

if (elapsed_time >= 5 seconds)
A fault is detected.

The system prints:

No IMU data for 5 seconds → FAULT

6. Fault Recovery
The control process attempts to recover by:
Killing the existing IMU process using kill()
Restarting it using spawn()
This ensures the system continues functioning even after failure.

7. CPU Load Testing
A separate load process (infinite loop) can be created to simulate high CPU usage.
Under load:
Latency increases
Jitter increases
This helps analyze real-time performance.

8. Performance Monitoring

The system outputs:

Latency (IMU to control delay)
Jitter (variation in latency)
Fault messages
Restart events

9. Final System Behavior
Normal condition:
Stable latency
Low jitter
Under fault:
IMU stops
Control detects within 5 seconds
IMU is restarted automatically
Under load:
Increased latency and jitter
System remains functional

10. Conclusion

This project demonstrates:

Use of QNX message passing
Real-time data handling
Fault detection and recovery
Performance monitoring (latency and jitter)

The system is designed to be modular, reliable, and suitable for real-time embedded applications like aerial vehicles.


##  RTOS Concepts Used

* Deterministic control loops
* Priority-based scheduling (Control loop = highest priority)
* Inter-process communication (Message passing)
* Resource management (I²C access)
* Periodic timers for control loops


##  Task Scheduling

* High Priority: Control Loop, IMU Data Processing
* Medium Priority: Sensor Fusion, GPS Processing
* Low Priority: Logging, Diagnostics, UI


##  Inter-Task Communication

* Message Passing
* Shared Memory


##  Results (Simulation)

* Deterministic execution of control loop
* Stable simulated sensor data processing
* Motor output generation based on control logic
* Efficient communication between modules


##  Test Cases

* Sensor data simulation validation
* Control loop timing verification
* Thread priority behavior testing
* IPC communication testing


## Logging & Visualization

* System logs for debugging
* Status indicators for module activity


## Interfaces Used

* I²C
* UART
* GPIO



## Project Structure

* `/src` → Source code
* `/docs` → Architecture diagrams
* `/presentation` → PPT
* `/results` → Output screenshots

## Future Work

* Real hardware implementation with sensors
* Advanced sensor fusion (Kalman Filter)
* Autonomous navigation system
