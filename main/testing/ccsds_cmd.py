from spacepackets.ccsds.spacepacket import SpacePacketHeader, PacketType
import serial

# Initialize serial port
# ttyUSB0 for cmd/tlm spacepackets bytes
# ttyACM0 (usb-c) for logging messages
ser = serial.Serial('/dev/ttyUSB0', baudrate=115200)

# Create spacepacket
spacepacket_header = SpacePacketHeader(
    packet_type=PacketType.TC, apid=0x01, seq_count=0, data_len=5
)
header_bytes = spacepacket_header.pack()

# Create full payload
payload = b"\x35\x2E\xF8\x53"
payload += header_bytes
payload += b"WOW!"
payload += b"\x00"

# Send spacepacket uart port
ser.write(payload)

# Close uart
ser.close()
