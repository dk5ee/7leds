# Python 3 server example
from http.server import BaseHTTPRequestHandler, HTTPServer
from uritools import urisplit
from urllib.parse import urlparse
import serial
from time import sleep
import json
from socketserver import ThreadingMixIn
import threading
import cv2
from PIL import Image
from io import BytesIO

enableSerial = 1
enableVideo = 1

hostName = "localhost"
serverPort = 8080
proxyAddress = "http://localhost:8080/"


class campictureclass:
    image = None
    imagesize = 0
    imagecount = 0
    videocapture = None
    callbacks = []

    def __init__(self):
        if enableVideo:
            self.videocapture = cv2.VideoCapture(0)
            if not self.videocapture.isOpened():
                self.videocapture.open()
            print ("camera connected")
        self.callbacks = []

    def __del__(self):
        if enableVideo:
            self.videocapture.release()
            print ("camera disconnected")

    def subscribe(self, callback):
        if enableVideo:
            self.callbacks.append(callback)

    def fire(self):
        for callback in self.callbacks:
            if callable(callback):
                callback()
            else:
                self.callbacks.remove(callback)  # weird..

    def getnewframe(self):
        ret, frame = self.videocapture.read()
        if ret:
            self.imagecount = self.imagecount + 1
            dim = (320, 200)
            scaledimage = cv2.resize(frame, dim, interpolation=cv2.INTER_AREA)
            imageRGB = cv2.cvtColor(scaledimage, cv2.COLOR_BGR2RGB)
            raw = Image.fromarray(imageRGB)
            tmpFile = BytesIO()
            raw.save(tmpFile, 'JPEG')
            self.imagesize = tmpFile.getbuffer().nbytes
            self.image = tmpFile.getvalue()
            self.fire()
        else:
            self.image = None


def getframesinbackground():
    while(enableVideo): # todo: find better way instead of polling
        sleep(0.2)
        campicture.getnewframe()


class serialclass:
    blockserial = 0;
    serialport = None
    
    def __init__(self):
        if enableSerial:
            self.serialport = serial.Serial('/dev/ttyUSB3', 9600)
            self.serialport.isOpen()
            print ("serial opened")

    def __del__(self):
        if enableSerial:
            self.serialport.close()
            print ("serial closed")

    def sendserial(self, name, value):
        if enableSerial and self.serialport is not None:
            self.serialport.write(value.encode())
            self.serialport.write(name.encode())

    def sendledstoUART(self):
        while (self.blockserial > 0):
            sleep(0.01) #todo this is ugly
        self.blockserial = 1;
        changedvals = 0
        for led in ledvalues.ledsdict:
            value = str(ledvalues.ledsdict[led]["value"])
            changed = ledvalues.ledsdict[led]["changed"]
            name = ledvalues.ledsdict[led]["name"]
            if changed > 0:
                self.sendserial(name, value)
                ledvalues.ledsdict[led]["changed"] = 0
                changedvals = changedvals + 1
        self.blockserial = 0;
        return changedvals


class ledvaluesclass:
    changed = 0
    ledsdict = {
      "white": {"name":"d", "value":0, "color":"#fff", "changed":1},
      "nearUV": {"name":"f", "value":0, "color":"#30f", "changed":1},
      "blue1": {"name":"a", "value":0, "color":"#00f", "changed":1},
      "blue2": {"name":"c", "value":0, "color":"#00f", "changed":1},
      "yellow": {"name":"b", "value":0, "color":"#ff0", "changed":1},
      "green": {"name":"g", "value":0, "color":"#0f0", "changed":1},
      "red": {"name":"e", "value":0, "color":"#f00", "changed":1}
    }
    callbacks=[]
    def subscribe(self, callback):
        self.callbacks.append(callback)
    def unsubscribe(self, callback):
        if callback in self.callbacks:
            self.callbacks.remove(callback)
    def trigger(self):
        if self.changed:
            self.changed = 0
            for callback in self.callbacks:
                if callable(callback):
                    if not self.changed:
                        callback()
                self.callbacks.remove(callback)  # weird..

    def setled(self, ledname, newvalue):
        lednameasc = str(ledname)
        if lednameasc in self.ledsdict:
            oldvalue = self.ledsdict[lednameasc]["value"]
            if oldvalue != newvalue:
                self.ledsdict[lednameasc]["value"] = str(newvalue)
                self.ledsdict[lednameasc]["changed"] = 1
                self.changed = 1;
            return 1
        return 0

