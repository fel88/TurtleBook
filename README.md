# TurtleBook

"_One book to read them all_"

<img src="https://github.com/user-attachments/assets/834d9b57-e441-4399-a74a-d50caecb808d" width="500"  />

<br/>
<img src="https://github.com/user-attachments/assets/31bb9521-6a11-493c-bf02-1f4a64062a83" width="200"  />

<img src="https://github.com/user-attachments/assets/b57e9fb0-8926-4ada-921e-aba7ac638fbe" width="200"  />
<img src="https://github.com/user-attachments/assets/657d3588-7098-4fa6-a373-ae8c5ec83855" width="200"  />


Zero buttons, solar powered, fully autonomous and hermetic e-book reader.

For everyone who loves reading books and DIY projects.
WYSIWYG e-book. 

### Hardware
- Arduino MEGA2560 (https://aliexpress.ru/item/32967417746.html?sku_id=66568659813)
- Waveshare E-ink display 5,83” (648×480)  https://www.waveshare.com/5.83inch-e-paper.htm
- FRAM memory
- OLED display
- INA219 (https://aliexpress.ru/item/33047166203.html?sku_id=12000044300903777)
- SD-card slot
- MPU6050 (instead of buttons)
- Reed switch (turn on/off)
- Lithium-ion capacitors (LIC) 4x200F  (https://en.cda-cap.com/index.php/product/lib-series/)
- Solar panel (something like this https://aliexpress.ru/item/1005008193807038.html?sku_id=12000044193770138)
- Buck-boost converter (https://aliexpress.ru/item/1005001525710076.html)
- NeoPixel (not necessarily)
- Other stuff: diode, transistor, resistors, etc
- Wires (https://aliexpress.ru/item/32793368381.html?sku_id=64310577090)
  
![image](https://github.com/user-attachments/assets/c51009ef-ac24-424c-9be7-da2aa96eabaf)

### How to switch pages?


https://github.com/user-attachments/assets/5e0592eb-202e-4b6f-8b21-0b80249d7fce


### How to convert books?
Use https://github.com/fel88/pdf2eink

### Sample books
<a href="https://github.com/fel88/TurtleBook/blob/main/samples/nemo.zip">Twenty Thousand Leagues Under the Seas (Jules Verne)</a>

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


Also It was inspired by my old project https://github.com/fel88/BIP and couple of github projects (https://hackaday.io/project/189366-supercaps-solar-panel-project/details) solar panel + supercap (https://github.com/aresta/Supercaps  it seems that it was moved to https://github.com/aresta/ESP32-C3_Supercaps )

Great quote from Aresta/Supercaps project:
![image](https://github.com/user-attachments/assets/18651167-2490-474c-9cd1-44f70db3f667)

