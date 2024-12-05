# uikit

[English|[简体中文](./README_zh-cn.md)]

## Introduction

UIKit is a toolkit for developing graphical user interfaces. It provides a set of common UI components and tools to simplify the development process.

UIKit includes the following tools:
- Font Manager: a full-featured font management tool that supports functions such as loading, creating, and deleting fonts. In addition, it supports font cache management, which can avoid redundant loading of fonts and frequent I/O reads, thereby achieving efficient font rendering. The Font Manager also has a special extension for managing emoji fonts and supports image fonts in PNG, BIN, and other formats.

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

For more information, please refer to the sample code in the `test` directory.