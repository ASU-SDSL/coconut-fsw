# GSE
The GSE task.


## Type Definitions

### telemetry_queue_transmission_t
`telemetry_queue_transmission_t` is a struct type that represents a telemetry packet. These fields must be provided:  
* apid: apid of packet
* payload_buffer: buffer for payload
* payload_size: size of the payload

## Globals

### uart0_queue

## API Functions

<details><summary><big>uart_queue_message()</big></summary>

#### Description
> Sends telemetry packet over serial line.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | buffer | char* | payload buffer |
> | size | size_t | payload size |

#### Returns
> None

</details>


## Internal Functions
### Note: These functions should ***NEVER*** be run outside of `gse.c`

<details><summary><big>uart_on_rx()</big></summary>

#### Description
> Function for uart interrupt handler.

#### Parameters
> None

#### Returns
> None

</details>  

<details><summary><big>uart_initialize()</big></summary>

#### Description
> Initializes uart line on raspberry pi pico.

#### Parameters
> | Name | Type | Description |
> | ---- | ---- | ----------- |
> | uart_instance | uart_inst_t* | uart instance |
> | tx_pin | int | tx pin on pico |
> | rx_pin | int | rx pin on pico |
> | irq | int | number of interrupt |

#### Returns
> None

</details>  
