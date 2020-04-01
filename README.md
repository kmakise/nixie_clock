# nixie_clock
使用辉光管作为显示器件的时钟 演示视频(B站)：https://www.bilibili.com/video/BV1cJ411u7Mh
这个时钟是直接先变动率探测仪的原理验证版本
## 说明
主控：STM32F103C8T6 驱动：74HC595+13001 电源：555 + irfp450n 辉光管：qs18-12
驱动方式和世界线变动率探测仪一致，详细定义见程序注释。
![1](https://github.com/kerisu/nixie_clock/blob/master/photo/1.jpg)
![2](https://github.com/kerisu/nixie_clock/blob/master/photo/2.jpg)
![3](https://github.com/kerisu/nixie_clock/blob/master/photo/3.jpg)
