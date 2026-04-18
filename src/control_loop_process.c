#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <sys/syspage.h>
#include <spawn.h>
#include <signal.h>

typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float mx, my, mz;
    uint64_t timestamp;
} imu_data_t;

int main() {

    name_attach_t *attach;
    imu_data_t data;
    int rcvid;

    pid_t imu_pid = -1;

    // Create channel
    attach = name_attach(NULL, "imu_channel", 0);
    if (attach == NULL) {
        perror("name_attach");
        return -1;
    }

    // For time conversion
    uint64_t cycles_per_sec = SYSPAGE_ENTRY(qtime)->cycles_per_sec;

    // Jitter variables
    double prev_latency = 0;
    int first_sample = 1;

    // CPU timing variables
    uint64_t work_start, work_end;

    
    int warmup = 5;
    int synced = 0;
    int64_t offset_cycles = 0;

    while (1) {

        uint64_t timeout_ns = 100 * 1000000;
        TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_RECEIVE, NULL, &timeout_ns, NULL);

        rcvid = MsgReceive(attach->chid, &data, sizeof(data), NULL);

        if (rcvid > 0) {

            work_start = ClockCycles();
            uint64_t now = ClockCycles();

            if (data.timestamp == 0 || data.timestamp > now) {
                printf("[CONTROL] Invalid timestamp\n");
                MsgReply(rcvid, 0, NULL, 0);
                continue;
            }


            if (warmup > 0) {
                warmup--;
                MsgReply(rcvid, 0, NULL, 0);
                continue;
            }

            if (!synced) {
                offset_cycles = now - data.timestamp;
                synced = 1;
                printf("[CONTROL] Time synchronized\n");
                MsgReply(rcvid, 0, NULL, 0);
                continue;
            }


            double latency_ms =
                (double)((now - data.timestamp) - offset_cycles)
                / cycles_per_sec * 1000;

            //  JITTER
            double jitter = 0;
            if (!first_sample) {
                jitter = latency_ms - prev_latency;
            }
            first_sample = 0;
            prev_latency = latency_ms;

            printf("[CONTROL] Latency = %.3f ms | Jitter = %.3f ms\n",
                   latency_ms, jitter);

            printf("[CONTROL] ACC: %.2f %.2f %.2f\n",
                   data.ax, data.ay, data.az);

            printf("[CONTROL] GYR: %.2f %.2f %.2f\n",
                   data.gx, data.gy, data.gz);

            MsgReply(rcvid, 0, NULL, 0);

            work_end = ClockCycles();

            double work_time_ms =
                (double)(work_end - work_start) / cycles_per_sec * 1000;

            double cpu_usage = (work_time_ms / 100.0) * 100.0;

            printf("[CONTROL] CPU Usage ≈ %.2f%% | Work Time = %.3f ms\n\n",
                   cpu_usage, work_time_ms);
        }

        else {
            printf("[CONTROL] Waiting for IMU data...\n");

            imu_pid = spawnl(P_NOWAIT,
                "../../../read_i2c/build/aarch64le-debug/read_i2c",
                "read_i2c",
                NULL);

            if (imu_pid == -1) {
                printf("Restarting imu....\n");
            } else {
                printf("[CONTROL] IMU started with PID %d\n", imu_pid);
            }
        }
    }

    return 0;
}