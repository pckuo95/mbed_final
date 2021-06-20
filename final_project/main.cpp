#include"mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"
BufferedSerial pc(USBTX,USBRX); //tx,rx
BufferedSerial uart(A1,A0); //tx,rx
BufferedSerial xbee(D1, D0);

// BBCar declare---------
Ticker servo_ticker;               
PwmOut pin5(D5), pin6(D6);              // D5 is faster, D6 is slower
BBCar car(pin5, pin6, servo_ticker);
DigitalInOut pin10(D10);
parallax_ping  ping1(pin10);
// ----------------------
// encode and other I/O delcare
DigitalIn encoder0(D11);
DigitalIn encoder1(D12);
DigitalOut led3(LED3);
DigitalOut led2(LED2);
Ticker encoder_ticker;
// pin5
volatile int stepcounter0 = 0;
volatile int last0 = 0;
// pin6
volatile int stepcounter1 = 0;
volatile int last1 = 0;
volatile int counttimer = 0;
// encoder function
void encoder_read() {
    int value0 = encoder0;
    counttimer++;               // this counter used for loop timeout
    if (!last0 && value0) {
       led3 = !led3;            // this led check encoder is work
       stepcounter0++;
    }
    last0 = value0;
    int value1 = encoder1;
    if (!last1 && value1) {
       led2 = !led2;            // this led check encoder is work
       stepcounter1++;
    }
    last1 = value1;
}
// ----------------------
// global variable declare
char buf[100];
// ----------------------
// project control function declare
void finalproject(void);        // control all subtask
int linefollow(void);           // subtask 1
int tagAlign(void);             // subtask 3
int goAround(void);             // subtask 2
int park(void);                 // subtask 4
int imgdetect(void);            // subtask 5
                                // use to control bbcar precisely
