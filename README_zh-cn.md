# uikit

[简体中文 | [English](./README.md)]

## 简介

uikit 是一个用于开发图形界面的工具集，它提供了一些常用的 UI 组件和工具，以简化开发过程。

uikit 提供了以下工具：
- 字体管理器：一款功能全面的字体管理工具。
    - 支持多项功能，如加载字体、创建字体和删除字体等。
    - 支持字体缓存管理，避免重复加载字体和频繁 IO 读取，从而实现高效的字体渲染。
    - 扩展 emoji 字体管理功能，支持 png、bin 等格式的图片字体。

- 二维码扫描：基于 quirc 库实现的二维码扫描器。 使用前需要将图片转换为灰度图, 提供 YUYV、NV12、RGBA8888、RGB565、RGB332 等格式图片转换到灰度图的接口。扫描结果将以字符串形式返回。

- 视频控件：在 openvela 现有的 media 框架基础上，通过对 LVGL image 控件进行扩展，增强了 LVGL 对 video buffer 的相关控制、获取和渲染能力。

- 异步刷新：支持在下次刷新前或刷新后，执行一些任务。你需要先初始化异步刷新模块，然后注册一些异步回调函数，这些回调函数会在下次刷新前或刷新后执行。

## 目录

```
├── include
│   └── uikit
├── src
│   ├── draw
│   ├── font_manager
│   ├── qrscan
│   ├── uikit_async.c
│   ├── uikit.c
│   ├── uikit_font_manager.c
│   ├── uikit_internal.h
│   └── video
└── test
```

## 使用

使用uikit，只需在代码中引入相应的组件即可。

具体参考示例代码可以查看 [test](./test) 目录下的文件。