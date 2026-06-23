# Wild-air-quality-monitor
An air quality monitor I've decided to make because of how bad the air in my city can be sometimes.
It has sht41, sgp40, scd41, PMS5003 as sensors for temperature, humidity, VOC CO2 and PM, 2"42 display and esp32 devkitc

# CAD
Made in Fusion360.

<img width="780" height="718" alt="anydesk00012" src="https://github.com/user-attachments/assets/799ccd82-0996-4be4-80e8-90f7a759d4b3" />

<img width="924" height="732" alt="anydesk00011" src="https://github.com/user-attachments/assets/6d5d48a6-da91-47e9-a85d-187dec9327ad" />

<img width="475" height="732" alt="anydesk00013" src="https://github.com/user-attachments/assets/ad709b05-cf3c-4ef4-b8e9-b8d97f48a0b7" />

# PCB
Made in KiCad.

Schematic:

<img width="1178" height="797" alt="anydesk00009" src="https://github.com/user-attachments/assets/dd0bf59b-dc26-4477-8ab5-51a459a6c4b1" />

PCB:

<img width="898" height="792" alt="anydesk00010" src="https://github.com/user-attachments/assets/f3be5db6-fd33-4cb8-96dd-c66859a83456" />

# Firmware was made by Claude Code

Display looks like this:

<img width="258" height="125" alt="Screenshot (24)" src="https://github.com/user-attachments/assets/0ae3b5cb-2c84-48f2-bd27-37f51f2a9ee9" />

# BOM

| Item | Description | Qty | Unit Price | Line Total | Source |
|---|---|---|---|---|---|
| PCB | Custom 2-layer PCB | 1 | $12.00 | $12.00 | [JLCPCB](https://jlcpcb.com/) |
| ESP32-S3 DevKitC | MCU dev board | 1 | $5.00 | $5.00 | [AliExpress](https://aliexpress.ru/item/1005007820190456.html?sku_id=12000058365460928) |
| 2.42" OLED | 128x64 SSD1309, I2C/SPI | 1 | $12.00 | $12.00 | [AliExpress](https://aliexpress.ru/item/4000002579405.html?sku_id=12000037978104081) |
| SGP40 | VOC gas sensor module | 1 | $5.00 | $5.00 | [AliExpress](https://aliexpress.ru/item/1005008380047927.html?sku_id=12000056592935021) |
| SCD41 | CO2 + temp/humidity sensor | 1 | $14.00 | $14.00 | [AliExpress](https://aliexpress.ru/item/1005010621571588.html?sku_id=12000053002326414) |
| SHT41 | Temp/humidity sensor | 1 | $1.50 | $1.50 | [AliExpress](https://aliexpress.ru/item/1005008842184215.html?sku_id=12000057743184144) |
| Li-ion battery | 500mAh 3.7V (502535) | 1 | $5.10 | $5.10 | [AliExpress](https://aliexpress.ru/item/1005006462526013.html?sku_id=12000037283411655) |
| TP4056 | Li-ion charger module | 1 | $0.50 | $0.50 | [AliExpress](https://aliexpress.ru/item/32453058256.html?sku_id=12000052763412620) |
| PMS5003 | Laser PM2.5 sensor | 1 | $13.20 | $13.20 | [AliExpress](https://aliexpress.ru/item/32772775425.html?sku_id=12000027208101193) |
| **GRAND TOTAL** |  |  |  | **$68.30** |  |

# Details:

I've used this tutorial as a reference: https://howtomechatronics.com/projects/diy-air-quality-monitor-pm2-5-co2-voc-ozone-temp-hum-arduino-meter/

