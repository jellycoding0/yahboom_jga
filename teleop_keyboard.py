import serial
import serial.tools.list_ports
import time
import sys
import atexit
import math
import threading
from pynput import keyboard

# Kinematic constants for the robot
WHEEL_DIAMETER = 0.065   # 65 mm = 0.065 m
WHEEL_BASE = 0.180       # 180 mm = 0.180 m
CIRCUMFERENCE = math.pi * WHEEL_DIAMETER

# Encoder Ticks Per Revolution (PPR)
# Left motor (130 RPM, 1:34 ratio) -> 11 PPR * 34 * 4 (TI1 & TI2 mode) = 1496 ticks/rev
TICKS_PER_REV_L = 1496
# Right motor (500 RPM, 1:10 ratio) -> 11 PPR * 10 * 4 (TI1 & TI2 mode) = 440 ticks/rev
TICKS_PER_REV_R = 440

# Distance conversion factor per tick
METERS_PER_TICK_L = CIRCUMFERENCE / TICKS_PER_REV_L
METERS_PER_TICK_R = CIRCUMFERENCE / TICKS_PER_REV_R

# State variables for Odometry (X, Y in meters, Theta in radians)
x = 0.0
y = 0.0
theta = 0.0

last_ticks_l = 0
last_ticks_r = 0
last_time = time.time()
first_run = True

# Global variables to store the latest values for instant UI updates
last_v_l = 0.0
last_v_r = 0.0
last_v = 0.0
last_w = 0.0
last_gyro_z = 0.0
imu_status = "CHECKING"  # Can be "OK", "FAILED", or "CHECKING"

# Serial port configuration
def auto_detect_port():
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        return None
    for p in ports:
        desc = p.description.lower()
        if "ch340" in desc or "usb-serial" in desc or "serial" in desc:
            return p.device
    return ports[0].device

PORT = auto_detect_port()

if not PORT:
    print("\n[ERROR] No active COM ports found!")
    sys.exit(1)

try:
    ser = serial.Serial(PORT, 115200, timeout=0.1)
except Exception as e:
    print("\n[ERROR] Failed to open serial port {}: {}".format(PORT, e))
    sys.exit(1)

# Clear screen once at startup
sys.stdout.write("\033[2J\033[H")
sys.stdout.flush()

def cleanup():
    # Move cursor down to prevent overwriting the exit log
    sys.stdout.write("\033[19;1H\n[Shutdown] Stopping motors and closing connection...\n")
    try:
        if ser.is_open:
            ser.write("S\n".encode('utf-8'))
            time.sleep(0.05)
            ser.close()
            print("[Shutdown] Completed successfully.")
    except Exception as e:
        print("[Shutdown] Error during cleanup: {}".format(e))

atexit.register(cleanup)

speed = 1000
current_cmd = 'S'
pressed_keys = set()
teleop_lock = threading.Lock()