void controlBBCar(double speed, double factorL, double factorR, double target_dist);
// -----------------------
void setupBBcar() {
    // set BBCar
    // please contruct you own calibration table with each servo
    double pwm_table0[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table0[] = {-21.126, -22.322, -21.684, -21.844, -10.357, 0.000, 11.111, 21.924, 21.525, 22.083, 21.445};
    double pwm_table1[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table1[] = {-12.038, -11.719, -11.958, -10.503, -4.222, 0.000, 5.020, 9.706, 10.125, 11.161, 10.683};
    // first and fourth argument : length of table
    car.setCalibTable(11, pwm_table0, speed_table0, 11, pwm_table1, speed_table1);
}

int main(){
    setupBBcar();
    encoder_ticker.attach(&encoder_read, .02);
    uart.set_baud(9600);
    // test interface-------- R representation 'Reset'
    char test[1];
    test[0] = 'R';
    pc.write(test, sizeof(test));
    xbee.write(test, sizeof(test));
    // ------------
    // Xbee------
    char inbuf[128], outbuf[128];
    FILE *devin = fdopen(&xbee, "r");
    FILE *devout = fdopen(&xbee, "w");
    while (1) {
        // clear buffer
        memset(inbuf, 0, 128);
        for(int i = 0; ; i++ ) {
            char recv = fgetc(devin);
            if(recv == '\n') {
                printf("\r\n");
                break;
            }
            inbuf[i] = fputc(recv, devout); // read Xbee info to buffer
        }
        if (inbuf[0] == '/') {
            RPC::call(inbuf, outbuf);
        } else if (inbuf[0] == 's') {   // use command start with 's' to call
            // call final project----
            finalproject();
            // ----
        } else {
            printf("not a command\n");
        }
    }   
}

void finalproject(void) {
    // donecheck
    int done = 0;
    // test Xbee I/O interface
    char printbuf[] = {'t', 'e', 's', 't', '!', '\0'};
    xbee.write(printbuf, sizeof(printbuf));
    done = linefollow();                    // follow the line
    if (!done) {
        printbuf[0] = 'D';
        printbuf[1] = 'o';
        printbuf[2] = 'n';
        printbuf[3] = 'e';
        printbuf[4] = '1';
    } else {
        printbuf[0] = 'e';
        printbuf[1] = 'r';
        printbuf[2] = 'r';
        printbuf[3] = 'o';
        printbuf[4] = 'r';
    }
    xbee.write(printbuf, sizeof(printbuf)); // return finish information
    done = goAround();
    if (!done) {
        printbuf[0] = 'D';
        printbuf[1] = 'o';
        printbuf[2] = 'n';
        printbuf[3] = 'e';
        printbuf[4] = '2';
    } else {
        printbuf[0] = 'e';
        printbuf[1] = 'r';
        printbuf[2] = 'r';
        printbuf[3] = 'o';
        printbuf[4] = 'r';
    }
    xbee.write(printbuf, sizeof(printbuf)); // return finish information
    done = tagAlign();
    if (!done) {
        printbuf[0] = 'D';
        printbuf[1] = 'o';
        printbuf[2] = 'n';
        printbuf[3] = 'e';
        printbuf[4] = '3';
    } else {
        printbuf[0] = 'e';
        printbuf[1] = 'r';
        printbuf[2] = 'r';
        printbuf[3] = 'o';
        printbuf[4] = 'r';
    }
    xbee.write(printbuf, sizeof(printbuf)); // return finish information
    done = park();
    if (!done) {
        printbuf[0] = 'D';
        printbuf[1] = 'o';
        printbuf[2] = 'n';
        printbuf[3] = 'e';
        printbuf[4] = '4';
    } else {
        printbuf[0] = 'e';
        printbuf[1] = 'r';
        printbuf[2] = 'r';
        printbuf[3] = 'o';
        printbuf[4] = 'r';
    }
    xbee.write(printbuf, sizeof(printbuf)); // return finish information
    done = imgdetect();
    if (!done) {
        printbuf[0] = 'D';
        printbuf[1] = 'o';
        printbuf[2] = 'n';
        printbuf[3] = 'e';
        printbuf[4] = '5';
    } else {
        printbuf[0] = 'e';
        printbuf[1] = 'r';
        printbuf[2] = 'r';
        printbuf[3] = 'o';
        printbuf[4] = 'r';
    }
    xbee.write(printbuf, sizeof(printbuf)); // return finish information
}

int linefollow(void) {
    printf("enter line mode\n");
    int enable = 1;
    int count = 0;
    int j = 0;
    
    while(1){
        int i;     
        memset(buf, 0, 100);
        i = 0;
        int readDone = 0;
        char read[1];    
        if (enable && j < 15) {
            read[0] = 'l';
            pc.write(read, sizeof(read));
        } else {
            read[0] = 'n';
        }        
        uart.write(read, sizeof(read));
        j++;
        printf("j = %d\n", j);
        counttimer = 0;
        while (readDone == 0 && i < 100 && enable && counttimer < 50) {
            if(uart.readable()) {
                char recv[1];
                uart.read(recv, sizeof(recv));
                pc.write(recv, sizeof(recv));
                buf[i] = recv[0];
                // pc.write("%c", buf[i]);
                if(recv[0] == '\n') {
                    readDone = 1;
                    j = 0;
                    enable = 0;
                    printf("Read Done! = %d\n", i);
                    break;
                }
                i++;
            }
        }

        if (readDone == 1) {
            count++;
            int x1 = 0, x2 = 0, y1 = 0, y2 = 0, theta = 0;
        
            for(i = 0; buf[i] != ',';i++) {
                x1 = 10 * x1;
                if (buf[i] == 0) break;
                x1 += buf[i] - '0'; 
            }
            i++;    // clear space
            for(; buf[i] != ',';i++) {
                x2 = 10 * x2;
                if (buf[i] == 0) break;
                x2 += buf[i] - '0';  
            }
            i++;    // clear space
            for(; buf[i] != ',';i++) {
                y1 = 10 * y1;
                if (buf[i] == 0) break;
                y1 += buf[i] - '0';  
            }
            i++;    // clear space
            for(;buf[i] != ',';i++) {
                y2 = 10 * y2;
                if (buf[i] == 0) break;
                y2 += buf[i] - '0';  
            }
            i++;    // clear space

            int theta_minus = 0;
            if (buf[i] == '-') {
                theta_minus = 1;
                i++;
            }
            for(;buf[i] != '\n';i++) {
                theta = 10 * theta;
                if (buf[i] == 0) break;
                theta += buf[i] - '0';  
            }
            if (theta_minus) theta = -theta;
            printf("%d,%d,%d,%d,%d\n",x1,x2,y1,y2,theta);
            // control ----------            
            //  '\'
            if (theta > 90) {
                if (x1 < 30 || x2 < 30) {
                    controlBBCar(30, 0.8, 0.7, 10);    // right
                } else {
                    controlBBCar(30, 0.6, 0.9, 10);    // left
                }
                if (theta < 150) {
                    controlBBCar(30, 0, 0.9, 2);
                }             
            } else {
                // '/'
                if (x1 > 130 || x2 > 130) {
                    controlBBCar(30, 0.6, 0.9, 10);   // left
                } else {
                    controlBBCar(30, 0.8, 0.7, 10);     // right
                }
                if (theta > 30) {
                    controlBBCar(30, 0.7, 0, 2);
                }
            }
            if (count > 3) {
                enable = 0;
                return 0;
            } else {
                enable = 1;
            }
        } else {
            car.stop();
            ThisThread::sleep_for(500ms);   
        }
        // control --------
    }
}

int tagAlign(void) {
    printf("enter tag mode\n");
    int enable = 1;
    double current_distance2tag = 0;
    int j = 0;
    
    while(1){
        current_distance2tag = (float)ping1;
        printf("too close current PING=%f\n", current_distance2tag);
        if (current_distance2tag < 36) {
            controlBBCar(-30, 0.7, 0.9, (36 - current_distance2tag));
        }
        if (j == 10) controlBBCar(30, 0.7, 0, 0.8);
        if (j == 13) controlBBCar(30, 0, 0.9, 1.6);
        if (j == 25) controlBBCar(-30, 0.7, 0.9, 20);
        int i;     
        memset(buf, 0, 100);
        i = 0;
        int readDone = 0;
        char read[1];    
        if (enable && j < 30) {
            read[0] = 't';
            pc.write(read, sizeof(read));
        } else {
            read[0] = 'n';
        }        
        uart.write(read, sizeof(read));
        j++;
        printf("j = %d\n", j);
        counttimer = 0;
        while (readDone == 0 && i < 100 && enable && counttimer < 50) {
            if(uart.readable()) {
                char recv[1];
                uart.read(recv, sizeof(recv));
                pc.write(recv, sizeof(recv));
                buf[i] = recv[0];
                // pc.write("%c", buf[i]);
                if(recv[0] == '\n') {
                    readDone = 1;
                    j = 0;
                    enable = 0;
                    //printf("Read Done! = %d\n", i);
                    break;
                }
                i++;
            }
        }

        // decode OpenMV---------------
        if (readDone == 1) {
            i = 0;
            int Tx = 0, Ry = 0;
            int Tx_minus = 0;
            if (buf[0] == '-') {
                Tx_minus = 1;
                i++;
            }
            for(; buf[i] != ',';i++) {
                Tx = 10 * Tx;
                if (buf[i] == 0) break;
                Tx += buf[i] - '0'; 
            }
            if (Tx_minus == 1) Tx = -Tx;

            i++;    // clear space
            for(; buf[i] != '\n';i++) {
                Ry = 10 * Ry;
                if (buf[i] == 0) break;
                Ry += buf[i] - '0';  
            }       
            if (360 > Ry && Ry > 180) {
                Ry = Ry - 360;
            }
            

            current_distance2tag = (float)ping1;
            printf("current PING=%f\n", current_distance2tag);
            enable = 1;


            if (current_distance2tag > 40) {
                if (Tx > 300) {
                    controlBBCar(30, 0, 0.9, 1);
                } else if (Tx > 50) {
                    controlBBCar(30, 0.6, 0.8, 5);
                } else if (Tx > 0) {
                    controlBBCar(30, 0.7, 0.95, 10);
                } else if (Tx > -50) {
                    controlBBCar(30, 0.7, 0.8, 10);
                } else if (Tx > -300) {
                    controlBBCar(30, 0.7, 0.7, 5);
                } else {
                    controlBBCar(30, 0.7, 0, 1);
                }
            }
            current_distance2tag = (float)ping1;
            printf("current PING=%f\n", current_distance2tag);
            if (current_distance2tag < 40) {
                if (Ry > 5) {
                    controlBBCar(30, 0.7, 0, 0.5);
                } else if (Ry < -5) {
                    controlBBCar(30, 0, 0.9, 0.5);
                } else {
                    controlBBCar(-30, 0.7, 0.9, 39 - current_distance2tag);
                    enable = 0;
                    printf("final PING=%f\n",(float)ping1);
                    return 0;
                }
            }
        }
    }
}

int goAround(void) {
    printf("enter around mode\n");
    double distance = (float)ping1;
    printf("distance = %f\n", distance);
    // go straight to cartain distance
    while (distance > 30) {
        car.gofollowline(30, 0.7, 0.9);
        ThisThread::sleep_for(100ms);
        distance = (float)ping1;
    }
    car.stop();
    ThisThread::sleep_for(1s);
    // Around-----------------
    controlBBCar(30, 0.7, 0, 16.5+0.5);    // trun right
    controlBBCar(30, 0.7, 0.9, 14);        // go straight
    controlBBCar(30, 0, 0.9, 16.5 - 4);    // trun left
    controlBBCar(30, 0.7, 0.9, 35);        // go straight
    controlBBCar(30, 0, 0.9, 16.5 - 4);    // trun left
    controlBBCar(30, 0.7, 0.9, 12);        // go straight
    controlBBCar(30, 0.7, 0, 7);           // trun right
    return 0;
}
void controlBBCar(double speed, double factorL, double factorR, double target_dist) {
    double current_distance0 = 0;
    double current_distance1 = 0;
    // initialize encoder counter
    stepcounter0 = 0;
    stepcounter1 = 0;
    car.gofollowline(speed, factorL, factorR);
    if (factorR == 0) {
        printf("turnRight,%f\n", target_dist);
    } else if (factorL == 0) {
        printf("turnLeft,%f\n", target_dist);
    } else {
        printf("go ,%f\n", target_dist);
    }
    while(current_distance0 < target_dist &&
            current_distance1 < target_dist) {
        current_distance0 = stepcounter0 * 6.5 * 3.1415 / 32;
        current_distance1 = stepcounter1 * 6.5 * 3.1415 / 32;
        ThisThread::sleep_for(100ms);
    }
    car.stop();
    ThisThread::sleep_for(1s);
}

int park(void) {
    printf("enter park mode\n");
    controlBBCar(-30, 0.7, 0.9, 10);
    controlBBCar(-30, 0, 0.9, 16.5 - 1.5);
    controlBBCar(-30, 0.7, 0.9, 18);
    return 0;
}

int imgdetect(void) {
    printf("enter img mode\n");
    int enable = 1;
    int j = 0;
    int counttimer = 0;
    int count = 0;
    while(1){
        int i;     
        memset(buf, 0, 100);
        i = 0;
        int readDone = 0;
        char read[1];    
        if (enable && j < 50) {
            read[0] = 'i';
            pc.write(read, sizeof(read));
        } else {
            read[0] = 'n';
            return 0;
        }        
        uart.write(read, sizeof(read));
        j++;
        printf("j = %d\n", j);
        counttimer = 0;

        while (readDone == 0 && i < 100 && enable && counttimer < 50) {
            if(uart.readable()) {
                char recv[1];
                uart.read(recv, sizeof(recv));
                pc.write(recv, sizeof(recv));
                xbee.write(recv, sizeof(recv));
                buf[i] = recv[0];
                if(recv[0] == '\n') {
                    readDone = 1;
                    j = 0;
                    enable = 0;
                    //printf("Read Done! = %d\n", i);
                    break;
                }
                i++;
            }
        }
        // decode OpenMV---------------
        if (readDone == 1) {
            i = 0;
            int value = 0;
            for(; buf[i] != '\n';i++) {
                value = 10 * value;
                if (buf[i] == 0) break;
                value += buf[i] - '0'; 
            }
            printf("img number =%d\n", value);
            count++;
            enable = 1;
            if (count > 20) enable = 0;
            ThisThread::sleep_for(500ms);
        }
    }
}