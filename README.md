# KitchenControl
Kitchen Control for Manuels new kitchen<br>
ATTENTION: program development is still in progress!<br>
(S/W compiles, but NOT yet tested)

used board: Adafruit Bluefruit nRF52 Feather <br><br>
- receives 3 PIR signals to detect human presence (hopefully not the hungry cats) <br>
- powers up 2 power-supplies for 5V and 12V <br>
- controls kitchen work bench light by two WW LED strips via 2 PWM outputs <br>
- receives serial message from 3 Drawer-Detect-controllers (Pro Trinket 5V and several US sensors)<br>
Pls. see files in erWINter/DrawerDetect<br>
- steers color effects and brightness for RGB-digital strip (placed below the work bench front), when drawer is opened<br>
<br>Pls. have a look at the signal overview! KitchenControl/KitchenControl-OvwSignal.png<br>
( a drawing is always hard to describe, but saves a lot of description ! )
