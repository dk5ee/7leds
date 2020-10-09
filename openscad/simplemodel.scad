$fn=100;

epsilon=0.001;
hol1x=22.4;
hol1y=-0.5;
dimx=38.5;
dimy=49.5;
cylR= sqrt(dimx*dimx*0.25+(dimy*dimy*0.25));

hol2x=-12.6;
hol2y=-14.4;

hol3x=-12.1;
hol3y=16;
ledch=4;
ledcr=8/2;
ledlr=6/2;
ledoff=11;
ledoff2=2;
hexathicknes=4;
ledholeh = hexathicknes*1.1;
ledholetopr = 5.5;
hexaheight = 60;
hexabaseR = 16;
hexaangle = 16;



module LED() {
    cylinder(ledch,ledcr,ledcr);
    translate([0,0,ledch])
    sphere(ledlr);
}

module platine() {
translate([-125,60,0]) 
import("freecadexport.stl");

for ( i = [1:1:7]) {
    rotate([0,0,i*360/7])
        translate([-ledoff,ledoff2,0]) LED();
}
}


module LEDhole() color("red") {
    translate([0,0,ledch-epsilon])
    cylinder(ledholeh+2*epsilon,ledlr,ledholetopr);
}
module allLEDholes()
for ( i = [1:1:7]) {
    rotate([0,0,i*360/7])
        translate([-ledoff,ledoff2,0]) LEDhole();
}

module hexablock() {
rotate([hexaangle,0,0])
translate([-50,-hexabaseR,-20])
cube ([100,100,100]);
}
module hexablocks() {
   
    intersection() {
        rotate([0,0,0]) hexablock();   
        rotate([0,0,60]) hexablock();
        rotate([0,0,120]) hexablock();
        rotate([0,0,180]) hexablock();
        rotate([0,0,-60]) hexablock();
        rotate([0,0,-120]) hexablock();
        translate([-200,-200,0])
        cube ([400,400,hexaheight]);
    }
}

module screwhole() {
    translate([0,0,-10])
    cylinder(100,1.25,1.25);
    translate([0,0,hexathicknes+ledch-0.25])
    cylinder(100,3.25,3.25);
}

difference() {
 translate([0,0,ledch])
difference() {
intersection() {
    
    union(){
    translate([0,0,hexathicknes])
    minkowski () {
        hexablocks();
        sphere(hexathicknes);
        
    }
    cylinder(48,cylR,0.75*cylR);
    }
      translate([-200,-200,0])
    cube ([400,400,hexaheight]);
}
translate([0,0,hexathicknes+epsilon]) 
hexablocks();
}
allLEDholes();
screwhole();
translate([-hol1x,-hol1y,0])screwhole();
translate([-hol2x,-hol2y,0])screwhole();
translate([-hol3x,-hol3y,0])screwhole();
}

//platine();
