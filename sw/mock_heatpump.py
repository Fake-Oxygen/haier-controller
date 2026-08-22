import time
import serial

PORT = "/dev/ttyUSB1"
BAUDRATE = 9600
SLAVE_ID = 0x11  # Device 17


def calc_crc(data: bytes) -> bytes:
    """Calculate Modbus RTU CRC16 (Low Byte, High Byte)."""
    crc = 0xFFFF
    for pos in data:
        crc ^= pos
        for _ in range(8):
            if (crc & 0x0001) != 0:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return bytes([crc & 0xFF, (crc >> 8) & 0xFF])


def make_modbus_response(slave: int, func: int, reg_values: list[int]) -> bytes:
    """Build a raw Modbus RTU response packet."""
    byte_count = len(reg_values) * 2
    payload = bytearray([slave, func, byte_count])

    for val in reg_values:
        payload.append((val >> 8) & 0xFF)  # High byte
        payload.append(val & 0xFF)         # Low byte

    payload.extend(calc_crc(payload))
    return bytes(payload)


# Pre-generate mock response frames matching your heat pump lengths
FRAME_R101 = make_modbus_response(SLAVE_ID, 0x03, [40, 52, 10, 0, 100, 1])     # 6 regs  -> 0x030c
FRAME_R141 = make_modbus_response(SLAVE_ID, 0x03, [i * 10 for i in range(16)])# 16 regs -> 0x0320
FRAME_R201 = make_modbus_response(SLAVE_ID, 0x03, [250])                     # 1 reg   -> 0x0302
FRAME_R241 = make_modbus_response(SLAVE_ID, 0x03, [i + 1 for i in range(22)]) # 22 regs -> 0x032c


def main():
    ser = serial.Serial(
        port=PORT,
        baudrate=BAUDRATE,
        parity=serial.PARITY_EVEN,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1,
    )

    print(f"Connected to {PORT}. Transmitting test Modbus frames for Slave {SLAVE_ID}...")

    try:
        while True:
            print("Sending R101 response (6 regs)...")
            ser.write(FRAME_R101)
            ser.flush()
            time.sleep(0.5)

            print("Sending R141 response (16 regs)...")
            ser.write(FRAME_R141)
            ser.flush()
            time.sleep(0.5)

            print("Sending R201 response (1 reg)...")
            ser.write(FRAME_R201)
            ser.flush()
            time.sleep(0.5)

            print("Sending R241 response (22 regs)...")
            ser.write(FRAME_R241)
            ser.flush()
            time.sleep(0.5)

            print("--- Cycle Complete ---")
            time.sleep(2.0)

    except KeyboardInterrupt:
        print("\nStopped frame emitter.")
    finally:
        ser.close()


if __name__ == "__main__":
    main()
