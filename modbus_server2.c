#include <modbus/modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#define SERVER_ID 1
#define HOLDING_REGISTERS_COUNT 10
#define PI 3.14159265358979323846
#define SINE_FREQUENCY 0.1
#define AMPLITUDE 32767


int main() {
    srand(time(NULL));
    modbus_t *ctx = modbus_new_tcp("0.0.0.0", 502);
    if (ctx == NULL) {
        fprintf(stderr, "Unable to create the Modbus server context\n");
        return -1;
    }
    
    modbus_set_slave(ctx, SERVER_ID);
    int server_socket = modbus_tcp_listen(ctx, 1);
    if (server_socket == -1) {
        fprintf(stderr, "Unable to listen on Modbus TCP\n");
        modbus_free(ctx);
        return -1;
    }

    modbus_mapping_t *mb_mapping = modbus_mapping_new(0, 0, HOLDING_REGISTERS_COUNT, HOLDING_REGISTERS_COUNT);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate modbus mapping: %s\n", modbus_strerror(errno));
        close(server_socket);
        modbus_free(ctx);
        return -1;
    }

    double sine_time = 0.0;
    while (1) {
        int client_socket = modbus_tcp_accept(ctx, &server_socket);
        if (client_socket == -1) {
            fprintf(stderr, "Failed to accept Modbus connection\n");
            break;
        }

        printf("Client connected.\n");

        while (1) {
            int random_value = rand() % 100;
	    double sine_value = AMPLITUDE * sin(SINE_FREQUENCY * sine_time);
            //mb_mapping->tab_registers[0] = random_value;
	    mb_mapping->tab_registers[0] = (uint16_t)(sine_value + AMPLITUDE);
        mb_mapping->tab_input_registers[0] = (uint16_t)(sine_value + AMPLITUDE);

	    sine_time += 0.1;

            uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
            int rc = modbus_receive(ctx, query);
            if (rc > 0) {
                printf("Received query of length: %d\n", rc);
                modbus_reply(ctx, query, rc, mb_mapping);
                printf("Random value %d served to Modbus client\n", mb_mapping->tab_registers[0]);
            } else if (rc == -1) {
                printf("Client disconnected or error occurred: %s\n", modbus_strerror(errno));
                break;
            }
        }

        close(client_socket);
    }

    modbus_mapping_free(mb_mapping);
    close(server_socket);
    modbus_free(ctx);

    return 0;
}

