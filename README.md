# Crypto led ticker
<img src="https://user-images.githubusercontent.com/49351206/168514664-b4d46868-bf53-447a-b277-4ffd8d9d9578.gif" width="350px" height="190px"/>


The ticker displays prices of two main cryptocurrencies - Ether and Bitcoin. It gathers data every 30s, but you can set different interval.

To build this project you need:
  * esp32
  * LED matrix MAX7219 I used 4 units + 3 units, 
    they are usually sold as 4 unites, but you can use as many as you want
  * some cables
  * 3d print which stl files are included in this repo
  * inserts to screw back cover
  * screws
  * additionaly antenna - mine esp32 has week signal

Setting up:
  * solder right pins of led matrix - according to main file - to esp32
  * load program into esp32
  * connect to access point created by controller
  * configure your SSID and password

![IMG_20220516_032641](https://user-images.githubusercontent.com/49351206/168516140-4e75cc4f-21ca-4d38-84e3-fbbd928f04eb.jpg)

![IMG_20220516_032647](https://user-images.githubusercontent.com/49351206/168516155-df955d3f-0315-4705-9438-21730ffc6aef.jpg)
