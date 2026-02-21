# Skia UI Surface Implementation

This directory contains the Skia-based cross-platform UI surface renderer for the MLA framework.

## Overview

The Skia renderer provides hardware-accelerated 2D rendering using the [Skia Graphics Library](https://skia.org/).
It implements the `mla_ui_surface_t` interface for creating native window surfaces with GPU-accelerated drawing.

## Features

- Full implementation of all MLA draw commands (rect, circle, ellipse, line, polyline, polygon, path, text, gradients)
- GPU-accelerated rendering via Skia's GL backend
- Font caching for text rendering performance (LRU cache)
- Brush/paint reuse to minimize allocations per frame
- Native window integration (X11 on Linux, HWND on Windows)
- Input event handling (mouse clicks, keyboard events)
- FPS counter in debug builds (`mla_debug_build`)

## Usage

Include the appropriate Skia surface header for your platform:

```cpp
#include "core-os-platform/skia/mla_global_ui_surface_skia.h"
```

The renderer registers itself via the global `g_ui_display_surface_low_level_access` provider.
Surfaces are created through `mla_ui_display_surface_create()`.

## Dependencies

- Skia library (linked externally)
- Platform windowing system (X11/Win32)
- OpenGL (for GPU backend)

## Performance

Designed for 60 FPS rendering with:
- Cached `SkFont` / `SkTypeface` objects (avoids per-frame font creation)
- Single reusable `SkPaint` object with color updates
- GPU-accelerated Skia backend (`GrDirectContext` + `SkSurface`)
- Immediate present mode in debug builds for lowest latency
