#!/usr/bin/env python3
# serial_autolistener.py
import argparse
import os
import sys
import time
import signal

try:
    import serial
    from serial.serialutil import SerialException
except ImportError:
    sys.stderr.write("Falta pyserial. Instálalo con: pip install pyserial\n")
    sys.exit(1)

RUNNING = True

def handle_sigint(signum, frame):
    global RUNNING
    RUNNING = False

signal.signal(signal.SIGINT, handle_sigint)
signal.signal(signal.SIGTERM, handle_sigint)

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def port_exists(path: str) -> bool:
    return os.path.exists(path)

def try_open(port, baud, timeout_s=0.5):
    """Intenta abrir el puerto con 8N1 y devuelve la instancia o None."""
    try:
        ser = serial.Serial(
            port=port,
            baudrate=baud,
            bytesize=serial.EIGHTBITS,   # 8
            parity=serial.PARITY_NONE,   # N
            stopbits=serial.STOPBITS_ONE,# 1
            timeout=timeout_s,
            write_timeout=timeout_s,
            exclusive=True  # evita compartir el fd si el SO lo soporta
        )
        return ser
    except SerialException as ex:
        return None
    except OSError:
        return None

def probe_for_data(ser, seconds=3.0):
    """Lee durante 'seconds' y devuelve True si se recibió algo."""
    deadline = time.time() + seconds
    total = 0
    while RUNNING and time.time() < deadline:
        try:
            chunk = ser.read(4096)
            total += len(chunk)
            # si hay líneas, imprímelas ya (evita perder datos del probe)
            if chunk:
                try:
                    sys.stdout.write(chunk.decode("utf-8", errors="replace"))
                    sys.stdout.flush()
                except Exception:
                    # como fallback, imprime bytes crudos
                    sys.stdout.buffer.write(chunk)
                    sys.stdout.flush()
        except (SerialException, OSError):
            return False
    return total > 0

def read_forever(ser, port, baud):
    eprint(f"✅ Conectado a {port} @ {baud} 8N1. Leyendo… (Ctrl+C para salir)")
    while RUNNING:
        try:
            data = ser.read(4096)
            if data:
                # imprime tal cual (UTF-8 si se puede; si no, reemplaza)
                try:
                    sys.stdout.write(data.decode("utf-8", errors="replace"))
                    sys.stdout.flush()
                except Exception:
                    sys.stdout.buffer.write(data)
                    sys.stdout.flush()
            else:
                # sin datos en este ciclo; duerme un poco para no quemar CPU
                time.sleep(0.01)
        except (SerialException, OSError) as ex:
            eprint(f"⚠️  Conexión perdida ({ex}). Reintentando…")
            break
    try:
        ser.close()
    except Exception:
        pass

def main():
    parser = argparse.ArgumentParser(
        description="Escucha /dev/ttyUSB0 cuando esté disponible y autoprueba bauds."
    )
    parser.add_argument(
        "-p", "--port", default="/dev/ttyUSB0",
        help="Puerto serie (por defecto: /dev/ttyUSB0)"
    )
    parser.add_argument(
        "-b", "--bauds", default="128000,115200",
        help="Lista de bauds separados por coma (ej: 128000,115200)"
    )
    parser.add_argument(
        "--probe-seconds", type=float, default=3.0,
        help="Segundos para probar cada baud antes de cambiar (por defecto: 3.0)"
    )
    parser.add_argument(
        "--scan-interval", type=float, default=0.5,
        help="Segundos entre verificaciones cuando el puerto no existe (por defecto: 0.5)"
    )
    args = parser.parse_args()

    bauds = []
    for b in args.bauds.split(","):
        b = b.strip()
        if not b:
            continue
        try:
            bauds.append(int(b))
        except ValueError:
            eprint(f"Baud inválido ignorado: {b}")
    if not bauds:
        eprint("No hay bauds válidos. Usa, por ejemplo: --bauds 128000,115200")
        sys.exit(2)

    eprint(f"🎧 Esperando {args.port}… probaré bauds: {', '.join(map(str, bauds))}")

    while RUNNING:
        # Espera a que el dispositivo aparezca
        while RUNNING and not port_exists(args.port):
            time.sleep(args.scan_interval)

        if not RUNNING:
            break

        # El puerto existe: probar cada baud
        for baud in bauds:
            if not RUNNING:
                break

            eprint(f"🔌 Intentando {args.port} @ {baud} 8N1 …")
            ser = try_open(args.port, baud)
            if ser is None:
                # No se pudo abrir (ocupado o permisos). Reintenta con siguiente baud o espera.
                continue

            # Probar si hay datos
            got_data = False
            try:
                got_data = probe_for_data(ser, seconds=args.probe_seconds)
            except Exception:
                got_data = False

            if got_data:
                # Mantener lectura continua con este baud
                read_forever(ser, args.port, baud)
                # Si se sale de read_forever, es porque se perdió la conexión o Ctrl+C.
                # Volver al ciclo externo para reintentos / reconexión.
                break
            else:
                eprint(f"⏭️  Sin datos en {baud} tras {args.probe_seconds}s. Probando siguiente…")
                try:
                    ser.close()
                except Exception:
                    pass
                # probar siguiente baud

        # si llegamos aquí sin estar RUNNING, salimos; si no, dormir un poco y volver a esperar
        if RUNNING:
            time.sleep(0.5)

    eprint("👋 Saliendo.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
