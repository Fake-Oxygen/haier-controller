import sqlite3
import paho.mqtt.client as mqtt
import cbor2
from datetime import datetime
import psycopg2
from datetime import datetime

BROKER_IP = "192.168.10.184"
TOPIC = "heatpump/status"

def insert_to_db(data):
    conn = psycopg2.connect(host="127.0.0.1", port=5432, dbname="heatpump", user="morow", password="1296")
    cur = conn.cursor()
    cur.execute("""
                INSERT INTO status (timestamp, ambient_temp, ch_temp, dhw_temp, ch_target_temp, dhw_target_temp, opr_mode, valve_state, tank_state, heater_state)
                VALUES (NOW(), %s, %s, %s, %s, %s, %s, %s, %s, %s);
                """, (data['ambient_temp'], data['ch_temp'], data['dhw_temp'], data['ch_target_temp'], data['dhw_target_temp'], data['opr_mode'], data['valve_state'], data['tank_state'], data['heater_state']))
    conn.commit()
    cur.close()
    conn.close()

def on_message(client, userdata, msg):
    try:
        payload = cbor2.loads(msg.payload)
        
        data = {
            "ambient_temp": round(payload[0], 2),
            "ch_temp": round(payload[1], 1),
            "dhw_temp": round(payload[2], 1),
            "ch_target_temp": round(payload[3], 1),
            "dhw_target_temp": round(payload[4], 1),
            "opr_mode": payload[5],
            "valve_state": payload[6],
            "tank_state": payload[7],
            "heater_state": payload[8],
        }
        insert_to_db(data)
    except Exception as e:
        print(f"Failed to decode or insert payload: {e}")

def main():
    client = mqtt.Client(client_id="db_logger_service")
    client.on_message = on_message

    client.connect(BROKER_IP, 1883, 60)
    client.subscribe(TOPIC)
    print(f"Logging telemetry from broker '{BROKER_IP}' on topic '{TOPIC}'...")

    client.loop_forever()

if __name__ == "__main__":
    main()
