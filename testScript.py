from pymodbus.client import ModbusTcpClient

#while(1):
  # Connect to the Modbus server
client = ModbusTcpClient('127.0.0.1', port=1502)
while(True):
  # Read holding registers starting at address 0, read 1 register
  response = client.read_holding_registers(0, 1)
  
  if response.isError():
      print(f"Error reading from Modbus server. {response}")
  else:
      print(f"Register Value: {response.registers[0]}")

# Close the connection
client.close()

