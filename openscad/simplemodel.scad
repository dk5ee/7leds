
hol1x=22.4;
hol1y=-0.5;

hol2x=-12.6;
hol2y=-14.4;

hol3x=-12.1;
hol3y=16;
ledch=4;
ledcr=3;
ledlr=2;
ledoff=11;

translate([-125,60,0]) 
import("freecadexport.stl");

module LED() {
    cylinder(ledch,ledcr,ledcr);
    translate([0,0,ledch])
    sphere(ledlr);
}
for ( i = [1:1:7]) {
    rotate([0,0,-7+i*360/7])
        translate([-ledoff,0,0]) LED();
}