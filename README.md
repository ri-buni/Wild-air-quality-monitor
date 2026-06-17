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

ItemDescriptionQtyUnit PriceLine TotalSourcePCBCustom 2-layer PCB1$12.00$12.00JLCPCBESP32-S3 DevKitCMCU dev board1$5.00$5.00AliExpress2.42" OLED128x64 SSD1309, I2C/SPI1$12.00$12.00AliExpressSGP40VOC gas sensor module1$5.00$5.00AliExpressSCD41CO2 + temp/humidity sensor1$14.00$14.00AliExpressSHT41Temp/humidity sensor1$1.50$1.50AliExpressLi-ion battery500mAh 3.7V (502535)1$5.10$5.10AliExpressTP4056Li-ion charger module1$0.50$0.50AliExpressPMS5003Laser PM2.5 sensor1$13.20$13.20AliExpressGRAND TOTAL$68.30

# Details:

I've used this tutorial as a reference: https://howtomechatronics.com/projects/diy-air-quality-monitor-pm2-5-co2-voc-ozone-temp-hum-arduino-meter/

