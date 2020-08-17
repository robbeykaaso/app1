# Abstract
an IO window component based on deepsight sdk  

# Attribute
* name: the default is "io1", it should be noticed that different io instance should have different name  
_sample_:  
```
    IO{
        name: "io2"
    }
```  
</br>

* param: the parameter for io  
_sample_:    
```
    property var param:{
                        "config": "",
                        "id": "USB_7230,0",
                        "in_descriptor": {
                                             "channel_in0": 1,
                                             "channel_in1": 2,
                                             "channel_in10": 1024,
                                             "channel_in11": 2048,
                                             "channel_in12": 4096,
                                             "channel_in13": 8192,
                                             "channel_in14": 16384,
                                             "channel_in15": 32768,
                                             "channel_in2": 4,
                                             "channel_in3": 8,
                                             "channel_in4": 16,
                                             "channel_in5": 32,
                                             "channel_in6": 64,
                                             "channel_in7": 128,
                                             "channel_in8": 256,
                                             "channel_in9": 512},
                        "name": "Digital IO controller",
                        "out_descriptor": {
                                              "channel_out0": 1,
                                              "channel_out1": 2,
                                              "channel_out10": 1024,
                                              "channel_out11": 2048,
                                              "channel_out12": 4096,
                                              "channel_out13": 8192,
                                              "channel_out14": 16384,
                                              "channel_out15": 32768,
                                              "channel_out2": 4,
                                              "channel_out3": 8,
                                              "channel_out4": 16,
                                              "channel_out5": 32,
                                              "channel_out6": 64,
                                              "channel_out7": 128,
                                              "channel_out8": 256,
                                              "channel_out9": 512},
                        "type": "io_adlink_usb"
                    }
```  
</br>

# API Pipe
* **name + "_sendIO"**  
trig the io to send message once. it is a pipeDelegate of "_IOReceived"  
</br>

* **name + "_turnIO"**  
turn on/off IO. the parameter "on" is boolean  
</br>

* **name + "IOStated"**  
output a string state of the io  
</br>

* **name + "_IOReceived"**  
output string received from the io  
</br>

# Test and Demo
* main.qml: io  
* io.cpp  
</br>