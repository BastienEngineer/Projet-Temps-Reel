#include <time.h>
#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>
#include <wiringPiI2C.h>
#include "pca9685_3.h"

#define LINE_PIN_RIGHT 19
#define LINE_PIN_MIDDLE 16
#define LINE_PIN_LEFT 20
#define Motor_B_EN 17
#define Motor_B_Pin1 27
#define Motor_B_Pin2 18

struct timespec diff(struct timespec start, struct timespec end);
uint16_t fd = 0;
int choix = 0;
int dir;

uint8_t setUpDevice(uint16_t fd)
{
    setAllPWM(fd,0,0);
    wiringPiI2CWriteReg8(fd,MODE2,OUTDRV);
    wiringPiI2CWriteReg8(fd,MODE1,ALLCALL);
    delay(0.005);
    uint8_t mode1=wiringPiI2CReadReg8(fd,MODE1);
    mode1=mode1 & SLEEP;
    wiringPiI2CWriteReg8(fd,MODE1,mode1);
    delay(0.005);
    setPWMFreq(fd,50);
    return 0;
}

uint8_t setPWMFreq(uint16_t fd, uint16_t freq)
{
    float prescaleval = CLOCKFREQ;
    prescaleval = prescaleval/4096.0;
    prescaleval = prescaleval/freq;
    prescaleval = prescaleval - 1.0;
    float prescale = (prescaleval + 0.5);
    uint8_t oldmode = wiringPiI2CReadReg8(fd,MODE1);
    uint8_t newmode = (oldmode & 0x7F) | 0x10;
    wiringPiI2CWriteReg8(fd,MODE1, newmode);
    wiringPiI2CWriteReg8(fd,PRESCALE, (int)prescale);
    wiringPiI2CWriteReg8(fd,MODE1, oldmode);
    delay(0.005);
    wiringPiI2CWriteReg8(fd,MODE1, oldmode | 0x80);
    return 0;
}

uint8_t setPWM(uint16_t fd, uint8_t channel, uint16_t on, uint16_t off)
{
    wiringPiI2CWriteReg8(fd, LED0_ON_L+4*channel, on & 0xFF);
    wiringPiI2CWriteReg8(fd, LED0_ON_H+4*channel, on >> 8);
    wiringPiI2CWriteReg8(fd, LED0_OFF_L+4*channel, off & 0xFF);
    wiringPiI2CWriteReg8(fd, LED0_OFF_H+4*channel, off >> 8);
    return 0;
}

uint8_t setAllPWM(uint16_t fd, uint16_t on, uint16_t off)
{
    wiringPiI2CWriteReg8(fd, ALL_LED_ON_L, on & 0xFF);
    wiringPiI2CWriteReg8(fd, ALL_LED_ON_H, on >> 8);
    wiringPiI2CWriteReg8(fd, ALL_LED_OFF_L, off & 0xFF);
    wiringPiI2CWriteReg8(fd, ALL_LED_OFF_H, off >> 8);
    return 0;
}

uint8_t sleepOn(uint16_t fd)
{
    uint8_t oldmode = wiringPiI2CReadReg8(fd,MODE1);
    uint8_t newmode = (oldmode & 0x7F) | 0x10;
    wiringPiI2CWriteReg8(fd,MODE1, newmode);
    return 0;
}

void motorStop(void)
{
    digitalWrite(Motor_B_Pin1, LOW);
    digitalWrite(Motor_B_Pin2, LOW);
    digitalWrite(Motor_B_EN, LOW);
}

void setup(void)
{
    wiringPiSetupGpio();
    pinMode(LINE_PIN_RIGHT, INPUT);
    pinMode(LINE_PIN_MIDDLE, INPUT);
    pinMode(LINE_PIN_LEFT, INPUT);
    pinMode(Motor_B_EN, OUTPUT);
    pinMode(Motor_B_Pin1, OUTPUT);
    pinMode(Motor_B_Pin2, OUTPUT);
    motorStop(); 
    fd = wiringPiI2CSetup(PCA9685_ADDR);
    setUpDevice(fd);
}

void motor_B(int direction, int speed)
{
    if (direction == 1)
    {
        digitalWrite(Motor_B_Pin1, HIGH);
        digitalWrite(Motor_B_Pin2, LOW);
    }
    softPwmCreate(Motor_B_EN, 0, 100);
    softPwmWrite(Motor_B_EN, speed);
    if (direction == -1)
    {
        digitalWrite(Motor_B_Pin1, LOW);
        digitalWrite(Motor_B_Pin2, HIGH);
    }
    softPwmCreate(Motor_B_EN, 0, 100);
    softPwmWrite(Motor_B_EN, speed);
}

void run1()
{
    switch(choix)
    {
        case 1:
        {
            motor_B(1,100);
            setPWM(fd, 0, 150, 375);
            break;
        }
        case 2:
        {
            motor_B(1,100);
            setPWM(fd, 0, 300, 600);
            break;
        }
        case 3:
        {
            motor_B(1,100);
            setPWM(fd, 0, 0, 150);
            break;
        }
        case 4:
        {
            //motorStop();
            //sleepOn(fd);
            motor_B(-1,100);
            if(dir==0)
            {
                setPWM(fd, 0, 0, 150);
            }
            else
            {
                setPWM(fd, 0, 300, 600);
            }
            break;
        }     
        default:
        {
            motorStop();
            sleepOn(fd);
            break;
        }
    }
}

void run()
{
    int status_right = digitalRead(LINE_PIN_RIGHT);
    int status_middle = digitalRead(LINE_PIN_MIDDLE);
    int status_left = digitalRead(LINE_PIN_LEFT);

    printf("%d %d %d\n", status_left,status_middle,status_right);

    if (status_middle == 1 && status_right == 0 && status_left == 0)
    {
        /*
        Control the robot forward
        */
        printf("forward\n");
        choix=1;
    }
    if (status_left == 1 && status_middle==0 && status_right==0) 
    {
        /*
        Control the robot to turn left
        */
        printf("right\n");
        choix=3;
        dir=1;
    }
    if (status_right == 1 && status_left==0 && status_middle==0)
    {
        /*
        Control the robot to turn right
        */
        printf("left\n");
        choix=2;
        dir=0;
    }
    if(status_middle==0 && status_left==0 && status_right==0)
    {
        //If the line is not detected, the robot stops
        printf("stop\n");
        choix=4;
    }
}

void job1(long fet)
{
    struct timespec start, end;
    int exit=0;
    setup();
    run1();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&start);
    while(!exit)
    {
        clock_gettime(CLOCK_THREAD_CPUTIME_ID,&end);
        if (diff(start,end).tv_nsec>=fet)
        {
            exit=1;
        }
        //  printf("%ld / % ld", diff(start,end).tv_nsec,fet);
    }
}

void job(long fet) // fet in nanoseconds
{
    struct timespec start, end;
    int exit=0;
    wiringPiSetupGpio(); // Initialize wiringPi library
    pinMode(LINE_PIN_RIGHT, INPUT);
    pinMode(LINE_PIN_MIDDLE, INPUT);
    pinMode(LINE_PIN_LEFT, INPUT);
    run();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID,&start);
    while(!exit)
    {
        clock_gettime(CLOCK_THREAD_CPUTIME_ID,&end);
        if (diff(start,end).tv_nsec>=fet)
        {
            exit=1;
        }
        //  printf("%ld / % ld", diff(start,end).tv_nsec,fet);
    }
}

struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) 
    {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } 
    else 
    {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}