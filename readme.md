# DIY electronic work card
English | [简体中文](#DIY电子工牌)
## Development environment setup
1. [Build the Arduino development environment.](https://heltec-automation.readthedocs.io/zh_CN/latest/general/how_to_install_git_and_arduino.html)   
2. [Install the development framework.](https://heltec-automation.readthedocs.io/zh_CN/latest/esp32/quick_start.html#git)   

## use demo
### Basic use of the development board
1. Open the **LoRaWan_CLASS_A_wakeOverTheAir.ino** example and configure it as follows.Select the **Tools** option, and enter the following configuration: (Note: Only the key options are listed below, other options can be configured as needed)
    | development board | lora_eink_board  |
    |------- |-----------------------|   
    |LoraWAN Region | REGION_CN470|
    |LoRaWan Debug  | none         |
    |LORAWAN_DEVEUI | CUSTOM     |
   
2. After connecting to the serial port, click Download to download the program to be run.
3.If you are prompted that a license is required, open the corresponding [URL](http://resource.heltec.cn/search), inquire and enter.
4. After normal operation, the serial port will print **Within 10 seconds, send any data to the serial port and enter the configuration mode.** Send any data to the serial port within 10S to enter the configuration mode. If it is not sent, it will enter normal operation after 10S. (The program will only wait 10S for the first run after reset, and will not wait again later.)
5. After entering the configuration mode, you can configure the following three parameters according to the following format.
>- dev_eui=2232330000888801
>- app_key=88888888888888888888888888886601
>- qrcode_info={"DeviceName":"e_link","ProductId":"WR00ATPGRU","Signature":"a83aaa9708a54790ae020dfa809ca998"}
6. After the configuration is complete, reset can be used normally.
### Use Tencent Lianlian for remote control
1. See [LoRaWAN Development Document](https://cloud.tencent.com/document/product/1081/52426) to add gateway and node information on Tencent Cloud Server.
2. Product development -> product name -> import object model, import the object model json file (decode/decode.json) into the object model.
3. Product Development -> Product Name -> Device Development -> Cloud Analysis -> Downlink Data Analysis, Copy the content of the data analysis file (data_decode/decode.js) to the downlink data analysis column.
4. Product development->product name->interactive development->configure applet->panel configuration, select H5 custom panel, and put the JS file (data_decode/SummitInfo_panel-default.c1a671ab6c.js) and CSS file (data_decode/1_SummitInfo_panel-default) .8a85310b27.css) into the corresponding location.
5. Open the Tencent Lianlian applet in WeChat, scan the QR code on the Tencent Cloud website (product development -> product name -> device debugging -> QR code) to enter the Tencent Lianlian applet delivery interface (as shown in the figure: img/ Tencent Lianlian applet interface.jpg). After connecting lora to the network, it can be distributed and displayed on the ink screen.        
Note: You can also parse the QR code above, and then configure it through the serial port and display it on the ink screen. Then use the Tencent Lianlian applet to scan.

# DIY电子工牌
## 开发环境搭建
1. [搭建Arduino开发环境。](https://heltec-automation.readthedocs.io/zh_CN/latest/general/how_to_install_git_and_arduino.html)   
2. [安装开发框架。](https://heltec-automation.readthedocs.io/zh_CN/latest/esp32/quick_start.html#git)   

## 使用演示
### 开发板基本使用
1. 打开 **LoRaWan_CLASS_A_wakeOverTheAir.ino** 示例，并按照如下步骤进行配置。
 选择**工具**选项,并进入如下配置：(注：下面只列出了关键选项，其它选项自行根据需要配置)
    | 开发板  | lora_eink_board  |
    |------- |-----------------------|   
    |LoraWAN Region | REGION_CN470|
    |LoRaWan Debug  | 无          |
    |LORAWAN_DEVEUI | CUSTOM     |
   
2. 连接上串口之后，点击下载，把需要运行的程序下载进去。
3. 如果提示需要license，打开对应[网址](http://resource.heltec.cn/search),查询并输入即可。
4. 正常运行之后，串口会打印 **Within 10 seconds, send any data to the serial port and enter the configuration mode.** 在10S内向串口发送任何数据，都可以进入配置模式。如果不发送，等10S之后，就会进入正常运行。（程序只有在复位之后的第一次运行才会等待10S，后面不会再次等待。）
5. 进入配置模式之后，可以按照一下格式，对下面三项参数进行配置。
>- dev_eui=2232330000888801
>- app_key=88888888888888888888888888886601
>- qrcode_info={"DeviceName":"e_link","ProductId":"WR00ATPGRU","Signature":"a83aaa9708a54790ae020dfa809ca998"}
6. 配置完成之后，复位即可正常进行使用。
### 使用腾讯连连进行远程控制
1. 参看 [LoRaWAN开发文档](https://cloud.tencent.com/document/product/1081/52426) 在腾讯云服务器上面添加网关和节点信息。
2. 产品开发->产品名称->导入物模型，将物模型json文件(decode/decode.json)，导入物模型。
3. 产品开发->产品名称->设备开发->云端解析->下行数据解析， 将数据解析文件(data_decode/decode.js)内容，复制到下行数据解析栏。
4. 产品开发->产品名称->交互开发->配置小程序->面板配置，选择H5自定义面板，并把JS文件(data_decode/SummitInfo_panel-default.c1a671ab6c.js)和CSS文件(data_decode/1_SummitInfo_panel-default.8a85310b27.css)导入到相对应的位置。
5. 在微信里面打开腾讯连连小程序，扫描腾讯云网页上面的二维码(产品开发->产品名称->设备调试->二维码)进入腾讯连连小程序的下发界面(如图：img/腾讯连连小程序界面.jpg)。在连接lora 入网之后，就可以进行下发，并显示到墨水屏上面了。   
注：也可以把上面的二维码解析出来，然后通过串口配置，显示到墨水屏上面。然后再使用腾讯连连小程序进行扫描。