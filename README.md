# KitchenControl
Kitchen Control for Manuels new kitchen

used board: Adafruit Bluefruit nRF52 Feather
receives 3 PIR signals to detect human presence (hopefully not the hungry cats)
powers up 2 power-supplies for 5V and 12V
controls kitchen work bench light by two WW LED strips via 2 PWM outputs
receives serial message from 3 Drawer-Detect-controllers (Pro Trinket 5V and several US sensors)
steers color effects/brighness for RGB-digital strip, which are placed above opened drawers and below the work bench front
