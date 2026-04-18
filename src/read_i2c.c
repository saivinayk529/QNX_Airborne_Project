#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/neutrino.h>
#include <sys/dispatch.h>

#include "rpi_i2c.h"

#define MPU9250_ADDR 0x68
#define WHO_AM_I     0x75
#define PWR_MGMT_1   0x6B

#define I2C_BUS 1

// Data structure for IPC
typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    uint64_t timestamp;
} imu_data_t;

int main(void) {

    uint8_t rawdata;
    float ax, ay, az;
    float gx, gy, gz;

    int coid;              // connection ID
    imu_data_t data;       // struct to send

  //  puts("MPU9250 Test");

    // Connect to control process
    coid = name_open("imu_channel", 0);

    if (coid == -1) {
      //  printf("Error: Could not connect to control process\n");
        return EXIT_FAILURE;
    }

    printf("[IMU] Connected to control process\n");

    // Check sensor
    if (smbus_read_byte_data(I2C_BUS, MPU9250_ADDR, WHO_AM_I, &rawdata) != I2C_SUCCESS) {
      //  puts("MPU9250 not detected");
      //  return EXIT_FAILURE;
    }

   // printf("WHO_AM_I: 0x%X\n", rawdata);
/*

    void mpu9250_init() {
        // Wake up MPU9250 (clear sleep bit)
        smbus_write_byte_data(I2C_BUS, MPU9250_ADDR, PWR_MGMT_1, 0x00);
    }
*/

    // 🔹 Main loop
    while (1) {

        // Read sensor data
        read_accel(&ax, &ay, &az);
        read_gyro(&gx, &gy, &gz);

        // Print locally (optional)
   //     printf("[IMU] Accel: X=%.2f Y=%.2f Z=%.2f g\n", ax, ay, az);
     //   printf("[IMU] Gyro : X=%.2f Y=%.2f Z=%.2f dps\n", gx, gy, gz);

        // 🔹 Fill struct
        data.ax = ax;
        data.ay = ay;
        data.az = az;

        data.gx = gx;
        data.gy = gy;
        data.gz = gz;
        data.timestamp = ClockCycles();
        // Send to control process
        if (MsgSend(coid, &data, sizeof(data), NULL, 0) == -1) {
          //  printf("Error sending data to control\n");
        }

        delay(10);  // your existing delay (10 ms)
    }

    return EXIT_SUCCESS;
}