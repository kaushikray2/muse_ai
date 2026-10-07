#!/usr/bin/env python3
"""
Battery Calibration Tester -- PC GUI (tkinter)
==============================================
Works with battery_calibrator.ino on the Arduino.

What it does:
  1. Configures the test from the GUI: cutoff voltage (PMOS cutoff =
     "battery dead"), sample interval, load resistance, max test time.
  2. Starts the discharge run (cell must be fully charged first).
     The Arduino switches the 12 ohm load on, samples cell voltage,
     integrates current (I = V / R) into mAh, and streams DATA lines.
  3. When cell voltage hits the cutoff, the Arduino opens the MOSFET
     and reports total measured capacity.
  4. The GUI builds the calibration table: voltage -> remaining capacity %
     (interpolated to N evenly spaced voltage points), shows it live in a
     table, and can save raw samples + calibration table as CSV.

Requires:  pip install pyserial
Run:       python3 battery_gui.py
"""

import csv
import queue
import threading
import time
import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    raise SystemExit("pyserial is required:  pip install pyserial")

BAUD_DEFAULT = 115200


# --------------------------------------------------------------------------
# Serial worker thread
# --------------------------------------------------------------------------
class SerialWorker(threading.Thread):
    """Reads lines from the Arduino and pushes parsed events into a queue."""

    def __init__(self, ser, outbox):
        super().__init__(daemon=True)
        self.ser = ser
        self.outbox = outbox
        self.running = True

    def run(self):
        while self.running:
            try:
                raw = self.ser.readline()
            except Exception as exc:                      # port died
                self.outbox.put(("ERROR", str(exc)))
                return
            if not raw:
                continue
            line = raw.decode("utf-8", errors="replace").strip()
            if not line:
                continue
            self.outbox.put(("LINE", line))

    def stop(self):
        self.running = False


# --------------------------------------------------------------------------
# Calibration math
# --------------------------------------------------------------------------
def build_calibration_table(samples, n_points):
    """samples: list of (elapsed_s, volts, discharged_mah).
    Returns list of (volts, remaining_percent), N points evenly spaced in
    voltage from the highest sampled voltage down to the lowest."""
    if len(samples) < 2 or n_points < 2:
        return []
    total = samples[-1][2]
    if total <= 0:
        return []
    # (voltage, remaining %) pairs, voltage descending
    pts = sorted(((v, (total - mah) / total * 100.0) for _, v, mah in samples),
                 key=lambda p: -p[0])
    v_hi, v_lo = pts[0][0], pts[-1][0]
    if v_hi <= v_lo:
        return [(v_hi, pts[0][1])]

    def interp(v):
        if v >= pts[0][0]:
            return pts[0][1]
        if v <= pts[-1][0]:
            return pts[-1][1]
        for (v1, p1), (v2, p2) in zip(pts, pts[1:]):
            if v2 <= v <= v1:
                f = (v - v2) / (v1 - v2) if v1 != v2 else 0.0
                return p2 + f * (p1 - p2)
        return pts[-1][1]

    return [(v_hi - (v_hi - v_lo) * i / (n_points - 1),
             interp(v_hi - (v_hi - v_lo) * i / (n_points - 1)))
            for i in range(n_points)]


