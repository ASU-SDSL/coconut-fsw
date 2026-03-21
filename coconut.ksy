meta:
  id: heartbeat_packet
  endian: be
  bit-endian: be

seq:
  - id: syncbytes
    type: u4

  - id: ccsds
    type: ccsds_primary_header

  - id: callsign
    type: str
    size: 64
    encoding: ASCII

  - id: payload_state
    type: u1

  - id: payload
    type: heartbeat_payload

types:
  ccsds_primary_header:
    meta:
      bit-endian: be
    seq:
      - id: version
        type: b3

      - id: packet_type
        type: b1

      - id: sec_hdr_flag
        type: b1

      - id: apid
        type: b11

      - id: seq_flags
        type: b2

      - id: seq_cnt
        type: b14

      - id: data_len
        type: u2

  heartbeat_payload:
    seq:
      - id: uptime
        type: u4
        meta:
          endian: le

      - id: hour
        type: u1

      - id: minute
        type: u1

      - id: second
        type: u1

      - id: month
        type: u1

      - id: date
        type: u1

      - id: year
        type: u1

      - id: rtc_temp
        type: f4
        meta:
          endian: le

      - id: ina0
        type: ina_channel

      - id: ina1
        type: ina_channel

      - id: ina2
        type: ina_channel

      - id: ina3
        type: ina_channel

      - id: ina4
        type: ina_channel

      - id: ina5
        type: ina_channel

      - id: battery_voltage
        type: f4
        meta:
          endian: le

      - id: battery_charge_percentage
        type: f4
        meta:
          endian: le

      - id: mag_x
        type: s2
        meta:
          endian: le

      - id: mag_y
        type: s2
        meta:
          endian: le

      - id: mag_z
        type: s2
        meta:
          endian: le

      - id: mag_temp
        type: s2
        meta:
          endian: le

      - id: vega_ant_switches
        type: u1

      - id: temp_eps
        type: s2
        meta:
          endian: le

      - id: temp_xm
        type: s2
        meta:
          endian: le

      - id: temp_xp
        type: s2
        meta:
          endian: le

      - id: temp_ym
        type: s2
        meta:
          endian: le

      - id: temp_yp
        type: s2
        meta:
          endian: le

      - id: temp_zm
        type: s2
        meta:
          endian: le

      - id: temp_zp
        type: s2
        meta:
          endian: le

      - id: rfm_state
        type: s2
        meta:
          endian: le

      - id: sx_state
        type: s2
        meta:
          endian: le

      - id: which_radio
        type: u1

      - id: command_count
        type: u4
        meta:
          endian: le

      - id: boot_count
        type: u4
        meta:
          endian: le

  ina_channel:
    meta:
      endian: le
    seq:
      - id: shunt
        type: u2

      - id: vbus
        type: u2

      - id: power
        type: u2

      - id: current
        type: u2