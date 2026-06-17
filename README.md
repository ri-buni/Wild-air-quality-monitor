# Wild-air-quality-monitor
An air quality monitor I've decided to make because of how bad the air in my city can be sometimes.
It has sht41, sgp40, scd41, PMS5003 as sensors for temperature, humidity, VOC CO2 and PM, 2"42 display and esp32 devkitc

# CAD
Made in Fusion360.

<img width="622" height="751" alt="anydesk00018" src="https://github.com/user-attachments/assets/0cca1ec1-856b-43f5-bad3-313317c76ac3" />

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

- [PCB](https://jlcpcb.com/) ~12$ for me
- [ESP32 S3 devkitc](https://aliexpress.ru/item/1005007820190456.html?sku_id=12000058365460928&spm=a2g2w.productlist.search_results.2.540076b71PPoYp) ~3.7$
- [2"42 OLED](https://aliexpress.ru/item/4000002579405.html?sku_id=12000037978104081&spm=a2g2w.productlist.search_results.0.30ada5b3CcquXc) ~7.25$
- [sgp40](https://aliexpress.ru/item/1005008380047927.html?sku_id=12000056592935021&spm=a2g2w.productlist.search_results.0.65214052kKBEOa) ~4.6$
- [scd41](https://aliexpress.ru/item/1005010621571588.html?sku_id=12000053002326414&spm=a2g2w.productlist.search_results.1.6427406cKYYdqG) ~15$
- [sht41](https://aliexpress.ru/item/1005008842184215.html?sku_id=12000057743184144&spm=a2g2w.productlist.search_results.0.31de4062I4IYzH) ~2$
- [500mAh 3.7V li ion battery](https://aliexpress.ru/item/1005006462526013.htmlshpMethod=Other&sku_id=12000037283411655&spm=a2g2w.productlist.search_results.0.7e9aae051GM2Nw) ~5.3$
- [TP4056 battery charger](https://aliexpress.ru/item/32453058256.html?spm=a2g2w.cart.cart_split.2.233e4aa6X9uCmK&sku_id=12000052763412620) ~0.5$
- [PMS5003](https://aliexpress.ru/item/32772775425.html?sku_id=12000027208101193&spm=a2g2w.productlist.search_results.0.38b27b89w3cIQV) ~13$

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

