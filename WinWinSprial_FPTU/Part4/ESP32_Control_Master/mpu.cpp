#include "mpu.h"

#include "MPU6050_6Axis_MotionApps20.h"

MPU6050 mpu(0x69); // <-- use for AD0 high

int16_t ax, ay, az;
int16_t gx, gy, gz;

//MPU control/status vars 
bool dmpReady = false; 
uint8_t mpuIntStatus;  //actual interrupt status byte 
uint8_t deviceStatus; //device status , 0 = success , 
uint16_t packetSize; //expected DMP packet size (defult 42) -- ?
uint16_t fifoCount; //count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO buffer storage 

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector





// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}


void setup_mpu(){
    //join I2C bus 
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
//        TWBR = 48; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
  //initialize serial communication

  while(!Serial); //common for Leonardo issues

  //initialize the mpu 
  Serial.println("Call MPU6050 Lib to initialize devices...");
  mpu.initialize(); //initialize I2C device by using MPU6050 library 

  //verify connection
  Serial.println("Tesing device connections");
  Serial.println(mpu.testConnection() ? F("MPU6050 connection test successed ") : F("MPU6050 connection test failed"));

//  //wait for begin , uncomment if we need wait user interruption
//  Serial.println("Press any button to begin");
//  while (Serial.available() && Serial.read()); // empty buffer
//  while (!Serial.available());                 // wait for data
//  while (Serial.available() && Serial.read()); // empty buffer again


  //load and configure DMP 
  Serial.println("initializing DMP"); 
  deviceStatus = mpu.dmpInitialize(); //use MPU6050 library to inilisalize the dmp 

 //feed offsets 
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

  //make sure it works 
  if (deviceStatus == 0) {
    Serial.println("DMP initialization success, now enable DMP for use");
    //turn on DMP 
    mpu.setDMPEnabled(true); //use MPU6050 library to enable DMP)

    //wait for first interrup . currently just leave it false automatically 
    mpuInterrupt == false; 

    //let main llop know it's ok to use DMP, set dmpRead flag to ture 
    dmpReady = true;
    Serial.println("DMP is ready to use.");

    //get expected DMP packet size for later comparison 
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    //ERROR! , device status !=0 when initializing DMP
    Serial.print("DMP initialization failed when using MPU6050 library:");
    if (deviceStatus == 1) {
      Serial.println(" intial memory load failed");
    } else if (deviceStatus == 2) {
      Serial.println(" failed to update DMP configuration");
    } else {
      Serial.print(" unknow error with code: ");
      Serial.println(deviceStatus);
    }
}
}


void display(){
    // read raw accel/gyro measurements from device
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    Serial.print("a/g:\t");
    Serial.print(ax); Serial.print("\t");
    Serial.print(ay); Serial.print("\t");
    Serial.print(az); Serial.print("\t");
    Serial.print(gx); Serial.print("\t");
    Serial.print(gy); Serial.print("\t");
    Serial.println(gz);
}

String get_acelgyro_json(){
    // read raw accel/gyro measurements from device
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    return "{\"ax\":"+String(ax)+", \"ay\":"+String(ay)+",\"az\":"+az+",\"gx\":"+gx+",\"gy\":"+gy+",\"gz\":"+gz+"}";
}

int ticket = 1;
void printOnlyOnce (String message){
  if (ticket == 1){
   Serial.println(message);
   ticket = 0 ;
  } else {
    return;
  }
}

String get_acel_ypr_json(){
    //if DMP not ready don't do anything 
  if (!dmpReady) {
    printOnlyOnce("MAIN LOOP: DMP disabled");
    return "";
  } else {

    //testing overflow 
    if (fifoCount == 1024) {
      mpu.resetFIFO();
      Serial.println("FIFO overflow");
    } else {


    //wait for enough avaliable data length
    while (fifoCount < packetSize) {
      //waiting until get enough
      fifoCount = mpu.getFIFOCount();
    }


    //read this packet from FIFO buffer 
    mpu.getFIFOBytes(fifoBuffer,packetSize);
    mpu.resetFIFO();

    //track FIFO count here is more then one packeage avalible 

    //reset fifo count 
    fifoCount -= packetSize ;
    //Serial.println(fifoCount);

    if (fifoCount > 2) {
        ////// clear fifo buffer 
    }
    //display stage 
    // display Euler angles in degrees
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    Serial.print("ypr\t");
    Serial.print(ypr[0] * 180/M_PI);
    Serial.print("\t");
    Serial.print(ypr[1] * 180/M_PI);
    Serial.print("\t");
    Serial.println(ypr[2] * 180/M_PI);

    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);

    ax = aaReal.x; ay = aaReal.y;  az = aaReal.z;
    Serial.print("ypr\t");
    Serial.print(ypr[0] * 180/M_PI);
    Serial.print("\t");
    Serial.print(ypr[1] * 180/M_PI);
    Serial.print("\t");
    Serial.println(ypr[2] * 180/M_PI);
  }
  }
    //mpu.getAcceleration(&ax, &ay, &az);
     return "{\"ax\":"+String(ax)+", \"ay\":"+String(ay)+",\"az\":"+az+",\"y\":"+ypr[0]+",\"p\":"+ypr[1]+",\"r\":"+ypr[2]+"}";
}
