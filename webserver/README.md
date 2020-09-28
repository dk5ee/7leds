
Webserver to control LED module

Features:
1. simple API and webinterface for controllng the LED module
2. synchronisation of controls for distinct browsers users
3. pseudo-videostreaming
4. everything runs over http

Requirements on hardware side:
1. Computer with networking
2. Attached LED module
3. Attached Webcam

Requirements on software side:
1. Linux on the computer
2. Python3 with some standard libraries, see 'pyled.py'
3. user needs write access to serial ports. serial port is hard coded in 'pyled.py'
4. user needs read access to webcam. the first webcam is used
5. Free port 8080. the port ist hard coded in 'pyled.py'

port forwarding for internet access:
1. set up linux server with private/public key auth
2. connect: `ssh -R 8088:localhost:8080 user@example.com`
3. on apache, remove for subdomain the "DocumentRoot" and insert:
`ProxyPass / http://127.0.0.1:8088/`
`ProxyPassReverse / http://127.0.0.1:8088/`
4. restart apache
5. change proxyAddress in pyled.py