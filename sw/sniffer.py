import logging
import serial
import time

# --- Configuration ---
PORT = "/dev/ttyUSB1"  # USB-to-RS485 adapter port
BAUDRATE = 9600
SLAVE_ADDR_HEX = "11"  # Slave ID 17 in Hex

logging.basicConfig(
    level=logging.INFO, format="%(asctime)s [%(levelname)s] %(message)s"
)

# Global variables to store parsed registers
R101, D101 = [], []
R141, D141 = [], []
R201 = []
R241, D241 = [], []


def get_ch_temp(payload):
    """Central heating temperature display function."""
    if len(payload) == 6:
        chtemp = int(hex(payload[1])[2:4], 16) / 2.0
        return chtemp
    return "Bad payload length"


def read_pump(ser):
    global R101, D101, R141, D141, R201, R241, D241

    logging.info(f"Sniffer active on {PORT} @ {BAUDRATE}-8-E-1...")

    while True:
        if not ser.is_open:
            logging.error("Serial connection closed.")
            break

        try:
            # Read 1 byte and check for target Slave ID (0x11 / 17)
            raw_byte = ser.read(1)
            if not raw_byte:
                continue

            rs = raw_byte.hex()

            if rs == SLAVE_ADDR_HEX:
                # Read function code / header (2 bytes)
                func_header = ser.read(2).hex()

                # --- Handle Write Commands (Function Code 0x10) ---
                if func_header == "1000":
                    header_bytes = ser.read(8).hex()
                    byte_len_hex = header_bytes[-2:]
                    byte_len = int(byte_len_hex, 16)
                    write_data = ser.read(byte_len).hex()

                    full_frame = "111000" + header_bytes + write_data
                    formatted = " ".join(
                        full_frame[i : i + 2]
                        for i in range(0, len(full_frame), 2)
                    )
                    logging.info(f"[WRITE CAPTURED] {formatted}")

                # --- Handle Read Responses (Function Code 0x03) ---

                # R101 Response (6 registers = 12 bytes = 0x0c)
                elif func_header == "030c":
                    R101, D101 = [], []
                    for _ in range(6):
                        val_hex = ser.read(2).hex()
                        if val_hex:
                            val_int = int(val_hex, 16)
                            R101.append(val_int)
                            m, l = divmod(val_int, 256)
                            D101.extend([m, l])

                    ch_temp = get_ch_temp(R101)
                    logging.info(f"[R101] Data: {D101} | CH Temp: {ch_temp}°C")

                # R141 Response (16 registers = 32 bytes = 0x20)
                elif func_header == "0320":
                    R141, D141 = [], []
                    for _ in range(16):
                        val_hex = ser.read(2).hex()
                        if val_hex:
                            val_int = int(val_hex, 16)
                            R141.append(val_int)
                            m, l = divmod(val_int, 256)
                            D141.extend([m, l])

                    logging.info(f"[R141] Data: {D141}")

                # R201 Response (1 register = 2 bytes = 0x02)
                elif func_header == "0302":
                    R201 = []
                    val_hex = ser.read(2).hex()
                    if val_hex:
                        R201.append(int(val_hex, 16))

                    logging.info(f"[R201] Data: {R201}")

                # R241 Response (22 registers = 44 bytes = 0x2c)
                elif func_header == "032c":
                    R241, D241 = [], []
                    for _ in range(22):
                        val_hex = ser.read(2).hex()
                        if val_hex:
                            val_int = int(val_hex, 16)
                            R241.append(val_int)
                            m, l = divmod(val_int, 256)
                            D241.extend([m, l])

                    logging.info(f"[R241] Data: {D241}")

        except Exception as e:
            logging.error(f"Error reading bus: {e}")
            break


def main():
    try:
        ser = serial.Serial(
            port=PORT,
            baudrate=BAUDRATE,
            parity=serial.PARITY_EVEN,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=1,
        )
    except Exception as e:
        logging.error(f"Could not open port {PORT}: {e}")
        return

    try:
        read_pump(ser)
    except KeyboardInterrupt:
        logging.info("Sniffer stopped by user.")
    finally:
        ser.close()


if __name__ == "__main__":
    main()
