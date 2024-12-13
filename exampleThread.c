#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Shared data and synchronization primitives
volatile int shared_data = 0;
pthread_mutex_t data_mutex;
pthread_cond_t data_cond;
bool data_available = false;

// Mock function to read data from Arduino (replace with actual implementation)
int read_from_serial() {
    // Replace this with your actual serial reading logic
    static int counter = 0;
    return counter++; // Incremental mock data
}

// Serial data reading thread
void* read_serial_data(void* arg) {
    while (1) {
        int new_data = read_from_serial(); // Call your serial reading function
        
        pthread_mutex_lock(&data_mutex);
        shared_data = new_data;
        data_available = true;
        pthread_cond_signal(&data_cond);
        pthread_mutex_unlock(&data_mutex);

        printf("Read data: %d\n", new_data);
        sleep(1); // Adjust based on serial read frequency
    }
    return NULL;
}

// Modbus client thread
void* update_modbus_client(void* arg) {
    while (1) {
        pthread_mutex_lock(&data_mutex);
        while (!data_available) {
            pthread_cond_wait(&data_cond, &data_mutex);
        }
        int data_to_send = shared_data;
        data_available = false;
        pthread_mutex_unlock(&data_mutex);

        // Simulate updating Modbus client
        printf("Updating Modbus client with data: %d\n", data_to_send);
        // Replace with actual Modbus communication logic
    }
    return NULL;
}

int main() {
    pthread_t serial_thread, modbus_thread;

    // Initialize mutex and condition variable
    pthread_mutex_init(&data_mutex, NULL);
    pthread_cond_init(&data_cond, NULL);

    // Create threads
    pthread_create(&serial_thread, NULL, read_serial_data, NULL);
    pthread_create(&modbus_thread, NULL, update_modbus_client, NULL);

    // Join threads (optional if the main thread should wait)
    pthread_join(serial_thread, NULL);
    pthread_join(modbus_thread, NULL);

    // Cleanup
    pthread_mutex_destroy(&data_mutex);
    pthread_cond_destroy(&data_cond);

    return 0;
}
