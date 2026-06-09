import sys
import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import serial
import serial.tools.list_ports
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# =========================
# GLOBAL VARIABLES & CONTROLS
# =========================
ANGLE_OFFSET = 0  # Global variable for your angle offset

# =========================
# UART SETUP
# =========================

ports = list(serial.tools.list_ports.grep("CH340"))

if not ports:
    print("Error: No CH340 device found.")
    sys.exit(1)

SERIAL_PORT = ports[0].device
BAUDRATE = 115200
try:
    ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=0.001)
except Exception as e:
    print(f"Error opening serial port: {e}")
    sys.exit(1)

# =========================
# PARAMETRI
# =========================
MAX_ANGLE = 360
MAX_DISTANCE = 1000

# =========================
# POLARNI PODACI
# =========================
scan = np.zeros((MAX_ANGLE, 3))
distances = np.zeros(MAX_ANGLE)

# Pre-allocate theta matrix and stack base for ultra-fast scatter updates
theta = np.radians(np.arange(MAX_ANGLE))
offsets_base = np.column_stack((theta, distances))

# Pre-calculate line sweeps for the static grid to avoid loops
grid_angles = np.radians(np.arange(0, 360, 45))

# =========================
# FIGURA CREATION
# =========================
fig = plt.figure(figsize=(7, 6))
ax = plt.subplot(111, polar=True)

fig.patch.set_facecolor("#0b0f14")
ax.set_facecolor("#0b0f14")

ax.set_theta_zero_location('N')
ax.set_theta_direction(-1)
ax.set_ylim(0, MAX_DISTANCE + 10)
ax.set_xticks([])
ax.set_yticks([])

# =========================
# RADAR GRID (Optimized)
# =========================
grid_color = "#1f9fbf"
distance_levels = [250, 500, 750, 1000]
label_angle = np.radians(89)
circle_linspace = np.linspace(0, 2*np.pi, 360)

for r in distance_levels:
    ax.plot(
        circle_linspace,
        [r]*360,
        color=grid_color,
        linewidth=0.6,
        alpha=0.6
    )
    ax.text(
        label_angle,
        r - 67,
        f"{r}",
        color="#ffffff",
        fontsize=9,        
        ha="right",        
        va="center",
        alpha=0.85
    )

for a, rad in zip(range(0, 360, 45), grid_angles):
    ax.plot(
        [rad, rad],
        [0, MAX_DISTANCE],
        color=grid_color,
        linewidth=0.5,
        alpha=0.4
    )
    ax.text(
        rad,
        MAX_DISTANCE + 120,
        f"{a}°",
        color=grid_color,
        fontsize=9,
        ha="center",
        va="center",
        alpha=0.9
    )

sweep_line, = ax.plot([0, 0], [0, 0], color="#7df9ff", linewidth=2)

# --- CHANGED LINE HERE: Using ax.text with transform ensures blitting works ---
info_text = ax.text(
    0.5, -0.15, "",
    transform=ax.transAxes,
    ha="center",
    color="#7df9ff",
    fontsize=12,
    family="monospace"
)

def get_color(distance):
    ratio = np.clip(distance / MAX_DISTANCE, 0, 1)
    return (1.0 - ratio, 0.3, ratio)

def read_uart_packet():
    try:
        if ser.in_waiting > 0:
            # Vectorized reading of bytes to string
            raw_data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            
            # Find last valid entry directly without splitting the entire buffer
            rfind_idx = raw_data.rfind('\r\n')
            if rfind_idx != -1:
                start_idx = raw_data.rfind('\r\n', 0, rfind_idx)
                last_line = raw_data[start_idx+2:rfind_idx] if start_idx != -1 else raw_data[:rfind_idx]
                
                if "," in last_line:
                    parts = last_line.split(",")
                    if len(parts) == 2:
                        return int(float(parts[0])), min(int(float(parts[1])), MAX_DISTANCE)
    except Exception:
        pass 
    return None, None

scat = ax.scatter(theta, distances, c=scan, s=12)
last_idx = 0

# =========================
# ANIMATION UPDATE (MODIFIED FOR PERFORMANCE)
# =========================
def update(frame):
    global last_idx, ANGLE_OFFSET
    raw_angle, distance = read_uart_packet()
    
    if raw_angle is not None:
        # ---- APPLICATION OF THE OFFSET ----
        angle = (raw_angle + ANGLE_OFFSET) % MAX_ANGLE
        idx = int(angle) % MAX_ANGLE
        
        # POPUNJAVANJE RUPA:
        if abs(idx - last_idx) < 50: 
            step_dir = 1 if idx > last_idx else -1
            # Optimized continuous slice update instead of looping single indexes
            rng = range(last_idx, idx, step_dir)
            indices = [i % MAX_ANGLE for i in rng]
            distances[indices] = distance
            scan[indices, :] = get_color(distance)
        
        distances[idx] = distance
        scan[idx, :] = get_color(distance)
        last_idx = idx

        # Fast conditional replacement without flattening arrays
        render_distances = np.where(distances >= MAX_DISTANCE, np.nan, distances)

        # In-place assignments instead of allocating brand new arrays inside the loop
        offsets_base[:, 1] = render_distances
        scat.set_offsets(offsets_base)
        scat.set_color(scan)
        
        rad_angle = np.radians(angle)
        sweep_line.set_xdata([rad_angle, rad_angle])
        sweep_line.set_ydata([0, MAX_DISTANCE])
        sweep_line.set_color("#ffffff")
        
        info_text.set_text(f"ANGLE: {idx:3d}°      DISTANCE: {distance:4d} mm")

    return scat, sweep_line, info_text

# =========================
# TKINTER GUI SETUP
# =========================
root = tk.Tk()
root.title("LIDAR Radar Controller")
root.configure(bg="#0b0f14")

# Matplotlib integration frame
plot_frame = tk.Frame(root, bg="#0b0f14")
plot_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

canvas = FigureCanvasTkAgg(fig, master=plot_frame)
canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)

# Control panel frame
control_frame = tk.Frame(root, bg="#10151d", padx=10, pady=10)
control_frame.pack(side=tk.BOTTOM, fill=tk.X)

# Label displaying current offset value
offset_label = tk.Label(
    control_frame, 
    text=f"Current Offset: {ANGLE_OFFSET}°", 
    fg="#7df9ff", 
    bg="#10151d",
    font=("monospace", 11)
)
offset_label.pack(side=tk.LEFT, padx=10)

def update_offset_ui():
    """Helper to sync label display with global variable"""
    offset_label.config(text=f"Current Offset: {ANGLE_OFFSET}°")

# Button actions
def decrease_offset():
    global ANGLE_OFFSET
    ANGLE_OFFSET = (ANGLE_OFFSET - 5) % MAX_ANGLE
    update_offset_ui()

def increase_offset():
    global ANGLE_OFFSET
    ANGLE_OFFSET = (ANGLE_OFFSET + 5) % MAX_ANGLE
    update_offset_ui()

# UI Interaction Elements
btn_dec = tk.Button(control_frame, text="-5°", command=decrease_offset, bg="#1f9fbf", fg="white", width=5)
btn_dec.pack(side=tk.LEFT, padx=5)

btn_inc = tk.Button(control_frame, text="+5°", command=increase_offset, bg="#1f9fbf", fg="white", width=5)
btn_inc.pack(side=tk.LEFT, padx=5)

# =========================
# START ANIMATION & TKINTER
# =========================
ani = animation.FuncAnimation(
    fig, update, interval=10, cache_frame_data=False, blit=True
)

root.mainloop()