def draw_dashboard(v_l, v_r, v, w, ticks_l, ticks_r, gyro_z):
    cmd_name = {
        'S': 'STOP',
        'F': 'FORWARD',
        'B': 'BACKWARD',
        'L': 'SPIN LEFT',
        'R': 'SPIN RIGHT'
    }.get(current_cmd, 'STOP')
    
    theta_deg = math.degrees(theta)
    
    # Color-coded IMU Status for visibility
    imu_display = "OK"
    if imu_status == "FAILED":
        imu_display = "FAILED (Check Wiring/Power)"
    elif imu_status == "CHECKING":
        imu_display = "INITIALIZING"
        
    dashboard = ""
    dashboard += "\033[H"  # Move cursor to top-left
    dashboard += "======================================================================\n"
    dashboard += "             YAHBOOM 2WD ROBOT TELEOP & SENSOR FUSION DASHBOARD        \n"
    dashboard += "======================================================================\n"
    dashboard += " [Serial Port] : {:8} | [Status] : CONNECTED (IMU: {})\n".format(PORT, imu_display)
    dashboard += " [Last Cmd]   : {:8} | [Base Speed] : {:4}\n".format(cmd_name, speed)
    dashboard += "----------------------------------------------------------------------\n"
    dashboard += "                             SENSOR FEEDBACK                          \n"
    dashboard += "----------------------------------------------------------------------\n"
    dashboard += " Left Encoder Ticks  : {:10} | Right Encoder Ticks : {:10}\n".format(ticks_l, ticks_r)
    dashboard += " Left Wheel Velocity : {:8.3f} m/s | Right Wheel Velocity: {:8.3f} m/s\n".format(v_l, v_r)
    dashboard += " IMU Gyro Z (YawRate): {:8.4f} rad/s\n".format(gyro_z)
    dashboard += "----------------------------------------------------------------------\n"
    dashboard += "                        SENSOR FUSION ODOMETRY DATA                   \n"
    dashboard += "----------------------------------------------------------------------\n"
    dashboard += " Robot Position X    : {:8.3f} m   | Robot Velocity v    : {:8.3f} m/s\n".format(x, v)
    dashboard += " Robot Position Y    : {:8.3f} m   | Robot Velocity w    : {:8.3f} rad/s\n".format(y, w)
    dashboard += " Robot Heading Theta : {:8.3f} deg | \n".format(theta_deg)
    dashboard += "======================================================================\n"
    dashboard += " * Calibration: Keep the robot STILL on boot to calibrate Gyro bias.   \n"
    dashboard += " Controls: HOLD [w / a / s / d] to drive, RELEASE to stop              \n"
    dashboard += "           Press [Space] for emergency stop, [q] to quit               \n"
    dashboard += "======================================================================\n"
    
    sys.stdout.write(dashboard)
    sys.stdout.flush()

def send_command(cmd_char, speed_val=0):
    global current_cmd
    if cmd_char == current_cmd:
        return
    if cmd_char == 'S':
        cmd_str = "S\n"
    else:
        cmd_str = "{} {}\n".format(cmd_char, speed_val)
    try:
        ser.write(cmd_str.encode('utf-8'))
        current_cmd = cmd_char
        # Instantly redraw the dashboard to reflect keypress feedback
        draw_dashboard(last_v_l, last_v_r, last_v, last_w, last_ticks_l, last_ticks_r, last_gyro_z)
    except:
        pass

def on_press(key):
    global speed
    try:
        char = key.char.lower()
    except AttributeError:
        return
    if char in ['w', 's', 'a', 'd']:
        if char not in pressed_keys:
            pressed_keys.add(char)
            with teleop_lock:
                if char == 'w':   send_command('F', speed)
                elif char == 's': send_command('B', speed)
                elif char == 'a': send_command('L', speed)
                elif char == 'd': send_command('R', speed)

def on_release(key):
    try:
        char = key.char.lower()
    except AttributeError:
        if key == keyboard.Key.space:
            with teleop_lock:
                send_command('S')
                pressed_keys.clear()
        return

    if char in pressed_keys:
        pressed_keys.remove(char)

    with teleop_lock:
        if not pressed_keys:
            if current_cmd != 'S':
                send_command('S')
        else:
            remaining_key = list(pressed_keys)[-1]
            if remaining_key == 'w':   send_command('F', speed)
            elif remaining_key == 's': send_command('B', speed)
            elif remaining_key == 'a': send_command('L', speed)
            elif remaining_key == 'd': send_command('R', speed)

    if char == 'q':
        sys.exit(0)

