# mbed_final
#中文解釋
#檔案內包含main.cpp(mbed)、finalproject.py(openMV)、car_contorl.py(Xbee control, optional)

1.mbed部分
(1) 以HW4結合Xbee與image classification為主。
(2) 將每一個subtask以finalproject()來做控制。
(3) Xbee部分讀取資料進來之後判斷為何種指令。以'/'開頭即為RPC function，呼叫RPC指令。以's'開頭即為finalproject指令(start之意)。
(4) 呼叫finalproject()立刻用Xbee回傳'test!'字串表示project開始執行。
(5) 每完成一個substask即回傳'Done(subtask number)'字串表示完成該subtask。
(6) introduce to each subtask below
    以下subtask完成後皆回傳0。
    (a) linefollow
        首先透過uart介面以字串'l'(line)傳送到openMV控制其回傳Line所需參數。參數解碼後開始移動車子。
        當線向左偏即左轉，向右偏即右轉。若現在螢幕左邊即右轉，在螢幕右邊即左轉。如此可以控制車子靠近線來行走。
    (b) goAround
        控制BBCar先用PING偵測正前方物體距離，直到靠近30cm以後開始繞過障礙物(假定障礙物以30cm * 30cm)為例往右座繞行。
    (c) tagApril
        首先透過uart介面以字串't'(tag)傳送到openMV控制其回傳tag所需參數。參數解碼後開始移動車子，並透過PING來協助控制，避免車子過近。
        當tag在螢幕右邊時，即靠左邊走，反之亦然。當距離過近時，向後退，然後對齊tag的方向。即停止。
    (d) park
        以指令呼叫停車，向左後方進行倒車入庫，d1 = 10, d2 = 15。
    (e) imgdetect
        首先透過uart介面以字串'i'(image)傳送到openMV控制其回傳imag classification所需參數，即分析數字，並立即以xbee回傳所看到數字。
        判斷30次自動停止。
 (7) controlBBcar() 用來操作BBCar的綜合指令，包含速度、左輪比例、右輪比例、綜合移動距離。
     速度以正為前進，負為後退。左右輪參數(0.7、0.9)為中央基準。移動距離以encoder來做控制。

2.openMV部分
  以收到之指令完成任務並回傳所需參數。
  (1) line follow: 以find line segment完成，偵測在螢幕下方之直線，並'只回傳一筆資料'。
  (2) tag: 回傳Tx, Ry控制BBcar的重要參數，只回傳一筆資料。
  (3) image classification: 分析螢幕所看到數字，並回傳。

3. car_contorl.py部分 (optional)
  不一定要透過此程式來控制Xbee，也可以直接呼叫/dev/ttyUSB0來控制。主要是方便控制之介面。一樣可以用方向鍵直接控制汽車，'p'倒車入庫，'s'呼叫fianl project。
4. 心得
   控制此車的軟體不難懂，但要精密調教硬體部分的極大誤差，需要花很多心思去調整，很費力，不同地板會有不同的控制速度，不同電池電量會導致BBcar驅動不了。螺絲鬆掉會導致BBcar判斷錯誤。以上都解決之後才有辦法好好控制汽車，因此軟體控制部分就變得相當複雜。