class MyServer(BaseHTTPRequestHandler):

    def buildjsstring(self):
        url = str(proxyAddress)
        returnvalue = bytes ("""
<script>
function sendvalue(name, value) {
const Http = new XMLHttpRequest();
const url = '%s';
var geturl = url + "?" +name+ "=" +value;
Http.open("GET", geturl);
Http.send();
}
var slidermap= new Map();
""" % url, "utf-8")
        for led in ledvalues.ledsdict:
            # color = ledsdict[led]["color"]
            # name = ledsdict[led]["name"]
            returnvalue = returnvalue + bytes("""
slidermap["slider%s"] = true;
var slider%s = document.getElementById("slider%s");
slider%s.oninput = function() { sendvalue("%s",this.value); }
slider%s.onmousedown = function() { 
    slidermap["slider%s"] = false;
}
slider%s.onmouseup = function() { 
    slidermap["slider%s"] = true;
}
""" % (led, led, led, led,led, led, led, led,led), "utf-8")
        returnvalue = returnvalue + bytes("""
var running = 1;

setInterval(function(){
    if (running) {
    load("/ask", function(response) { 
        data=JSON.parse(response);
        for (const name in data) {
            if (data.hasOwnProperty(name)) {
                entry = data[name];
                if (entry.hasOwnProperty('value') && slidermap["slider"+name] )
                   document.getElementById("slider"+name).value = entry['value']
            }
        } 
    });
    }
}, 10);
function load(url, callback) {
    running = 0;
    const Http = new XMLHttpRequest();
    Http.timeout = 50000;
    Http.onreadystatechange = stateChange;
    Http.ontimeout = function (e) {
    document.getElementById('container').innerHTML = "lost connection"
};
    function stateChange() {
        if(Http.readyState < 4) {  return; }
        if(Http.status !== 200) {  return; }
        if(Http.readyState === 4) {
            callback(Http.responseText);
            running = 1;
        }
    }
    Http.open("GET", url);
    Http.send();
}
""", "utf-8")
        returnvalue = returnvalue + bytes("""
</script>
    """, "utf-8")
        return returnvalue

    def buildledsliders(self):
        returnvalue =bytes("""
<div >
""" , "utf-8")
        for led in ledvalues.ledsdict:
            color = ledvalues.ledsdict[led]["color"]
            value = ledvalues.ledsdict[led]["value"]
            line = bytes("""
<span style="background-color: %s">
    <input type="range" min="0" max="255" value="%s" class="slider" id="slider%s" style="width: 512px;">
</span>
<span style="display: inline-blockserial;width:6em;">%s</span>
<br>\n""" % (color, str(value), led, led), "utf-8")
            returnvalue = returnvalue + line
        returnvalue = returnvalue +bytes("""
</div>
""" , "utf-8")
        return returnvalue

    def mainpage(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(bytes("""<!doctype html>
<html lang="en">
    <head><meta charset="utf-8">
    <title>PYLED</title>
    <link rel="shortcut icon" href="data:image/x-icon;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD////////////////c9/7/KMn7/wK7+v8ClNn/AqPt/yNXfv8Cufn/Aq3q/wKx+P8CgMz/XJO8/7Gxsv/DxMT//////////////////v///6fz/v9Prsj/G3Kd/yNujP81Qk3/KGBx/x9XeP8gXnj/Y5m0/93m6//s7Oz/7u7u////////////z8/P//X19f+psrr/c42j/3GMo/99kaH/g5Gd/32Mmf9zgYz/bXV7/29ydf+Li4z/9vb2/8PDw////////////1FRUf+Bg4X/u8LI/8fP1//ByM7/tLm+/7C1uf+nrLD/naOo/5qgpP+YnJ//k5WW/3V1df8WFhb///////////88PDz/tLS1//b4+f/u8fP/9/j6/+rs7v/d3uD/tba3/7Cys/+trrD/qKmq/6Chov8zMzP/Ghoa////////////aGho/5OTk//5+vr/7e7v/+Dh4v/P0NH/q6ys/6Chof+wsbL/w8TF/7W2t/+dnp//FhYW/0NDQ////////////+np6f9OTk7/6+zt/8rKy/+bnJ3/a3iC/1dtgP9rcXb/ioqL/62trv/Cw8T/XFxd/ygoKP/Ozs7/////////////////YWFh/9PT0/9xv93/EWqT/wBIeP8AH4f/Ajln/xJPf/9GiLr/v8DB/zg4OP9WVlb/////////////////+Pj4/4SEhP//////2/3//132//8C7/7/ANX8/wGu/v9SwPj/uNbp/+Hj5P+6u7z/GRoa//j4+P///////////9fX1/+0tLX///////////9NTk7/bLS4/07s/v9gi6D/WFpc//n6+//r7O3/3t/g/ysrK//V1dX////////////f39//sbGx////////////vLy8/+np6f//////4ODg/729vf///////Pz8/+Pk5f8rKyz/3t7e/////////////v7+/4OEhP/////////////////////////////////////////////////Ozs//Kisr//7+/v////////////////+JiYr/tra2////////////8PDw/39/f//z8/P////////////o6Oj/QEBA/4CAgP//////////////////////9vb2/0RERP9UVFT/aGho/ygoKf8UFBT/Kioq/21tbf9oaGj/JCQk/z09Pv/19fX////////////////////////////s7Oz/UVFR/xkaGv8YGBj/FxcX/xgYGP8ZGRn/Ghoa/05OTv/r6+v///////////////////////////////////////z8/P+5ubn/cXFx/0BBQf9AQED/cXFx/7i4uP/8/Pz/////////////////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==" />
</head>
<body style="color: #888;background-color: #000;" >""" , "utf-8"))
        if enableVideo:
            self.wfile.write(bytes("<div id=\"container\"><img src=\"/video\" alt=""></div>", "utf-8"))
        self.wfile.write(self.buildledsliders())
        self.wfile.write(self.buildjsstring())
        self.wfile.write(bytes("""
<a href="https://github.com/dk5ee/7leds/tree/master/webserver">dk5ee 7leds</a>
</body></html>""", "utf-8"))

    def longpollcallback(self):
        self.longpollwaiting=0;
        try:
            self.wfile.write(bytes(json.dumps(ledvalues.ledsdict), "utf-8"))
        except BrokenPipeError:
            pass
    def longpoll(self) :
        self.longpollwaiting = 1
        self.sleepcount = 0
        ledvalues.subscribe(self.longpollcallback)
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.end_headers()
        while (self.longpollwaiting and self.sleepcount < 20):
            sleep(1) 
            self.sleepcount = self.sleepcount + 1
        if self.longpollwaiting:
            self.wfile.write(bytes(json.dumps(ledvalues.ledsdict), "utf-8"))
        ledvalues.unsubscribe(self.longpollcallback)
        
    def videostreamcallback(self):
        if self.goon:
            try:
                self.wfile.write(bytes("\n--jpgboundary\n", "utf-8"))  # todo: check the linebreaks
                self.send_header("Content-Type", "image/jpeg")
                self.send_header("Content-Length", campicture.imagesize)
                self.end_headers()
                self.wfile.write(campicture.image)
                self.wfile.flush()
            except BrokenPipeError:
                self.goon = 0

    def videostream(self):
        if enableVideo:
            self.send_response(200)
            self.send_header("Content-Type", "multipart/x-mixed-replace; boundary=jpgboundary")
            self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0")
            self.end_headers()
            campicture.subscribe(self.videostreamcallback)
            self.goon = enableVideo
            while self.goon:
                sleep(1.0) # todo: ugly, again

    def oneimage(self):
        if enableVideo:
            self.send_response(200)
            self.send_header("Content-type", "image/jpeg")
            self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0")
            self.end_headers()
            self.wfile.write(campicture.image)
            self.wfile.flush()

    def do_GET(self):
        parts = urisplit(self.path)
        params = parts.getquerydict()
        uriparts = urlparse(self.path)
        mypath = uriparts.path
        if (mypath == "/video" and enableVideo):
            self.videostream()
        elif (mypath == "/ask"):
            self.longpoll()
        elif (mypath == "/image" and enableVideo):
            self.oneimage()
        else:
            hasparam = 0
            for param in params:
                hasparam = hasparam + ledvalues.setled(param, params[param][0])
            if hasparam > 0:
                self.send_response(200)
                self.send_header("Content-type", "text/html")
                self.end_headers()
                myserialport.sendledstoUART()
                self.wfile.write(bytes("ok", "utf-8"))
                ledvalues.trigger()
            else:
                self.mainpage()


class ThreadingSimpleServer(ThreadingMixIn, HTTPServer):
    pass


if __name__ == "__main__":
    myserialport = serialclass()
    ledvalues = ledvaluesclass()
    campicture = campictureclass()
    server = ThreadingSimpleServer((hostName, serverPort), MyServer)
    videothread = threading.Thread(target=getframesinbackground, name="videothread")
    videothread.start()
    print("Server started http://%s:%s" % (hostName, serverPort))
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    server.server_close()
    print("server stopped")
