# uikit

[English|[简体中文](./README_zh-cn.md)]

## Introduction

UIKit is a toolkit for developing graphical user interfaces. It provides a set of common UI components and tools to simplify the development process.

UIKit includes the following tools:
- Font Manager: a comprehensive font management tool.
    - It supports multiple functions, such as loading fonts, creating fonts, and deleting fonts.
    - It also supports font cache management, which can avoid loading fonts repeatedly and frequent IO reading, thus achieving efficient font rendering.
    - The Font Manager also specifically expands the management function of emoji fonts, and supports image fonts in formats such as png and bin.

- QR Code Scanner: a QR code scanner based on the quirc library. Before using it, the image needs to be converted to grayscale. It provides interfaces for converting YUYV, NV12, RGBA8888, RGB565, RGB332 and other formats of images to grayscale. The scanning result will be returned as a string.

- Video Widget: based on the existing media framework of OpenVela, it extends the LVGL image widget to enhance LVGL's control, acquisition, and rendering capabilities for video buffers.

- Asynchronous Refresh: supports executing some tasks before or after the next refresh. You need to initialize the asynchronous refresh module first, and then register some asynchronous function callbacks. These callback functions will be executed before or after the next refresh.

## Directory

```
├── include
│   └── uikit
├── src
│   ├── draw
│   ├── font_manager
│   ├── qrscan
│   ├── uikit_async.c
│   ├── uikit.c
│   ├── uikit_font_manager.c
│   ├── uikit_internal.h
│   └── video
└── test
```

## Usage

Using UIKit is very simple, just include the corresponding component in your code.

For more information, please refer to the sample code in the [test](./test) directory.