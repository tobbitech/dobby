# Aidon HAN Message Parsing Example

This document explains how to parse HAN (Home Area Network) messages from Aidon smart electricity meters commonly used in Norway and other Nordic countries.

## Overview

Aidon meters send data over a serial interface (HAN port) at 2400 baud, 8N1. The messages follow the DLMS/COSEM standard and use OBIS (Object Identification System) codes to identify different measurement values.

## Message Structure

### 1. Frame Format

```
+------+--------+----------+------+----------+------+
| 0x7E | HEADER | HCS      | DATA | FCS      | 0x7E |
+------+--------+----------+------+----------+------+
```

- **0x7E**: Start/end flag byte
- **HEADER**: 6 bytes containing frame format info
- **HCS**: 2-byte Header Check Sequence (CRC16/X25)
- **DATA**: Variable length payload
- **FCS**: 2-byte Frame Check Sequence (CRC16/X25)
- **0x7E**: End flag

### 2. Header Structure (6 bytes)

```cpp
uint8_t header[6] = {
    0xA0,          // Frame format (0xA0 = Type 0, one byte length)
    0xXX,          // Frame length
    0x02,          // Destination address
    0x03,          // Source address  
    0x13,          // Control field
    0xXX           // HCS high byte
};
```

### 3. Data Structure

After the header comes the actual data payload containing OBIS codes and values:

```
- 9 bytes: LLC header and wrapper
- 1 byte: Data type (usually 0x01 for structure)
- 1 byte: Number of items/lines in the payload
- N x (OBIS code + value) pairs
```

## OBIS Codes

Common OBIS codes used by Aidon meters:

| OBIS Code | Description | Unit | Type |
|-----------|-------------|------|------|
| `01.01.00.02.81.FF` | OBIS list version identifier | - | String |
| `00.00.60.01.00.FF` | Meter ID (serial number) | - | String |
| `00.00.60.01.07.FF` | Meter type | - | String |
| `01.00.01.07.00.FF` | Active power import (instant) | W | int32 |
| `01.00.02.07.00.FF` | Active power export (instant) | W | int32 |
| `01.00.03.07.00.FF` | Reactive power import | VAr | int32 |
| `01.00.04.07.00.FF` | Reactive power export | VAr | int32 |
| `01.00.1F.07.00.FF` | Current L1 | A×10¹ | int16 |
| `01.00.33.07.00.FF` | Current L2 | A×10¹ | int16 |
| `01.00.47.07.00.FF` | Current L3 | A×10¹ | int16 |
| `01.00.20.07.00.FF` | Voltage L1 | V×10¹ | uint16 |
| `01.00.34.07.00.FF` | Voltage L2 | V×10¹ | uint16 |
| `01.00.48.07.00.FF` | Voltage L3 | V×10¹ | uint16 |
| `00.00.01.00.00.FF` | Meter clock/timestamp | - | DateTime |
| `01.00.01.08.00.FF` | Cumulative active import | kWh×10² | uint32 |
| `01.00.02.08.00.FF` | Cumulative active export | kWh×10² | uint32 |
| `01.00.03.08.00.FF` | Cumulative reactive import | kVArh×10² | uint32 |
| `01.00.04.08.00.FF` | Cumulative reactive export | kVArh×10² | uint32 |

¹ Values have 0.1 resolution (divide by 10 for actual value)
² Values have 0.01 resolution (divide by 100 for actual value)

## Data Types

Values in HAN messages use different data types:

| Type Code | Description | Length | Example |
|-----------|-------------|--------|---------|
| `0x09` | Octet string (timestamp, etc.) | Variable | Clock data |
| `0x0A` | Visible string (ASCII) | Variable | Meter ID |
| `0x06` | Unsigned 32-bit integer | 4 bytes | Energy values |
| `0x10` | Signed 16-bit integer | 2 bytes | Current |
| `0x12` | Unsigned 16-bit integer | 2 bytes | Voltage |

## Parsing Example

### Example Message (Hex)

