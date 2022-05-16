# Crypto led ticker
![VID_20220516_031840 (3)](https://user-images.githubusercontent.com/49351206/168514664-b4d46868-bf53-447a-b277-4ffd8d9d9578.gif)

After setting up with your local network it gathers data every 30s and display current prices of bitcoin and ethereum.

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
