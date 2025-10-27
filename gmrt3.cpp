// Juan Christopher Reinaldo Sipayung 24/544528/TK/60526
#include <Wire.h>
#include <ESP32Servo.h>
#include <MPU6050.h> // Gunakan library MPU6050 dari Adafruit atau Electronic Cats

// --------- PIN SETUP ---------
#define SERVO1_PIN 17
#define SERVO2_PIN 16
#define SERVO3_PIN 27
#define SERVO4_PIN 25
#define SERVO5_PIN 26
#define PIR_PIN 23

// --------- OBJEK SERVO & SENSOR ---------
Servo servo1, servo2, servo3, servo4, servo5;
MPU6050 mpu;

// --------- KONSTAN DAN VARIABEL ---------
const int INITIAL_POS = 90;   // Posisi awal semua servo (tegak lurus)
const float SERVO_GAIN = 0.5; // Kecepatan respons servo terhadap sudut
const float ROLL_THRESHOLD = 5.0;
const float PITCH_THRESHOLD = 5.0;
const float YAW_THRESHOLD = 3.0;

bool motionDetected = false;
bool prevMotionState = false;
unsigned long yawStopTime = 0;
bool yawStopped = false;

void setup()
{
    Serial.begin(115200);
    Wire.begin(21, 22);

    // Inisialisasi sensor MPU6050
    Serial.println("Menghubungkan ke MPU6050...");
    mpu.initialize();
    Serial.println(mpu.testConnection() ? "MPU6050 Tersambung ✅" : "Gagal Tersambung ❌");

    // Inisialisasi semua servo
    servo1.attach(SERVO1_PIN);
    servo2.attach(SERVO2_PIN);
    servo3.attach(SERVO3_PIN);
    servo4.attach(SERVO4_PIN);
    servo5.attach(SERVO5_PIN);
    setAllServos(INITIAL_POS);

    // Inisialisasi sensor PIR
    pinMode(PIR_PIN, INPUT);
    Serial.println("Sistem Siap! 🔧");
    delay(1000);
}

void loop()
{
    // Baca PIR (deteksi gerakan eksternal)
    motionDetected = digitalRead(PIR_PIN);
    if (motionDetected && !prevMotionState)
        handleMotion();
    prevMotionState = motionDetected;

    // Baca data dari sensor MPU6050
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Hitung sudut Roll, Pitch, dan Yaw
    float roll = atan2(ay, az) * 180.0 / PI;
    float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
    float yaw = gz / 131.0; // Gyroscope (perubahan arah)

    // --------- KONTROL ROLL: Servo 1 & 2 MELAWAN ARAH ---------
    if (abs(roll) > ROLL_THRESHOLD)
    {
        int offset = (int)(-roll * SERVO_GAIN);
        moveServo(servo1, INITIAL_POS + offset);
        moveServo(servo2, INITIAL_POS + offset);
    }
    else
        setNeutral(servo1, servo2);

    // --------- KONTROL PITCH: Servo 3 & 4 SEARAH ARAH ---------
    if (abs(pitch) > PITCH_THRESHOLD)
    {
        int offset = (int)(pitch * SERVO_GAIN);
        moveServo(servo3, INITIAL_POS + offset);
        moveServo(servo4, INITIAL_POS + offset);
    }
    else
        setNeutral(servo3, servo4);

    // --------- KONTROL YAW: Servo 5 MENGIKUTI, LALU KEMBALI ---------
    if (abs(yaw) > YAW_THRESHOLD)
    {
        int offset = (int)(yaw * 2.0); // gain lebih besar untuk yaw
        moveServo(servo5, INITIAL_POS + offset);
        yawStopped = false;
    }
    else
    {
        if (!yawStopped)
        {
            yawStopTime = millis();
            yawStopped = true;
        }
        if (millis() - yawStopTime > 1000)
            moveServo(servo5, INITIAL_POS);
    }

    // --------- DEBUG MONITOR ---------
    Serial.print("Roll: ");
    Serial.print(roll);
    Serial.print(" | Pitch: ");
    Serial.print(pitch);
    Serial.print(" | Yaw: ");
    Serial.print(yaw);
    Serial.print(" | PIR: ");
    Serial.println(motionDetected);

    delay(50);
}

// ======================= FUNGSI BANTU =======================

// Semua servo ke posisi awal (tegak lurus)
void setAllServos(int pos)
{
    servo1.write(pos);
    servo2.write(pos);
    servo3.write(pos);
    servo4.write(pos);
    servo5.write(pos);
}

// Kembalikan dua servo ke posisi awal
void setNeutral(Servo &a, Servo &b)
{
    a.write(INITIAL_POS);
    b.write(INITIAL_POS);
}

// Gerakkan servo ke posisi dengan pembatasan
void moveServo(Servo &s, int pos)
{
    s.write(constrain(pos, 0, 180));
}

// Ketika PIR aktif: semua servo bergerak lalu kembali
void handleMotion()
{
    Serial.println("Gerakan terdeteksi! Semua servo bergerak...");
    int target[5] = {45, 135, 60, 120, 30};
    servo1.write(target[0]);
    servo2.write(target[1]);
    servo3.write(target[2]);
    servo4.write(target[3]);
    servo5.write(target[4]);
    delay(500);
    setAllServos(INITIAL_POS);
    delay(500);
    Serial.println("Gerakan selesai, sistem kembali normal.");
}
