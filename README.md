# VFX Project1 - HDR Image

## 0. Team Members
* M10815112 鄭鈺哲

## 1. Program Usage
### Build
Use CMakeLists.txt to compile code.
* Environment :
    * Visual Studio 2015/2017 release

### Quick Usage
* Input directory
```
[input_directory]/
├──[image1]
├──...
└──info.txt
```

* Example of info.txt
```
[image_name] 1024
[image_name] 512
[image_name] 256
[image_name] 128
[image_name] 64
[image_name] 32
[image_name] 16
[image_name] 8
[image_name] 4
[image_name] 2
[image_name] 1
[image_name] 0.5
[image_name] 0.25
[image_name] 0.125
[image_name] 0.0625
[image_name] 0.03125
```

* Run
```
HDR_Program.exe [input_directory] [lumin] [depth]
```
* Default Run
```
HDR_Program.exe ./church 0.15 4
```


## 2. Code Work
### 2.1 Image Alignment
* MTB Algorithm
* Use Median Threshold Bitmap algorithm

### 2.2 HDR Reconstruction
* Debevec's method (Uniform sample reference pixel)

### 2.3 Tone Mapping
* Photographic
    * Global Operator

## 3. Data
* Response Curve

| Church | park | ib_stair |
| -------- | -------- | -------- |
| ![](https://i.imgur.com/yoXdflW.png)     |![](https://i.imgur.com/prVU58Y.png)     |![](https://i.imgur.com/lU9tw6I.png)

* Church
![](https://i.imgur.com/Je96OFf.jpg)
* Park
![](https://i.imgur.com/mmEXZ0J.jpg)
* Stair
![](https://i.imgur.com/T2ocLqz.jpg)

## 4. HDR Good Result
### 4.1 Parameter
* Image Alignment
    * depth = 0
* Debevec's Method
    * lambda = 10
* Photographical Tone Mapping
    * a(lumin) = 0.15
    * delta = 10^-6


* Response Curve
![](https://i.imgur.com/1DCtZrl.png)

* ToneMapping Result
![](https://i.imgur.com/DWC64Bd.jpg)
