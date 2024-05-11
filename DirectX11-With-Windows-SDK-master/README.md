# DirectX11 With Windows SDK教程演示项目
[![Build status](https://ci.appveyor.com/api/projects/status/fv2f3emvusqsuj49?svg=true)](https://ci.appveyor.com/project/MKXJun/directx11-with-windows-sdk-hk9xb) ![](https://img.shields.io/badge/license-MIT-dddd00.svg) [![](https://img.shields.io/badge/Ver-1.32.4-519dd9.svg)](https://github.com/MKXJun/DirectX11-With-Windows-SDK/blob/master/MarkdownFiles/Updates/Updates.md)

现代DX11系列教程：使用Windows SDK(C++)开发Direct3D 11.x

## 最近更新

2021/11/9 Ver1.32.4

- 修改cmake，删除VS2015/2017的解决方案与项目，推荐使用其它VS版本的用户使用cmake构建项目
- 调整部分项目着色器名称以适配cmake

## 博客教程

**[博客园](https://www.cnblogs.com/X-Jun/p/9028764.html)**

[CSDN](https://blog.csdn.net/x_jun96/article/details/80293670)

## QQ群交流

QQ群号：727623616

欢迎大家来交流，项目及博客有什么问题也可以在这里提出。

## 如何运行项目

**对于Win10系统，如使用VS2019/2022，可直接打开解决方案；使用VS2015/2017的推荐使用cmake构建项目**

**对于Win7和Win8.x的系统，使用cmake来构建项目，特别的，Win7系统需要勾选`WIN7_SYSTEM_SUPPORT`**

建议一次性生成所有项目，比单独生成会快很多。生成完成后，若要指定运行哪个项目，需要对项目右键-设为启动项。

![](https://github.com/MKXJun/DirectX11-With-Windows-SDK/blob/master/MarkdownFiles/001.png)

如遇到项目无法编译、打开的问题，[请点此处](https://github.com/MKXJun/DirectX11-With-Windows-SDK/blob/master/MarkdownFiles/How-To-Build-Solution/README.md)

## 使用cmake构建自己的VS项目(推荐)

可使用cmake-gui.exe构建项目，唯一需要注意的就是Win 7系统的用户需要勾选`WIN7_SYSTEM_SUPPORT`

> **注意：** 
>
> 1. **教程不支持VS2013及更低版本！**
> 2. **VS2015在安装时需要勾选VS2015 更新 3， 以及Tools(1.4.1)和Windows 10 SDK(10.0.14393)！**
> 3. Win7系统需要安装Service Pack 1以及KB2670838补丁以支持Direct3D 11.1

## 支持/赞赏博主
**博客和项目维护不易，如果本系列教程对您有所帮助，希望能够扫码支持一下博主，谢谢！**

![](https://github.com/MKXJun/DirectX11-With-Windows-SDK/blob/master/MarkdownFiles/002.png)![](https://github.com/MKXJun/DirectX11-With-Windows-SDK/blob/master/MarkdownFiles/003.png)

## 其他

[使用Direct3D 11.x(Windows SDK)编写的魔方](https://github.com/MKXJun/Rubik-Cube)

[完整更新记录](https://github.com/MKXJun/DirectX11-With-Windows-SDK/blob/master/MarkdownFiles/Updates/Updates.md)

