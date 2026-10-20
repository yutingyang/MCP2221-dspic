#!/usr/bin/env python3
"""
透過 Adafruit MCP2221A (USB 虛擬序列埠) 與 dsPIC 通訊的電腦端程式。

用法:
    python mcp2221_host.py --list              # 列出序列埠
    python mcp2221_host.py                     # 自動尋找 MCP2221A 並進入互動模式
    python mcp2221_host.py -p COM5 --demo      # 指定埠，跑一次自動測試
"""
import argparse
import sys
import time

import serial
from serial.tools import list_ports

MCP2221_VID = 0x04D8
MCP2221_PID = 0x00DD


def find_mcp2221():
    for p in list_ports.comports():
        if p.vid == MCP2221_VID and p.pid == MCP2221_PID:
            return p.device
    return None


def send(ser, cmd):
    ser.write((cmd + "\r\n").encode("ascii"))
    return ser.readline().decode("ascii", errors="replace").strip()


def demo(ser):
    tests = ["PING", "LED ON", "ECHO hello dsPIC", "CNT", "LED OFF", "HELP"]
    for cmd in tests:
        print(f">> {cmd}")
        print(f"<< {send(ser, cmd)}")
        time.sleep(0.3)


def interactive(ser):
    print("輸入指令 (HELP 查看，Ctrl+C 離開)")
    try:
        while True:
            cmd = input("> ").strip()
            if cmd:
                print(send(ser, cmd))
    except (KeyboardInterrupt, EOFError):
        print()


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-p", "--port", help="序列埠 (例如 COM5 或 /dev/ttyACM0)")
    ap.add_argument("-b", "--baud", type=int, default=115200)
    ap.add_argument("--list", action="store_true", help="列出所有序列埠")
    ap.add_argument("--demo", action="store_true", help="執行自動測試")
    args = ap.parse_args()

    if args.list:
        for p in list_ports.comports():
            vidpid = f"{p.vid:04X}:{p.pid:04X}" if p.vid is not None else "----:----"
            print(f"{p.device:15} {vidpid}  {p.description}")
        return

    port = args.port or find_mcp2221()
    if port is None:
        sys.exit("找不到 MCP2221A，請用 --list 確認或用 -p 指定")

    # MCP2221A 的 UART 鮑率由電腦端開埠時設定，必須與 dsPIC 相同
    with serial.Serial(port, args.baud, timeout=1) as ser:
        print(f"已連線 {port} @ {args.baud}")
        time.sleep(0.1)
        ser.reset_input_buffer()
        if args.demo:
            demo(ser)
        else:
            interactive(ser)


if __name__ == "__main__":
    main()
