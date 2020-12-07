# ESP8266PlainUpdater
Update Over The Air your ESP8266 sketch using internal Update library.
Included in your sketch, PlainUpdater class can be used to monitor UDP broadcasts and identify itself over the WiFi network.
This allows other programs to discover local devices and port where the device is listening for updates and push the new sketch to this port.

PlainUpdater may also check for updates on startup. Use PlainUpdater::emergency() in void setup() to check at specific IP/Port for update.
It is usefull if you sketch is hanging and the device has been reset by the watchdog.

In the /server directory you may find an example on C++/QT how to make PUSH and PULL updates.
You may try compiled version for Windows 10 x64, located at server/WinX64bin. Download the zip, extract it and run the Executable inside.

Enjoy!
