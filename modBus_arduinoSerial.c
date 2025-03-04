#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <modbus/modbus.h>
#include <math.h>

// Shared Data and Synchronization
volatile float shared_values[5] = {0}; // Array for parsed values
volatile bool active_state = false;
pthread_mutex_t data_mutex;
pthread_cond_t data_cond;
bool data_available = false;

// Serial Port Configuration
#define SERIAL_PORT "/dev/ttyACM0" // Replace with your port if different
#define BAUD_RATE B9600           // Baud rate for serial communication

// Modbus Configuration
#define SERVER_ID 1
#define HOLDING_REGISTERS_COUNT 10
#define AMPLITUDE 32767

// Serial Port Functions
int configure_serial(const char *port) {
    int serial_fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd == -1) {
        perror("Error opening serial port");
        return -1;
    }

    struct termios options;
    tcgetattr(serial_fd, &options);           
    cfsetispeed(&options, BAUD_RATE);         
    cfsetospeed(&options, BAUD_RATE);         
    options.c_cflag |= (CLOCAL | CREAD);      
    options.c_cflag &= ~PARENB;               
    options.c_cflag &= ~CSTOPB;               
    options.c_cflag &= ~CSIZE;                
    options.c_cflag |= CS8;                   
    options.c_cflag &= ~CRTSCTS;              
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
    options.c_iflag &= ~(IXON | IXOFF | IXANY); 
    options.c_oflag &= ~OPOST;                

    tcsetattr(serial_fd, TCSANOW, &options);  
    return serial_fd;
}

void parse_csv(const char *input, float *values, int max_values) {
    char *token;
    char buffer[128];
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    int index = 0;
    token = strtok(buffer, ";");
    while (token != NULL && index < max_values) {
        values[index] = atof(token);
        token = strtok(NULL, ";");
        index++;
    }
}

// Serial Data Reading Thread
void* read_serial_data(void* arg) {
    int serial_fd = configure_serial(SERIAL_PORT);
    if (serial_fd == -1) {
        pthread_exit(NULL);
    }

    char buffer[128];
    float local_values[5];
    int bytes_read;
    bool last_state = !active_state;

    while (1) {
        
        // **Check if state changed and send update to Arduino**
        if (active_state != last_state) {
            char command = active_state ? '1' : '0';  // '1' = Active, '0' = Inactive
            write(serial_fd, &command, 1);  // Send 1-byte command
            printf("Sent state command to Arduino: %c\n", command);
            last_state = active_state;
        }

        memset(buffer, 0, sizeof(buffer));
        bytes_read = read(serial_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Received: %s\n", buffer);

            parse_csv(buffer, local_values, 5);

            pthread_mutex_lock(&data_mutex);
            for (int i = 0; i < 5; i++) {
                shared_values[i] = local_values[i];
            }
            data_available = true;
            pthread_cond_signal(&data_cond);
            pthread_mutex_unlock(&data_mutex);
        }
        usleep(100000);
    }

    close(serial_fd);
    return NULL;
}

// Modbus Server Thread
void* modbus_server(void* arg) {
    modbus_t *ctx = modbus_new_tcp("0.0.0.0", 502);
    if (ctx == NULL) {
        fprintf(stderr, "Unable to create the Modbus server context\n");
        pthread_exit(NULL);
    }

    modbus_set_slave(ctx, SERVER_ID);
    int server_socket = modbus_tcp_listen(ctx, 1);
    if (server_socket == -1) {
        fprintf(stderr, "Unable to listen on Modbus TCP\n");
        modbus_free(ctx);
        pthread_exit(NULL);
    }

    modbus_mapping_t *mb_mapping = modbus_mapping_new(0, 0, HOLDING_REGISTERS_COUNT, HOLDING_REGISTERS_COUNT);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate modbus mapping: %s\n", modbus_strerror(errno));
        close(server_socket);
        modbus_free(ctx);
        pthread_exit(NULL);
    }

    int serial_fd = configure_serial(SERIAL_PORT);
    if(serial_fd == -1) {
        fprintf(stderr, "Serial port error, No Arduino control availabl. \n");
    }

    while (1) {
        int client_socket = modbus_tcp_accept(ctx, &server_socket);
        if (client_socket == -1) {
            fprintf(stderr, "Failed to accept Modbus connection\n");
            break;
        }

        printf("Client connected.\n");

        while (1) {
            
            // Process Modbus requests
            uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
            int rc = modbus_receive(ctx, query);
            if (rc > 0) {
                int reg_addr = MODBUS_GET_INT16_FROM_INT8(query, 2);
                int value = MODBUS_GET_INT16_FROM_INT8(query, 4);
                
                // Detect if a client writes to register 5 (state control)
                if (query[1] == MODBUS_FC_WRITE_SINGLE_REGISTER) {
                    active_state = (value==1);
                    printf("Received command: %d. System is now %s.\n", value, active_state ? "ACTIVE" : "INACTIVE");
    
                    if (serial_fd != -1) {
                        char command = active_state ? '1' : '0';  // '1' = Active, '0' = Inactive
                        write(serial_fd, &command, 1);  // Send 1-byte command
                        printf("Sent state command to Arduino: %c\n", command);

                    }
                }

                // Respond to Modbus client
                modbus_reply(ctx, query, rc, mb_mapping);
                printf("Updated Modbus registers.\n");
            } else if (rc == -1) {
                printf("Client disconnected or error occurred: %s\n", modbus_strerror(errno));
                break;
            }
        }

        close(client_socket);
    }

    if (serial_fd != -1) {
        close(serial_fd);
    }
    
    modbus_mapping_free(mb_mapping);
    close(server_socket);
    modbus_free(ctx);

    return NULL;
}

// Main Function
int main() {
    pthread_t serial_thread, modbus_thread;

    // Initialize mutex and condition variable
    pthread_mutex_init(&data_mutex, NULL);
    pthread_cond_init(&data_cond, NULL);

    // Create threads
    pthread_create(&serial_thread, NULL, read_serial_data, NULL);
    pthread_create(&modbus_thread, NULL, modbus_server, NULL);

    // Join threads
    pthread_join(serial_thread, NULL);
    pthread_join(modbus_thread, NULL);

    // Cleanup
    pthread_mutex_destroy(&data_mutex);
    pthread_cond_destroy(&data_cond);

    return 0;
}
