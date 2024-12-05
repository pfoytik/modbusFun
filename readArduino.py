import serial
import time

# Initialize serial connection
def init_serial(port="/dev/ttyACM0", baud_rate=9600, timeout=1):
    try:
        ser = serial.Serial(port, baud_rate, timeout=timeout)
        print(f"Connected to {port} at {baud_rate} baud.")
        return ser
    except serial.SerialException as e:
        print(f"Failed to connect to {port}: {e}")
        return None

# Read data from Arduino
def read_from_arduino(ser):
    try:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            return float(line)
        else:
            return None
    except Exception as e:
        print(f"Error reading from serial: {e}")
        return None

# Send data to Arduino
def send_to_arduino(ser, message):
    try:
        ser.write(f"{message}\n".encode('utf-8'))
    except Exception as e:
        print(f"Error sending to Arduino: {e}")

def main():
    # Replace '/dev/ttyUSB0' with the correct port if different
    ser = init_serial(port="/dev/ttyACM0", baud_rate=9600)

    if not ser:
        return

    try:
        while True:
            sine_value = read_from_arduino(ser)
            if sine_value is not None:
                print(f"Received: {sine_value}")

                if sine_value > 100:
                    #pass
                    send_to_arduino(ser, "1")
                else:
                    #pass
                    send_to_arduino(ser, "0")
            time.sleep(0.1)

            #data = read_from_arduino(ser)
            #if data:
            #    print(f"Received: {data}")
            #time.sleep(0.1)  # Adjust for your needs
    except KeyboardInterrupt:
        print("Exiting program.")
    finally:
        if ser and ser.is_open:
            ser.close()
            print("Serial connection closed.")

if __name__ == "__main__":
    main()


