# dsPIC + Adafruit MCP2221A 與電腦連線範例

Adafruit MCP2221A 插上電腦後會出現一個 **USB 虛擬序列埠 (CDC)**，同時也是 HID 的 I2C/GPIO 橋接器。
本範例使用最簡單也最穩定的方式：**UART**。

```
 PC (USB)  <──>  MCP2221A  <── UART 115200 8N1 ──>  dsPIC33CK
  Python / 終端機            TX ──────────> RB15 (U1RX)
                             RX <────────── RB14 (U1TX)
                             GND ────────── GND
```

## 接線

| Adafruit MCP2221A | dsPIC33CK256MP508 | 說明 |
|---|---|---|
| **TX** | RB15 (RP47, U1RX) | MCP2221A 送出 → dsPIC 接收 |
| **RX** | RB14 (RP46, U1TX) | dsPIC 送出 → MCP2221A 接收 |
| **GND** | GND | **一定要共地** |
| 3V (選用) | VDD | 只有 dsPIC 板沒有自己供電時才接 |

- MCP2221A 板子的 I/O 為 3.3V，dsPIC33 也是 3.3V，可直接連接。
- TX/RX 要**交叉**接。

## 檔案

```
firmware/
  config.h   時脈、鮑率、腳位設定 (換晶片/換腳位只改這裡)
  uart.c/.h  UART1 驅動 (中斷接收 + 環形緩衝)
  main.c     指令解析主程式
host/
  mcp2221_host.py  電腦端 Python 程式
```

## 韌體建置 (MPLAB X + XC16 / XC-DSC)

1. MPLAB X → New Project → Standalone Project → 裝置選 `dsPIC33CK256MP508` (或你的型號)。
2. 把 `firmware/` 下的 `.c` 加入 Source Files，`.h` 加入 Header Files。
3. 視你的板子修改：
   - `main.c` 的 `#pragma config ICS`（燒錄/除錯腳位）
   - `config.h` 的 UART 腳位與 LED 腳位
4. 編譯並燒錄。

時脈使用內部 FRC 8 MHz (FCY = 4 MHz)，UART 使用 dsPIC33CK 的分數式鮑率模式，
115200 bps 誤差約 0.8%，不需要外部晶振或 PLL。

## 電腦端

### 驅動
- Windows 10/11、Linux、macOS 都內建 CDC 驅動，插上即可。
  - Windows：裝置管理員會出現 `USB 序列裝置 (COMx)`
  - Linux：`/dev/ttyACM0`（若權限不足：`sudo usermod -aG dialout $USER` 後重新登入）

### 用終端機測試
任何序列埠工具 (PuTTY、Tera Term、Arduino Serial Monitor、`screen /dev/ttyACM0 115200`) 設定
**115200, 8N1**，行尾送 CR 或 LF，輸入 `HELP` 即可。

> MCP2221A 的 UART 鮑率是**由電腦端開啟序列埠時的設定決定**的，必須跟 dsPIC 一致。

### 用 Python
```bash
cd host
pip install -r requirements.txt
python mcp2221_host.py --list          # 列出序列埠 (MCP2221A 為 04D8:00DD)
python mcp2221_host.py --demo          # 自動找 MCP2221A 並跑測試
python mcp2221_host.py -p COM5         # 指定埠、互動模式
```

預期輸出：
```
已連線 COM5 @ 115200
>> PING
<< PONG
>> LED ON
<< OK
>> ECHO hello dsPIC
<< hello dsPIC
>> CNT
<< CNT 4
...
```

## 指令表

| 指令 | 回應 |
|---|---|
| `PING` | `PONG` |
| `LED ON` / `LED OFF` | `OK` |
| `ECHO <文字>` | `<文字>` |
| `CNT` | `CNT <已處理指令數>` |
| `HELP` | 指令列表 |
| 其他 | `ERR unknown command: ...` |

## 移植到其他 dsPIC

本範例以 dsPIC33CK 為準。若使用其他系列：

| 系列 | 需要修改 |
|---|---|
| dsPIC33EP / dsPIC33FJ | PPS 解鎖改用 `__builtin_write_OSCCONL(OSCCON & ~0x40)`；UART 改用 `U1MODEbits.BRGH` + `U1BRG = FCY/(16*baud)-1`、`U1STAbits.UTXBF` / `U1STAbits.URXDA`；RP 編號與 RPOR 暫存器依 datasheet |
| dsPIC30F (例如 30F4011) | 無 PPS，UART 腳位固定；`U1BRG = FCY/(16*baud)-1`，建議使用 7.3728 MHz 晶振 × PLL 才能得到準確鮑率 |
| dsPIC33CH / dsPIC33CK 其他型號 | 基本相同，只需確認腳位與 RP 編號 |

## 常見問題

- **完全沒回應**：檢查 TX/RX 是否交叉、是否共地、電腦端鮑率是否 115200。
- **亂碼**：鮑率不符或 FCY 設定錯誤（`config.h` 的 `FOSC` 必須是實際時脈）。
- **只收到開機訊息但指令無效**：確認終端機有送出換行 (CR 或 LF)。