```
7E A0 82 01 02 03 13 E6 E7 00 0F 00 00 00 00 00 
01 11 02 02 0F 00 16 27 00 00 09 0C 07 E6 0B 0E 
02 0F 1E 0D FF 80 00 00 02 09 06 01 01 00 02 81 
FF 0A 0B 41 49 44 4F 4E 5F 56 30 30 30 31 02 02 
0F 00 16 1B 00 00 09 11 07 E6 01 01 00 00 00 00 
FF 09 0C 07 E6 0B 0E 02 0F 1E 0D FF 80 00 00 02 
03 09 06 00 00 60 01 00 FF 0A 10 36 38 33 35 36 
33 31 35 32 30 30 30 30 30 30 30 02 02 0F 00 16 
23 00 00 09 0C 07 E6 0B 0E 02 0F 1E 0D FF 80 00 
00 02 03 09 06 00 00 60 01 07 FF 0A 04 36 35 31 
35 02 02 0F 00 16 1D 00 00 09 06 01 00 01 07 00 
FF 06 00 00 07 95 02 02 0F 00 16 21 00 00 09 06 
01 00 02 07 00 FF 06 00 00 00 00 02 02 0F 00 16 
41 00 00 09 06 01 00 1F 07 00 FF 10 00 0E FF FF
```

### Step-by-Step Parsing

1. **Check start flag**: `0x7E`
2. **Read header** (6 bytes): `A0 82 01 02 03 13`
3. **Verify header checksum**: Next 2 bytes (little-endian)
4. **Skip LLC header**: 9 bytes
5. **Read data type**: Usually `0x01` (structure)
6. **Read number of items**: How many OBIS code+value pairs follow
7. **For each item**:
   - Skip 4 bytes (type identifier)
   - Read 6-byte OBIS code
   - Read 1-byte variable type
   - Parse value based on type:
     - **String (0x0A)**: Read length byte, then string
     - **Uint32 (0x06)**: Read 4 bytes (big-endian)
     - **Int16 (0x10)**: Read 2 bytes (big-endian)
     - **Uint16 (0x12)**: Read 2 bytes (big-endian)
     - **DateTime (0x09)**: Read length, then parse date/time structure
   - Apply scaling factor if needed (e.g., divide by 10 for current/voltage)
8. **Verify frame checksum**: Last 2 bytes before end flag
9. **Check end flag**: `0x7E`

## Code Example

Here's a simplified parsing example:

