# COMMAND
The command task.


## Type Definitions

### command_apid_t
`command_api_t` is an enum that represents the different commands that can be received. These are:  
* STORE_GROUNDNODE_DATA
* CHANGE_HEARTBEAT_TELEM_RATE
* REQUEST_DOWNLINK_GROUNDNODE_DATA
* REQUEST_DOWNLINK_TELEMETRY_DATA

### command_byte_t
`command_byte_t` is a type representing a single byte of a command packet. The underlying type is char.

## Globals

### command_byte_queue
`command_byte_queue` is the global command queue. It will queue up bytes, allowing the user to parse them as packets.


## API Functions

<details><summary><big>receive_command_byte_from_isr()</big></summary>

#### Description
> Receives a single byte from interrupt. Only use this function from interrupts.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | ch | char | byte that is received from isr |

#### Returns
> None

</details>


## Internal Functions
### Note: These functions should ***NEVER*** be run outside of `command.c`

<details><summary><big>parse_command_packet()</big></summary>

#### Description
> Parses a command packet.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | header | ccsds_header_t | packet header |
> | payload_buf | uint8_t* | payload buffer |
> | payload_size | uint32_t | size of payload |

#### Returns
> None

</details>

<details><summary><big>parse_radio_packet()</big></summary>

#### Description
> Parse radio packet.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | packet | uint8_t* | packet |
> | packet_size | size_t | size of packet |

#### Returns
> None

</details>