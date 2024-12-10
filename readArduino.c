#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#define SERIAL_PORT "/dev/ttyACM0" // Replace with your port if different
#define BAUD_RATE B9600           // Baud rate for serial communication

// Function to configure the serial port
int configure_serial(const char *port) {
    int serial_fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd == -1) {
        perror("Error opening serial port");
        return -1;
    }

    struct termios options;
    tcgetattr(serial_fd, &options);           // Get current port settings
    cfsetispeed(&options, BAUD_RATE);         // Set input baud rate
    cfsetospeed(&options, BAUD_RATE);         // Set output baud rate
    options.c_cflag |= (CLOCAL | CREAD);      // Enable receiver and set local mode
    options.c_cflag &= ~PARENB;               // No parity
    options.c_cflag &= ~CSTOPB;               // 1 stop bit
    options.c_cflag &= ~CSIZE;                // Clear current data size setting
    options.c_cflag |= CS8;                   // 8 data bits
    options.c_cflag &= ~CRTSCTS;              // Disable hardware flow control
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input mode
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control
    options.c_oflag &= ~OPOST;                // Raw output mode

    tcsetattr(serial_fd, TCSANOW, &options);  // Apply settings
    return serial_fd;
}

// Function to parse comma-separated values
void parse_csv(const char *input, float *values, int max_values) {
    char *token;
    char buffer[128];
    strncpy(buffer, input, sizeof(buffer) - 1); // Copy input to buffer
    buffer[sizeof(buffer) - 1] = '\0';          // Null-terminate the buffer

    int index = 0;
    token = strtok(buffer, ",");
    while (token != NULL && index < max_values) {
        values[index] = atof(token); // Convert string to float
        token = strtok(NULL, ",");
        index++;
    }
}

int main() {
    int serial_fd = configure_serial(SERIAL_PORT);
    if (serial_fd == -1) {
        return 1; // Exit if serial port setup failed
    }

    char buffer[128];        // Buffer for incoming data
    float values[5];         // Array to store parsed values
    int bytes_read;

    printf("Listening on %s...\n", SERIAL_PORT);

    while (1) {
        // Read data from the serial port
        memset(buffer, 0, sizeof(buffer));
        bytes_read = read(serial_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null-terminate the received string
            printf("Received: %s\n", buffer);

            // Parse the CSV data
            parse_csv(buffer, values, 5);
            printf("Parsed values: ");
            for (int i = 0; i < 5; i++) {
                printf("%.2f ", values[i]);
            }
            printf("\n");
        }

        usleep(100000); // Sleep for 100ms
    }

    close(serial_fd); // Close the serial port
    return 0;
}