```cpp
void parse_han_message(uint8_t* message, size_t length) {
    size_t i = 0;
    
    // Check start flag
    if (message[i++] != 0x7E) {
        Serial.println("Invalid start flag");
        return;
    }
    
    // Read and verify header
    uint8_t header[6];
    for (int j = 0; j < 6; j++) {
        header[j] = message[i++];
    }
    
    uint16_t header_checksum = message[i++] | (message[i++] << 8);
    uint16_t calc_header_checksum = crc16x25(header, 6);
    
    if (header_checksum != calc_header_checksum) {
        Serial.println("Header checksum error");
        return;
    }
    
    // Skip LLC header (9 bytes)
    i += 9;
    
    // Read data type and number of items
    uint8_t datatype = message[i++];
    uint8_t num_items = message[i++];
    
    // Parse each item
    for (int item = 0; item < num_items; item++) {
        // Skip type identifier
        i += 4;
        
        // Read OBIS code
        uint8_t obis_code[6];
        for (int j = 0; j < 6; j++) {
            obis_code[j] = message[i++];
        }
        
        // Read variable type
        uint8_t var_type = message[i++];
        
        // Parse value based on type
        if (var_type == 0x0A) {
            // String
            uint8_t str_len = message[i++];
            char str_value[str_len + 1];
            for (int j = 0; j < str_len; j++) {
                str_value[j] = message[i++];
            }
            str_value[str_len] = '\0';
            
            // Example: Check if this is meter ID
            if (obis_code[0] == 0x00 && obis_code[1] == 0x00 && 
                obis_code[2] == 0x60 && obis_code[3] == 0x01 &&
                obis_code[4] == 0x00 && obis_code[5] == 0xFF) {
                Serial.print("Meter ID: ");
                Serial.println(str_value);
            }
            
        } else if (var_type == 0x06) {
            // Uint32 (big-endian)
            uint32_t value = (message[i] << 24) | (message[i+1] << 16) | 
                            (message[i+2] << 8) | message[i+3];
            i += 4;
            i += 6; // Skip trailing bytes
            
            // Example: Check if this is active power import
            if (obis_code[0] == 0x01 && obis_code[1] == 0x00 && 
                obis_code[2] == 0x01 && obis_code[3] == 0x07 &&
                obis_code[4] == 0x00 && obis_code[5] == 0xFF) {
                Serial.print("Active import: ");
                Serial.print(value);
                Serial.println(" W");
            }
            
        } else if (var_type == 0x10) {
            // Int16 (big-endian)
            int16_t value = (message[i] << 8) | message[i+1];
            i += 2;
            i += 6; // Skip trailing bytes
            
            // Example: Check if this is current L1
            if (obis_code[0] == 0x01 && obis_code[1] == 0x00 && 
                obis_code[2] == 0x1F && obis_code[3] == 0x07 &&
                obis_code[4] == 0x00 && obis_code[5] == 0xFF) {
                float current = value / 10.0; // Apply scaling
                Serial.print("Current L1: ");
                Serial.print(current, 1);
                Serial.println(" A");
            }
            
        } else if (var_type == 0x12) {
            // Uint16 (big-endian)
            uint16_t value = (message[i] << 8) | message[i+1];
            i += 2;
            i += 6; // Skip trailing bytes
            
            // Example: Check if this is voltage L1
            if (obis_code[0] == 0x01 && obis_code[1] == 0x00 && 
                obis_code[2] == 0x20 && obis_code[3] == 0x07 &&
                obis_code[4] == 0x00 && obis_code[5] == 0xFF) {
                float voltage = value / 10.0; // Apply scaling
                Serial.print("Voltage L1: ");
                Serial.print(voltage, 1);
                Serial.println(" V");
            }
            
        } else if (var_type == 0x09) {
            // DateTime (octet string)
            uint8_t dt_len = message[i++];
            uint16_t year = (message[i] << 8) | message[i+1];
            i += 2;
            uint8_t month = message[i++];
            uint8_t day = message[i++];
            uint8_t dow = message[i++]; // Day of week
            uint8_t hour = message[i++];
            uint8_t minute = message[i++];
            uint8_t second = message[i++];
            // Skip remaining bytes
            i += (dt_len - 8);
            
            Serial.printf("Meter time: %04d-%02d-%02d %02d:%02d:%02d\n",
                         year, month, day, hour, minute, second);
        }
    }
    
    // Verify frame checksum
    uint16_t frame_checksum = message[i++] | (message[i++] << 8);
    uint16_t calc_frame_checksum = crc16x25(message + 1, i - 3);
    
    if (frame_checksum != calc_frame_checksum) {
        Serial.println("Frame checksum error");
        return;
    }
    
    // Check end flag
    if (message[i] != 0x7E) {
        Serial.println("Invalid end flag");
        return;
    }
    
    Serial.println("Message parsed successfully!");
}

// CRC16/X25 calculation
uint16_t crc16x25(uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    uint16_t crc16_table[] = {
        0x0000, 0x1081, 0x2102, 0x3183,
        0x4204, 0x5285, 0x6306, 0x7387,
        0x8408, 0x9489, 0xa50a, 0xb58b,
        0xc60c, 0xd68d, 0xe70e, 0xf78f
    };
    
    while (length--) {
        crc = (crc >> 4) ^ crc16_table[(crc & 0xf) ^ (*data & 0xf)];
        crc = (crc >> 4) ^ crc16_table[(crc & 0xf) ^ (*data++ >> 4)];
    }
    
    return ~crc;
}
```

## Hardware Connection

To read HAN messages from an Aidon meter:

1. **Connect to HAN port** on the meter (usually an RJ45 or RJ12 connector)
2. **Pin configuration** (typical):
   - Pin 1: +5V (or leave unconnected if using separate power)
   - Pin 2: Data (connect to RX pin on microcontroller)
   - Pin 3: Data Request (optional, can be tied high to enable continuous transmission)
   - Pin 4: GND
3. **Serial configuration**: 2400 baud, 8 data bits, no parity, 1 stop bit (8N1)
4. **Level shifting**: HAN port uses TTL or RS232 levels - check your meter's documentation

## Message Frequency

- **List 1** (short): Every 10 seconds (contains instant power and voltage/current)
- **List 2** (medium): Every hour (adds cumulative energy)
- **List 3** (long): Every 6 hours or on request (contains all available data)

## Troubleshooting

- **No data**: Check Data Request pin is high, verify baud rate and pin connections
- **Checksum errors**: Verify CRC16/X25 implementation, check for noise on serial line
- **Missing values**: Different meter models support different OBIS codes
- **Wrong values**: Check scaling factors and data types

## References

- [NVE AMS HAN Interface Description](https://www.nek.no/wp-content/uploads/2018/11/Teknisk-rapport-NVE-2016-grensesnittbeskrivelse-for-AMS-4-1.pdf)
- [DLMS/COSEM Standard](https://www.dlms.com/)
- [Norwegian HAN Information](https://hanporten.no/)

## License

This example is provided for educational purposes. See project LICENSE file for more information.
