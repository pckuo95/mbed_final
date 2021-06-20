import pyb, sensor, image, time, math, os, tf
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)

net = "trained.tflite"
labels = [line.rstrip('\n') for line in open("labels.txt")]
clock = time.clock()
f_x = 112.45
f_y = 84.34
c_x = 80
c_y = 60
def degrees(radians):
   return (180 * radians) / math.pi
uart = pyb.UART(3,9600,timeout_char=1000)
uart.init(9600,bits=8,parity = None, stop=1, timeout_char=1000)
while(True):
    clock.tick()
    img = sensor.snapshot()
    i = 0;
    c = uart.readchar()
    if (c == 108):
        for l in img.find_line_segments(merge_distance = 1, max_theta_diff = 5):
            if ((l.magnitude() > 5) & (l.y1() < 40) & (l.y2() < 40) & (i == 0)):
                i = 1
                img.draw_line(l.line(), color = (255, 0, 0))
                print_args = (l.x1(), l.x2(), l.y1(), l.y2(), l.theta())
                uart.write(("%d,%d,%d,%d,%d\n" % print_args).encode())
                print("%d,%d,%d,%d,%d\n" % print_args)
    elif (c == 116):
        for tag in img.find_apriltags(fx=f_x, fy=f_y, cx=c_x, cy=c_y):
            sendData = (tag.x_translation()*100, degrees(tag.y_rotation()))
            uart.write(("%d,%d,%d\n" % sendData).encode())
            print("%d,%d,%d\n" % sendData)
    elif (c == 105):
        for obj in tf.classify(net, img, min_scale=1.0, scale_mul=0.5, x_overlap=0.0, y_overlap=0.0):
            img.draw_rectangle(obj.rect())
            img.draw_string(obj.x()+3, obj.y()-1, labels[obj.output().index(max(obj.output()))], mono_space = False)
            print("This is : ",labels[obj.output().index(max(obj.output()))])
            uart.write("%s\n" % labels[obj.output().index(max(obj.output()))])