# --------------------------------------------------------------------------
# GUI
# --------------------------------------------------------------------------
class BatteryCalApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Battery Calibration Tester")
        self.geometry("980x760")

        self.ser = None
        self.worker = None
        self.inbox = queue.Queue()
        self.testing = False

        self.samples = []          # (elapsed_s, volts, discharged_mah)
        self.test_start_wall = 0.0
        self.load_ohms = 12.0
        self.cutoff_v = 3.0

        self._build_widgets()
        self.after(120, self._poll_inbox)
        self.protocol("WM_DELETE_WINDOW", self._on_close)

    # -- widget layout ------------------------------------------------------
    def _build_widgets(self):
        # Connection row
        conn = ttk.LabelFrame(self, text="Connection")
        conn.pack(fill="x", padx=8, pady=4)
        ttk.Label(conn, text="Port:").pack(side="left", padx=4)
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(conn, textvariable=self.port_var,
                                       width=28, state="readonly")
        self.port_combo.pack(side="left", padx=4)
        ttk.Button(conn, text="Refresh", command=self._refresh_ports).pack(side="left", padx=4)
        ttk.Label(conn, text="Baud:").pack(side="left", padx=4)
        self.baud_var = tk.StringVar(value=str(BAUD_DEFAULT))
        ttk.Combobox(conn, textvariable=self.baud_var, width=8,
                     values=["9600", "57600", "115200"]).pack(side="left", padx=4)
        self.connect_btn = ttk.Button(conn, text="Connect", command=self._toggle_connect)
        self.connect_btn.pack(side="left", padx=8)
        self.conn_status = ttk.Label(conn, text="disconnected", foreground="red")
        self.conn_status.pack(side="left", padx=4)
        self._refresh_ports()

        # Test configuration
        cfg = ttk.LabelFrame(self, text="Test configuration")
        cfg.pack(fill="x", padx=8, pady=4)
        self.cfg_vars = {}
        fields = [
            ("cutoff_v",   "Cutoff voltage (V) -- PMOS opens, cell = dead", "3.0"),
            ("interval_s", "Sample interval (s)",                            "1.0"),
            ("load_ohms",  "Load resistance (ohm)",                          "12.0"),
            ("max_hours",  "Max test time (hours, failsafe)",                "12"),
            ("table_pts",  "Calibration table points",                       "25"),
            ("nominal",    "Nominal capacity (mAh, optional ref)",            ""),
        ]
        for i, (key, label, default) in enumerate(fields):
            ttk.Label(cfg, text=label).grid(row=i // 2, column=(i % 2) * 2,
                                            sticky="w", padx=6, pady=2)
            var = tk.StringVar(value=default)
            ttk.Entry(cfg, textvariable=var, width=12).grid(
                row=i // 2, column=(i % 2) * 2 + 1, sticky="w", padx=6, pady=2)
            self.cfg_vars[key] = var

        # Controls
        ctl = ttk.Frame(self)
        ctl.pack(fill="x", padx=8, pady=4)
        self.start_btn = ttk.Button(ctl, text="Start Test", command=self._start_test,
                                    state="disabled")
        self.start_btn.pack(side="left", padx=4)
        self.stop_btn = ttk.Button(ctl, text="Stop Test", command=self._stop_test,
                                   state="disabled")
        self.stop_btn.pack(side="left", padx=4)
        ttk.Button(ctl, text="Save raw CSV", command=self._save_raw).pack(side="left", padx=4)
        ttk.Button(ctl, text="Save table CSV", command=self._save_table).pack(side="left", padx=4)
        ttk.Button(ctl, text="Clear", command=self._clear_all).pack(side="left", padx=4)

        # Live readouts
        live = ttk.LabelFrame(self, text="Live")
        live.pack(fill="x", padx=8, pady=4)
        self.live_vars = {}
        for i, name in enumerate(["Voltage (V)", "Current (mA)", "Elapsed",
                                  "Discharged (mAh)", "Status"]):
            ttk.Label(live, text=name + ":").grid(row=0, column=i * 2, padx=6, pady=4, sticky="e")
            var = tk.StringVar(value="--")
            ttk.Label(live, textvariable=var, width=14,
                      font=("TkDefaultFont", 10, "bold")).grid(row=0, column=i * 2 + 1,
                                                              padx=2, pady=4, sticky="w")
            self.live_vars[name] = var
        self.live_vars["Status"].set("idle")

        # Plot + log side by side
        mid = ttk.Frame(self)
        mid.pack(fill="both", expand=True, padx=8, pady=4)
        plotf = ttk.LabelFrame(mid, text="Voltage vs time")
        plotf.pack(side="left", fill="both", expand=True, padx=(0, 4))
        self.canvas = tk.Canvas(plotf, bg="white", height=220)
        self.canvas.pack(fill="both", expand=True, padx=4, pady=4)
        self.canvas.bind("<Configure>", lambda _e: self._draw_plot())

        logf = ttk.LabelFrame(mid, text="Log")
        logf.pack(side="left", fill="both", expand=True, padx=(4, 0))
        self.log = scrolledtext.ScrolledText(logf, height=12, width=40, state="disabled")
        self.log.pack(fill="both", expand=True, padx=4, pady=4)

        # Calibration table
        tblf = ttk.LabelFrame(self, text="Calibration table: voltage -> remaining capacity")
        tblf.pack(fill="both", expand=True, padx=8, pady=4)
        cols = ("voltage", "percent")
        self.table = ttk.Treeview(tblf, columns=cols, show="headings", height=8)
        self.table.heading("voltage", text="Voltage (V)")
        self.table.heading("percent", text="Remaining (%)")
        self.table.column("voltage", width=120, anchor="center")
        self.table.column("percent", width=120, anchor="center")
        self.table.pack(side="left", fill="both", expand=True, padx=4, pady=4)
        sb = ttk.Scrollbar(tblf, orient="vertical", command=self.table.yview)
        sb.pack(side="right", fill="y")
        self.table.configure(yscrollcommand=sb.set)

        self.cal_table = []

    # -- helpers ------------------------------------------------------------
    def _log(self, msg):
        ts = time.strftime("%H:%M:%S")
        self.log.configure(state="normal")
        self.log.insert("end", f"[{ts}] {msg}\n")
        self.log.see("end")
        self.log.configure(state="disabled")

    def _refresh_ports(self):
        ports = [p.device for p in list_ports.comports()]
        self.port_combo["values"] = ports
        if ports and not self.port_var.get():
            self.port_var.set(ports[0])

    def _send(self, text):
        if self.ser and self.ser.is_open:
            self.ser.write((text + "\n").encode())
            self._log(f">> {text}")

    # -- connection ---------------------------------------------------------
    def _toggle_connect(self):
        if self.ser and self.ser.is_open:
            self._disconnect()
        else:
            port = self.port_var.get()
            if not port:
                messagebox.showwarning("No port", "Select a serial port first.")
                return
            try:
                self.ser = serial.Serial(port, int(self.baud_var.get()), timeout=1)
            except Exception as exc:
                messagebox.showerror("Connect failed", str(exc))
                return
            time.sleep(1.5)  # let Arduino reset; catch its READY
            self.ser.reset_input_buffer()
            self.worker = SerialWorker(self.ser, self.inbox)
            self.worker.start()
            self._send("PING")
            self.connect_btn.config(text="Disconnect")
            self.conn_status.config(text=f"connected ({port})", foreground="green")
            self.start_btn.config(state="normal")
            self._log(f"Connected to {port}, sent PING (expect PONG + READY).")

    def _disconnect(self):
        self._send("STOP")
        if self.worker:
            self.worker.stop()
            self.worker = None
        if self.ser:
            try:
                self.ser.close()
            except Exception:
                pass
            self.ser = None
        self.testing = False
        self.connect_btn.config(text="Connect")
        self.conn_status.config(text="disconnected", foreground="red")
        self.start_btn.config(state="disabled")
        self.stop_btn.config(state="disabled")
        self.live_vars["Status"].set("idle")
        self._log("Disconnected.")

    # -- test control -------------------------------------------------------
    def _read_config(self):
        try:
            cutoff   = float(self.cfg_vars["cutoff_v"].get())
            interval = float(self.cfg_vars["interval_s"].get())
            load     = float(self.cfg_vars["load_ohms"].get())
            max_h    = float(self.cfg_vars["max_hours"].get())
            n_pts    = int(self.cfg_vars["table_pts"].get())
        except ValueError:
            messagebox.showerror("Bad config", "Cutoff/interval/load/hours/points must be numbers.")
            return None
        if not (0.5 <= cutoff <= 5.0 and interval >= 0.2 and load > 0
                and max_h > 0 and n_pts >= 2):
            messagebox.showerror("Bad config", "Check ranges: cutoff 0.5-5 V, "
                                 "interval >= 0.2 s, load > 0, points >= 2.")
            return None
        return dict(cutoff=cutoff, interval=interval, load=load,
                    max_h=max_h, n_pts=n_pts)

    def _start_test(self):
        cfg = self._read_config()
        if not cfg or not (self.ser and self.ser.is_open):
            return
        if not messagebox.askyesno(
                "Start discharge test",
                f"Cell must be FULLY CHARGED.\n\n"
                f"Discharge through {cfg['load']:.1f} ohm until "
                f"{cfg['cutoff']:.2f} V, sampling every {cfg['interval']:.1f} s.\n"
                f"Start now?"):
            return
        self.cutoff_v = cfg["cutoff"]
        self.load_ohms = cfg["load"]
        self.n_pts = cfg["n_pts"]
        self.samples = []
        self.cal_table = []
        for row in self.table.get_children():
            self.table.delete(row)
        self.test_start_wall = time.time()
        self.testing = True
        cutoff_mv = int(round(cfg["cutoff"] * 1000))
        interval_ms = int(round(cfg["interval"] * 1000))
        max_min = int(round(cfg["max_h"] * 60))
        self._send(f"START {cutoff_mv} {interval_ms} {max_min}")
        self.start_btn.config(state="disabled")
        self.stop_btn.config(state="normal")
        self.live_vars["Status"].set("running")

    def _stop_test(self):
        self._send("STOP")

    def _finish_test(self, total_mah, reason):
        self.testing = False
        self.start_btn.config(state="normal")
        self.stop_btn.config(state="disabled")
        self.live_vars["Status"].set(f"done ({reason})")
        self.live_vars["Discharged (mAh)"].set(f"{total_mah:.1f}")
        nom_txt = self.cfg_vars["nominal"].get().strip()
        extra = ""
        if nom_txt:
            try:
                extra = f"  (nominal {float(nom_txt):.0f} mAh -> {total_mah / float(nom_txt) * 100:.1f}% of nominal)"
            except ValueError:
                pass
        self._log(f"Test finished ({reason}). Measured capacity: {total_mah:.1f} mAh.{extra}")
        self.cal_table = build_calibration_table(self.samples, self.n_pts)
        for row in self.table.get_children():
            self.table.delete(row)
        for v, p in self.cal_table:
            self.table.insert("", "end", values=(f"{v:.3f}", f"{p:.1f}"))
        self._log(f"Calibration table built: {len(self.cal_table)} points "
                  f"(voltage -> remaining %).")
        self._draw_plot()

    # -- incoming serial data -----------------------------------------------
    def _poll_inbox(self):
        try:
            while True:
                kind, payload = self.inbox.get_nowait()
                if kind == "LINE":
                    self._handle_line(payload)
                elif kind == "ERROR":
                    self._log(f"Serial error: {payload}")
                    self._disconnect()
        except queue.Empty:
            pass
        self.after(120, self._poll_inbox)

    def _handle_line(self, line):
        self._log(f"<< {line}")
        parts = line.split()
        if not parts:
            return
        tag = parts[0]
        if tag == "DATA" and len(parts) == 4 and self.testing:
            try:
                mv, elapsed_s, mah = int(parts[1]), int(parts[2]), float(parts[3])
            except ValueError:
                return
            v = mv / 1000.0
            self.samples.append((elapsed_s, v, mah))
            self.live_vars["Voltage (V)"].set(f"{v:.3f}")
            self.live_vars["Current (mA)"].set(f"{v / self.load_ohms * 1000:.1f}")
            h, rem = divmod(elapsed_s, 3600)
            m, s = divmod(rem, 60)
            self.live_vars["Elapsed"].set(f"{h:d}:{m:02d}:{s:02d}")
            self.live_vars["Discharged (mAh)"].set(f"{mah:.1f}")
            if len(self.samples) % 5 == 0:
                self._draw_plot()
        elif tag == "DONE" and len(parts) >= 3:
            try:
                total = float(parts[1])
            except ValueError:
                total = self.samples[-1][2] if self.samples else 0.0
            self._finish_test(total, parts[2].lower())
        elif tag == "STOPPED":
            total = self.samples[-1][2] if self.samples else 0.0
            self._log("Stopped by user; building table from partial data.")
            self._finish_test(total, "stopped")
        elif tag in ("PONG", "READY", "ACK"):
            pass  # already logged
        elif tag == "ERR":
            self._log(f"Arduino error: {line}")
            messagebox.showwarning("Arduino", line)

    # -- plot ---------------------------------------------------------------
    def _draw_plot(self):
        c = self.canvas
        c.delete("all")
        w, h = c.winfo_width(), c.winfo_height()
        if w < 40 or h < 40 or len(self.samples) < 2:
            return
        m = 34
        ts = [s[0] for s in self.samples]
        vs = [s[1] for s in self.samples]
        t0, t1 = ts[0], max(ts[-1], ts[0] + 1)
        v0, v1 = min(vs), max(vs)
        if v1 - v0 < 0.05:
            v1 = v0 + 0.05
        # decimate for drawing
        step = max(1, len(ts) // 600)
        pts = []
        for t, v in zip(ts[::step], vs[::step]):
            x = m + (t - t0) / (t1 - t0) * (w - 2 * m)
            y = h - m - (v - v0) / (v1 - v0) * (h - 2 * m)
            pts += [x, y]
        c.create_line(*pts, fill="#1a73e8", width=2)
        # cutoff line
        yc = h - m - (self.cutoff_v - v0) / (v1 - v0) * (h - 2 * m)
        if m <= yc <= h - m:
            c.create_line(m, yc, w - m, yc, fill="red", dash=(4, 3))
            c.create_text(w - m - 4, yc - 8, anchor="e",
                          text=f"cutoff {self.cutoff_v:.2f} V", fill="red", font=("TkDefaultFont", 8))
        c.create_text(m, 12, anchor="w", text=f"{v1:.2f} V", font=("TkDefaultFont", 8))
        c.create_text(m, h - 12, anchor="w", text=f"{v0:.2f} V", font=("TkDefaultFont", 8))

    # -- file output ----------------------------------------------------------
    def _save_raw(self):
        if not self.samples:
            messagebox.showinfo("Nothing to save", "No samples recorded yet.")
            return
        path = filedialog.asksaveasfilename(defaultextension=".csv",
                                            filetypes=[("CSV", "*.csv")],
                                            initialfile="discharge_raw.csv")
        if not path:
            return
        with open(path, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["elapsed_s", "voltage_V", "current_mA", "discharged_mAh"])
            for t, v, mah in self.samples:
                w.writerow([t, f"{v:.4f}", f"{v / self.load_ohms * 1000:.2f}", f"{mah:.2f}"])
        self._log(f"Raw data saved: {path} ({len(self.samples)} samples).")

    def _save_table(self):
        if not self.cal_table:
            messagebox.showinfo("Nothing to save", "No calibration table yet -- run a test first.")
            return
        path = filedialog.asksaveasfilename(defaultextension=".csv",
                                            filetypes=[("CSV", "*.csv")],
                                            initialfile="calibration_table.csv")
        if not path:
            return
        with open(path, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["voltage_V", "remaining_percent"])
            for v, p in self.cal_table:
                w.writerow([f"{v:.3f}", f"{p:.1f}"])
        self._log(f"Calibration table saved: {path}.")

    def _clear_all(self):
        self.samples = []
        self.cal_table = []
        for row in self.table.get_children():
            self.table.delete(row)
        for var in self.live_vars.values():
            var.set("--")
        self.live_vars["Status"].set("idle")
        self._draw_plot()
        self._log("Cleared samples and table.")

    def _on_close(self):
        try:
            if self.worker:
                self.worker.stop()
            if self.ser and self.ser.is_open:
                try:
                    self.ser.write(b"STOP\n")
                except Exception:
                    pass
                self.ser.close()
        finally:
            self.destroy()


if __name__ == "__main__":
    BatteryCalApp().mainloop()
