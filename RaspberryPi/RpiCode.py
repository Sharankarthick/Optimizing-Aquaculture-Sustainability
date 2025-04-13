import paho.mqtt.client as mqtt
import mysql.connector

# MySQL database settings
mysql_host = "localhost"
mysql_user = "aquaUser"
mysql_password = "raspberry"
mysql_database = "aquaDB"

def store_sensor_data(data):
    try:
        conn = mysql.connector.connect(
            host=mysql_host,
            user=mysql_user,
            password=mysql_password,
            database=mysql_database
        )
        cursor = conn.cursor()

        # Extract sensor values using regular expressions
        import re
        values = re.findall(r'[-+]?\d*\.\d+|\d+', data)

        # Check if we have all expected values
        if len(values) == 4:
            # Parse and store sensor values in the MySQL database
            cursor.execute("INSERT INTO sensor_data (pH, TDS, Turbidity, Temperature) VALUES (%s, %s, %s, %s)",
                           (float(values[0]), float(values[1]), float(values[2]), float(values[3]))
                          )
            conn.commit()
            print("Data stored successfully.")
        else:
            print("Invalid data format. Expected 4 sensor values, found", len(values))

        conn.close()

    except Exception as e:
        print("Error storing data in MySQL:", e)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe("sensor_data")
    else:
        print(f"Connection failed with result code {rc}")

def on_message(client, userdata, msg):
    data = msg.payload.decode()
    print(f"\nReceived message: {data}")

    # Store data in MySQL
    store_sensor_data(data)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Set username and password for MQTT connection
client.username_pw_set(username="aquaServer", password="raspberry")

client.connect("192.168.131.181", 1883, 60)

client.loop_forever()
