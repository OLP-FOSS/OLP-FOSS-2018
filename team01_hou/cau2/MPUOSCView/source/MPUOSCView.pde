// oscP5
import oscP5.*;
import netP5.*;
// ToxicLibs
import toxi.geom.*;
import toxi.processing.*;

ToxiclibsSupport gfx;

Quaternion quat = new Quaternion(1, 0, 0, 0);

OscP5 oscP5;

PFont font;

float six[];

String activity_labels[] = new String[] {"NoLabel", "Walking", "Jogging", "Cycling", "Stairs", "Sitting", "Standing"};

int activity = 0;

void setup() {
    
    six = new float[6];
    
    // square viewport for OpenGL rendering
    size(640, 640, P3D);
    gfx = new ToxiclibsSupport(this);

    // lights and antialiasing
    lights();
    smooth();

    /* listen for incoming data at port 8081 */
    oscP5 = new OscP5(this, 8081);

    oscP5.plug(this, "imu", "/imu");

    font = createFont("Arial", 16, true);
}

void oscEvent(OscMessage oscm) {
  //print("osc message: ");
  //print("addrpattern = " + oscm.addrPattern());
  //println(", typetag = " + oscm.typetag());
  }

public void imu(int xactivity, float quant_w, float quant_x, float quant_y, float quant_z, float ax, float ay, float az, float yaw, float pitch, float roll) {
  //println(quant_w, quant_x, quant_y, quant_z);
  activity = xactivity;
  quat.set(quant_w, quant_x, quant_y, quant_z);
  six[0] = ax;
  six[1] = ay;
  six[2] = az;
  six[3] = yaw;
  six[4] = pitch;
  six[5] = roll;
}

void draw() {
    // draw background (custom)
    background(0);
    
    textFont(font, 16);
    fill(255, 0, 0);
    text("QUAT = " + (Math.ceil(quat.w*1000)/1000) + " " + (Math.ceil(quat.x*1000)/1000) + " " + (Math.ceil(quat.y*1000)/1000), 15, 15);
    fill(255, 255, 255);
    text("ACCEL = " + (Math.ceil(six[0]*1000)/1000) + " " + (Math.ceil(six[1]*1000)/1000) + " " + (Math.ceil(six[2]*1000)/1000), 15, 35);
    fill(0, 0, 255);
    text("YPR = " + (Math.ceil(six[3]*1000)/1000) + " " + (Math.ceil(six[4]*1000)/1000) + " " + (Math.ceil(six[5]*1000)/1000), 15, 60);
    fill(0, 255, 0);
    text("ACT = " + activity_labels[activity], 60, 90);

    // translate things to the middle of the viewport
    pushMatrix();
    translate(width / 2, height / 2);

    // 3-step rotation from yaw/pitch/roll angles (gimbal lock!) and other weirdness I haven't figured out yet
    //rotateY(-ypr[0]);
    //rotateZ(-ypr[1]);
    //rotateX(-ypr[2]);

    // toxiclibs direct angle/axis rotation from quaternion (NO gimbal lock!) (axis order [1, 3, 2] and inversion [-1, +1, +1] is a consequence of different coordinate system orientation assumptions between Processing and MPU DMP)
    float[] axis = quat.toAxisAngle();
    rotate(axis[0], -axis[1], axis[3], axis[2]);

    // main body (red)
    fill(255, 0, 0, 200);
    box(10, 10, 200);

    // front-facing tip (blue)
    fill(0, 0, 255, 200);
    pushMatrix();
    translate(0, 0, -120);
    rotateX(PI/2);
    drawCylinder(0, 20, 20, 8);
    popMatrix();

    // wings
    fill(222, 240, 0, 200);

    beginShape(TRIANGLES);
    vertex(-100,  2, 30); vertex(0,  2, -80); vertex(100,  2, 30);  // wing top layer
    vertex(-100, -2, 30); vertex(0, -2, -80); vertex(100, -2, 30);  // wing bottom layer
//  vertex(-2, 0, 98); vertex(-2, -30, 98); vertex(-2, 0, 70);  // tail left layer    vertex( 2, 0, 98); vertex( 2, -30, 98); vertex( 2, 0, 70);  // tail right layer
    endShape();

    // tail fin
/*
    beginShape(QUADS);
    vertex(-100, 2, 30); vertex(-100, -2, 30); vertex(  0, -2, -80); vertex(  0, 2, -80);
    vertex( 100, 2, 30); vertex( 100, -2, 30); vertex(  0, -2, -80); vertex(  0, 2, -80);
    vertex(-100, 2, 30); vertex(-100, -2, 30); vertex(100, -2,  30); vertex(100, 2,  30);
    vertex(-2,   0, 98); vertex(2,   0, 98); vertex(2, -30, 98); vertex(-2, -30, 98);
    vertex(-2,   0, 98); vertex(2,   0, 98); vertex(2,   0, 70); vertex(-2,   0, 70);
    vertex(-2, -30, 98); vertex(2, -30, 98); vertex(2,   0, 70); vertex(-2,   0, 70);
    endShape();
*/

    popMatrix();
}

void drawCylinder(float topRadius, float bottomRadius, float tall, int sides) {
    return;
/*
    float angle = 0;
    float angleIncrement = TWO_PI / sides;
    beginShape(QUAD_STRIP);
    for (int i = 0; i < sides + 1; ++i) {
        vertex(topRadius*cos(angle), 0, topRadius*sin(angle));
        vertex(bottomRadius*cos(angle), tall, bottomRadius*sin(angle));
        angle += angleIncrement;
    }
    endShape();

    // If it is not a cone, draw the circular top cap
    if (topRadius != 0) {
        angle = 0;
        beginShape(TRIANGLE_FAN);

        // Center point
        vertex(0, 0, 0);
        for (int i = 0; i < sides + 1; i++) {
            vertex(topRadius * cos(angle), 0, topRadius * sin(angle));
            angle += angleIncrement;
        }
        endShape();
    }

    // If it is not a cone, draw the circular bottom cap
    if (bottomRadius != 0) {
        angle = 0;
        beginShape(TRIANGLE_FAN);

        // Center point
        vertex(0, tall, 0);
        for (int i = 0; i < sides + 1; i++) {
            vertex(bottomRadius * cos(angle), tall, bottomRadius * sin(angle));
            angle += angleIncrement;
        }
        endShape();
    }
*/
}