# Kinematics and Sensor Fusion Odometry (Encoder + IMU Gyro Z)
def update_odometry(ticks_l, ticks_r, gyro_z):
    global x, y, theta, last_ticks_l, last_ticks_r, last_time, first_run
    
    current_time = time.time()
    dt = current_time - last_time
    if dt <= 0:
        dt = 0.001
        
    if first_run:
        last_ticks_l = ticks_l
        last_ticks_r = ticks_r
        last_time = current_time
        first_run = False
        return 0.0, 0.0, 0.0, 0.0

    # Calculate tick differences
    delta_l = ticks_l - last_ticks_l
    delta_r = ticks_r - last_ticks_r
    
    last_ticks_l = ticks_l
    last_ticks_r = ticks_r
    last_time = current_time
    
    # Calculate distance traveled by each wheel in meters
    d_l = delta_l * METERS_PER_TICK_L
    d_r = delta_r * METERS_PER_TICK_R
    
    # Linear velocity of each wheel (m/s)
    v_l = d_l / dt
    v_r = d_r / dt
    
    # Robot linear displacement from encoders
    d_center = (d_r + d_l) / 2.0
    
    # Orientation change directly from IMU Gyroscope Z-axis (Yaw rate in rad/s)
    d_theta = gyro_z * dt
    
    # Update robot position and orientation
    # Using midpoint integration (Runge-Kutta 2nd order)
    x += d_center * math.cos(theta + d_theta / 2.0)
    y += d_center * math.sin(theta + d_theta / 2.0)
    theta += d_theta
    
    # Keep theta normalized in range [-pi, pi]
    theta = math.atan2(math.sin(theta), math.cos(theta))
    
    # Robot linear and angular velocities
    v = d_center / dt
    w = gyro_z
    
    return v_l, v_r, v, w

# Draw the initial dashboard layout immediately at startup so the screen is not blank
draw_dashboard(0.0, 0.0, 0.0, 0.0, 0, 0, 0.0)

# Start background thread to read serial print statements from the robot
def read_serial():
    global last_v_l, last_v_r, last_v, last_w, last_ticks_l, last_ticks_r, last_gyro_z, imu_status
    while True:
        try:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    if "Encoder L:" in line and "GyroZ:" in line:
                        try:
                            # Parse line: "Encoder L: -1694 | R: 3568 | GyroZ: -0.0150 | IMU: 1"
                            parts = line.split('|')
                            l_part = parts[0].split(':')[-1].strip()
                            r_part = parts[1].split(':')[-1].strip()
                            g_part = parts[2].split(':')[-1].strip()
                            i_part = parts[3].split(':')[-1].strip()
                            
                            ticks_l = int(l_part)
                            ticks_r = int(r_part)
                            gyro_z = float(g_part) / 10000.0
                            imu_ok = int(i_part)
                            
                            # Update IMU Connection Status
                            imu_status = "OK" if imu_ok == 1 else "FAILED"
                            
                            # Update kinematics and fusion calculations
                            v_l, v_r, v, w = update_odometry(ticks_l, ticks_r, gyro_z)
                            
                            # Cache values for instant UI updates
                            last_v_l, last_v_r, last_v, last_w = v_l, v_r, v, w
                            last_ticks_l, last_ticks_r, last_gyro_z = ticks_l, ticks_r, gyro_z
                            
                            # Redraw CLI Dashboard
                            draw_dashboard(v_l, v_r, v, w, ticks_l, ticks_r, gyro_z)
                        except ValueError:
                            # Skip corrupted/incomplete lines
                            continue
                    elif "Initializing" in line or "success" in line or "failed" in line or "WHO_AM_I" in line:
                        # Move cursor to logging line (below dashboard) and print startup debug messages
                        sys.stdout.write("\033[20;1H[System Log]: {}\n\033[H".format(line))
                        sys.stdout.flush()
        except (serial.SerialException, OSError):
            break
        time.sleep(0.01)

read_thread = threading.Thread(target=read_serial, daemon=True)
read_thread.start()

# Start pynput keyboard listener
listener = keyboard.Listener(on_press=on_press, on_release=on_release)
listener.start()

# Main thread loop: sleeps and remains interruptible by Ctrl+C
try:
    while listener.running:
        time.sleep(0.1)
except KeyboardInterrupt:
    pass
finally:
    listener.stop()
