from spacepackets.ccsds.spacepacket import SpacePacketHeader, PacketType
import serial

# Initialize serial port
# ttyUSB0 for cmd/tlm spacepackets bytes
# ttyACM0 (usb-c) for logging messages
ser = serial.Serial('/dev/ttyUSB0', baudrate=115200)

sync_bytes = b"\x35\x2E\xF8\x53"

while True:
    # receive telemetry
    received_byte = ser.read(1)
    print(received_byte)
  
    # # Keep stalling til we receive the 4 sync bytes in order
    # if received_byte == sync_bytes[0]:
    #     sync_temp = received_byte
    #     for i in range(len(1, received_byte)):
    #         received_byte = ser.read(1)
    #         if received_byte != sync_bytes[i]:
    #             break
    #         sync_temp += received_byte
    #     if sync_temp != sync_bytes:
    #         continue
    #     print(b"received sync bytes: " + sync_temp)
    # else:
    #     continue

    # # Parse Spacepacket
    # raw_bytes = ser.read(6)
    # sp = SpacePacketHeader.unpack(raw_bytes)
    # print(sp)
    # print(raw_bytes)

# Close uart
ser.close()
