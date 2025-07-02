# TurtleBook

"_One book to read them all_"

<img src="https://github.com/user-attachments/assets/834d9b57-e441-4399-a74a-d50caecb808d" width="500"  />

<br/>
<img src="https://github.com/user-attachments/assets/31bb9521-6a11-493c-bf02-1f4a64062a83" width="200"  />

<img src="https://github.com/user-attachments/assets/b57e9fb0-8926-4ada-921e-aba7ac638fbe" width="200"  />
<img src="https://github.com/user-attachments/assets/657d3588-7098-4fa6-a373-ae8c5ec83855" width="200"  />


Zero buttons, solar powered, fully autonomous and hermetic e-book reader.

For everyone who loves reading books and DIY projects.

Instructables:
https://www.instructables.com/Solar-Powered-Zero-Buttons-E-book-Reader/

WYSIWYG e-book. 

### Hardware
- Arduino MEGA2560 (https://aliexpress.ru/item/32967417746.html?sku_id=66568659813)
- Waveshare E-ink display 5,83” (648×480)  https://www.waveshare.com/5.83inch-e-paper.htm
- FRAM memory (FM24W256-GTR)
- WaveShare shield https://www.waveshare.com/product/displays/e-paper/e-paper-shield-b.htm
- Crystal oscillator KSE-7U16000MAB143ZA3
- OLED display (128x32 i2c)
- INA219 (https://aliexpress.ru/item/33047166203.html?sku_id=12000044300903777)
- SD-card slot
- MPU6050 (instead of buttons)
- Reed switch (turn on/off)
- Lithium-ion capacitors (LIC) 4x200F  (https://en.cda-cap.com/index.php/product/lib-series/) (https://item.szlcsc.com/3567431.html?lcsc_vid=QlRbX1wET1gIUAZeFVIKBFxWEwVWBVUAEVQKVVxUEwMxVlNSR1daUF1WT1RZXztW)
- Solar panel (something like this https://aliexpress.ru/item/1005008193807038.html?sku_id=12000044193770138)
- Buck-boost converter (https://aliexpress.ru/item/1005001525710076.html)
- NeoPixel (not necessarily)
- Other stuff: diode (1N5817), transistor (2N2222), resistors, etc
- Wires (https://aliexpress.ru/item/32793368381.html?sku_id=64310577090)

### How to assemble PCB 

#### Panel driver PCB v4.0 (Charger + Wifi integrated)

<img src="https://github.com/user-attachments/assets/9b24b3ea-a282-4fbd-931f-6a6ded69d2e2" width="75%"  />

#### Panel driver PCB v3.0 (no charger, no wifi)
![image](https://github.com/user-attachments/assets/c51009ef-ac24-424c-9be7-da2aa96eabaf)

You need next components to solder PCB board:
1. Arduino MEGA2560 
2. Waveshare shield
3. MPU6050
4. INA219
5. Crystal quartz
6. FRAM
   
### How to switch pages?


https://github.com/user-attachments/assets/5e0592eb-202e-4b6f-8b21-0b80249d7fce

### How many pages can I read?

If you have 3 LIC charged to 3.88V you can read about 880 pages (LICs will be discharged to 3.05V)

### How to convert books?
Use https://github.com/fel88/pdf2eink

### Sample books
<a href="https://github.com/fel88/TurtleBook/blob/main/samples/nemo.zip">Twenty Thousand Leagues Under the Seas (Jules Verne)</a>

### Wifi (optional)

You can add Wemos D1 v3 module in order to use Wifi 


<img src="https://github.com/user-attachments/assets/8100f600-fab7-446a-a428-500a60b89aca" width="500"  />

There are two ways of WiFi usage at the moment:
1. Wifi Access point: Web page (upload, download books). Use your phone/laptop to load books via browser
2. P2P sender/reciever. Direct book 2 book sharing

Components:
1. Wemos d1 v3 module
2. PNP transistor 2N3906
3. resistor 330

### Charger

A pass (series) regulator based on TL431 (adjust the output voltage level to ~3.95-3.98V)

Components:
1. resistor 330Ohm, 47kOhm
2. TL431
3. D882 (almost any NPN will do)
4. MAX40203
5. Pot 47kOhm

   
![charger5](https://github.com/user-attachments/assets/a7a9190b-997a-478e-9110-a0ad8f0da7ad)

<img src="https://github.com/user-attachments/assets/1be9f826-257e-4922-aa21-57da1e37be66" width="500"  />



### Weight

Approx. 260g (with wemos d1)

### History

I am a big fan of reading books, so I've decided to make a small contribution to the world of e-readers .

I travel on the train a lot and I needed some reliable reader. That's why I decided to make DIY e-book reader.

Default e-readers have so many needless options. But all you need 99% of time is turn to next page.

E-ink paper don't consume power till you read it without switching. So Eink+solar power is very suitable combination of technologies.

This book doesn't contain any physical buttons. There is accelerometer inside the book and it can recognize your gestures. It can be used in discrete way (to switch page or to switch menu position in both directions), or it can be used in analog way (e.g. to set brightness of the NeoPixel led)

To turn on the book you just move special plastic key in special place. The internal reed switch will be activated and the book will turn on.

This book has some distinctions from the default e-reader and satisfy my needs better.

   - Solar powered (more autonomous)
   - No buttons. (you need to make some effort to switch the page like in real book)
   - Hybrid capacitors (LIC) instead of batteries
   - Sealed / hermetic ( you can use compound+expoxy resin to totally sealed the book and it'll become water proof)
   - More reliable plexiglass case . You can put cup of coffee on it or you can throw it in your backpack and not worry that it will break.
   - Nothing useless in the firmware
   - Open hardware and firmware
   - Eink for reading and small OLED for quick response menu operations


### Pre-history

It was inspired by my old project https://github.com/fel88/BIP and couple of github projects (https://hackaday.io/project/189366-supercaps-solar-panel-project/details) solar panel + supercap (https://github.com/aresta/Supercaps  it seems that it was moved to https://github.com/aresta/ESP32-C3_Supercaps )

Great quote from Aresta/Supercaps project:
![image](https://github.com/user-attachments/assets/18651167-2490-474c-9cd1-44f70db3f667)

