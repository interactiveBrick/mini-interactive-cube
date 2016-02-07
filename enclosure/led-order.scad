

module side($color, $label) {
  union() {
    // PLANE
    color($color) scale([10, 10, 1]) cube(center=true);

    // BUTTON GRID
	for (j = [0 : 3]) for (i = [0 : 3]) {
      color("black")
      translate([i*1.5-2.25,j*1.5-2.25,0.6])
      scale([1.0,1.0,0.2])
      cube(center=true);
    }
   
      
    // ARROW
    color("white") translate([-2.4, 3.0, 1.3]) union() {
      translate([-0.4, 0.0, 0]) scale([3, 0.5, 0.3]) cube(center=true);
      translate([0.5, -.5, 0]) rotate([0, 0, 45]) scale([2, 0.5, 0.3]) cube(center=true);
	
      translate([0.5, 0.5, 0]) rotate([0,0,-45]) scale([2, 0.5, 0.3]) cube(center=true);
	}

    // TEXT 
    color("white")
	translate([-2.5,-1.5, 1.3])
      scale([0.3,0.3, 0.3])
        linear_extrude(height = 1.0) {
          text($label, font="Liberation Sans:style=Bold Italic");
        }
}}

// side("green", "TP");

translate(v=[0,0,5]) rotate([0,0,0]) side("red", "TP");
translate(v=[0,0,-5]) rotate([0,180,180]) side("green", "BT");
translate(v=[0,-5,0]) rotate([90,0,0]) side("cyan", "FT");
translate(v=[0,5,0]) rotate([90,0,180]) side("blue", "BK");
translate(v=[5,0,0]) rotate([90,0,90]) side("green", "RT");
translate(v=[-5,0,0]) rotate([90,0,-90]) side("green", "LT");
