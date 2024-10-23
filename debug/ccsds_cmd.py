from spacepackets.ccsds.spacepacket import SpacePacketHeader, PacketType
import serial
import sys

# Initialize serial port
# ttyUSB0 for cmd/tlm spacepackets bytes
# ttyACM0 (usb-c) for logging messages
ser = serial.Serial('/dev/ttyACM1', baudrate=115200)

# ## Create change telem rate payload
# spacepacket_header = SpacePacketHeader(
#     packet_type=PacketType.TC, apid=0x01, seq_count=0, data_len=3)
# header_bytes = spacepacket_header.pack()
# payload = b"\x35\x2E\xF8\x53"
# payload += header_bytes
# payload += int.to_bytes(1, 4, byteorder='little') # ms
# ser.write(payload)

# ### List STEVE task payload
# spacepacket_header = SpacePacketHeader(
#     packet_type=PacketType.TC, apid=0x04, seq_count=0, data_len=0)
# header_bytes = spacepacket_header.pack()
# payload = b"\x35\x2E\xF8\x53"
# payload += header_bytes
# payload += b"\x69"

### Touch file
sync_bytes = b"\x35\x2E\xF8\x53"
payload = b"/lol.txt" 
payload += (b"\x00" * (0x101 - len(payload)))
spacepacket_header = SpacePacketHeader(
    packet_type=PacketType.TC, apid=10, seq_count=0, data_len=len(payload)-1)
header_bytes = spacepacket_header.pack()
final_cmd = sync_bytes + header_bytes + payload
ser.write(final_cmd)

### Ls file
sync_bytes = b"\x35\x2E\xF8\x53"
payload = b"/"
payload += (b"\x00" * (0x101 - len(payload)))
spacepacket_header = SpacePacketHeader(
    packet_type=PacketType.TC, apid=5, seq_count=0, data_len=len(payload)-1)
header_bytes = spacepacket_header.pack()
final_cmd = sync_bytes + header_bytes + payload
# print(final_cmd)
ser.write(final_cmd)

# Read output
while True:
    cc = ser.read(1)
    sys.stderr.buffer.write(cc)

# Close uart
ser.close()
