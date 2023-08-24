from spacepackets.ccsds.spacepacket import SpacePacketHeader, PacketType
import serial

# Initialize serial port
# ttyUSB0 for cmd/tlm spacepackets bytes
# ttyACM0 (usb-c) for logging messages
ser = serial.Serial('/dev/ttyUSB0')

# Create spacepacket
spacepacket_header = SpacePacketHeader(
    packet_type=PacketType.TC, apid=0x01, seq_count=0, data_len=0
)
header_bytes = spacepacket_header.pack()

# Create full payload
payload = header_bytes
payload += "A" * 69
payload += "\x00"

# Send spacepacket uart port
ser.write(payload)

# Close uart
ser.close()
