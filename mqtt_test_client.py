#!/usr/bin/env python3
"""
ESP32-CAM MQTT Test Client

This script allows you to test MQTT commands with the ESP32-CAM device.
Make sure to install the required dependencies:
    pip install paho-mqtt

Usage:
    python mqtt_test_client.py
"""

import json
import time
import paho.mqtt.client as mqtt
from datetime import datetime

# Configuration - Update these values
MQTT_BROKER = "localhost"  # Change to your MQTT broker
MQTT_PORT = 1883
MQTT_USERNAME = None  # Set if required
MQTT_PASSWORD = None  # Set if required

# Topics
COMMAND_TOPIC = "esp32cam/command"
STATUS_TOPIC = "esp32cam/status"
FACES_TOPIC = "esp32cam/faces"

class ESP32CAMController:
    def __init__(self):
        self.client = mqtt.Client()
        self.connected = False
        
        # Set up callbacks
        self.client.on_connect = self.on_connect
        self.client.on_disconnect = self.on_disconnect
        self.client.on_message = self.on_message
        
        # Set credentials if provided
        if MQTT_USERNAME and MQTT_PASSWORD:
            self.client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    
    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print(f"[{self.timestamp()}] Connected to MQTT broker")
            self.connected = True
            
            # Subscribe to status topics
            client.subscribe(STATUS_TOPIC)
            client.subscribe(FACES_TOPIC)
            print(f"[{self.timestamp()}] Subscribed to {STATUS_TOPIC} and {FACES_TOPIC}")
        else:
            print(f"[{self.timestamp()}] Failed to connect to MQTT broker: {rc}")
    
    def on_disconnect(self, client, userdata, rc):
        print(f"[{self.timestamp()}] Disconnected from MQTT broker")
        self.connected = False
    
    def on_message(self, client, userdata, msg):
        try:
            topic = msg.topic
            payload = msg.payload.decode('utf-8')
            
            print(f"\n[{self.timestamp()}] Message from {topic}:")
            
            # Try to parse as JSON for better formatting
            try:
                data = json.loads(payload)
                print(json.dumps(data, indent=2))
            except json.JSONDecodeError:
                print(payload)
                
        except Exception as e:
            print(f"[{self.timestamp()}] Error processing message: {e}")
    
    def timestamp(self):
        return datetime.now().strftime("%H:%M:%S")
    
    def connect(self):
        try:
            print(f"[{self.timestamp()}] Connecting to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}")
            self.client.connect(MQTT_BROKER, MQTT_PORT, 60)
            self.client.loop_start()
            
            # Wait for connection
            timeout = 10
            while not self.connected and timeout > 0:
                time.sleep(0.5)
                timeout -= 0.5
            
            if not self.connected:
                print(f"[{self.timestamp()}] Connection timeout")
                return False
            
            return True
        except Exception as e:
            print(f"[{self.timestamp()}] Connection error: {e}")
            return False
    
    def disconnect(self):
        self.client.loop_stop()
        self.client.disconnect()
    
    def send_command(self, command, params=None):
        if not self.connected:
            print(f"[{self.timestamp()}] Not connected to MQTT broker")
            return False
        
        message = {"command": command}
        if params:
            message["params"] = params
        
        payload = json.dumps(message)
        
        try:
            result = self.client.publish(COMMAND_TOPIC, payload)
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                print(f"[{self.timestamp()}] Sent command: {command}")
                return True
            else:
                print(f"[{self.timestamp()}] Failed to send command: {result.rc}")
                return False
        except Exception as e:
            print(f"[{self.timestamp()}] Error sending command: {e}")
            return False

def show_menu():
    print("\n" + "="*50)
    print("ESP32-CAM MQTT Test Client")
    print("="*50)
    print("1.  Start streaming")
    print("2.  Stop streaming")
    print("3.  Enable face detection")
    print("4.  Disable face detection")
    print("5.  Capture image")
    print("6.  Get status")
    print("7.  Get face data")
    print("8.  Set camera quality (low)")
    print("9.  Set camera quality (high)")
    print("10. Reboot ESP32")
    print("0.  Quit")
    print("-"*50)

def main():
    controller = ESP32CAMController()
    
    if not controller.connect():
        print("Failed to connect to MQTT broker. Exiting.")
        return
    
    try:
        while True:
            show_menu()
            choice = input("Enter your choice (0-10): ").strip()
            
            if choice == "0":
                break
            elif choice == "1":
                controller.send_command("start_stream")
            elif choice == "2":
                controller.send_command("stop_stream")
            elif choice == "3":
                controller.send_command("enable_face_detection")
            elif choice == "4":
                controller.send_command("disable_face_detection")
            elif choice == "5":
                controller.send_command("capture_image")
            elif choice == "6":
                controller.send_command("get_status")
            elif choice == "7":
                controller.send_command("get_face_data")
            elif choice == "8":
                controller.send_command("set_camera_quality", {"quality": 20})
            elif choice == "9":
                controller.send_command("set_camera_quality", {"quality": 10})
            elif choice == "10":
                confirm = input("Are you sure you want to reboot the ESP32? (y/N): ")
                if confirm.lower() == 'y':
                    controller.send_command("reboot")
            else:
                print("Invalid choice. Please try again.")
            
            if choice != "0":
                time.sleep(1)  # Brief pause to see responses
    
    except KeyboardInterrupt:
        print(f"\n[{controller.timestamp()}] Interrupted by user")
    
    finally:
        controller.disconnect()
        print(f"[{controller.timestamp()}] Disconnected from MQTT broker")

if __name__ == "__main__":
    main()
