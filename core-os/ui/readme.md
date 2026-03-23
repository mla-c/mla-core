# UI Module

The UI module provides a cross-platform user interface framework for CoreOS applications. It supports both native display surfaces and web-based remote rendering via HTTP/WebSocket, allowing applications to be controlled and monitored through a browser.

## Architecture

The UI module is organized into four layers:

- **Controls** (`controls/`): Reusable UI widgets (buttons, labels, text inputs, windows) built on a base control abstraction with layout, rendering, and input handling.
- **Drawing** (`surfaces/`): A rendering primitive layer that defines draw commands (rectangles, circles, lines, text, gradients, paths) and input events, all serializable for network transmission.
- **Display** (`display/`): Platform-specific display surface backends (native windows, remote web canvas).
- **Web Integration** (`web/`): HTTP server and WebSocket-based remote rendering, including embedded web client resources and RPC surface discovery. See also: [Web UI Documentation](web/readme.md)

### Component Relationships

```
┌──────────────────────────────────────────┐
│  Application Logic                       │
│  (manipulates control tree each frame)   │
└───────────────┬──────────────────────────┘
                │
                ▼
┌──────────────────────────────────────────┐
│  Control Surface (mla_ui_control_surface)│
│  Bridges controls ↔ rendering backend    │
│  Manages render loop and input dispatch  │
└───────────────┬──────────────────────────┘
                │
       ┌────────┴─────────┐
       ▼                  ▼
   Render Phase       Input Phase
   Controls →         Surface →
   Draw Commands      Input Events →
   → Surface          Controls
       │
       ▼
┌──────────────────────────────────────────┐
│  Surface Backends                        │
│  ├─ Native Display Surface               │
│  └─ Web Remote Surface (WebSocket)       │
└──────────────────────────────────────────┘
```

## Controls

The UI module provides several built-in controls, all based on `mla_ui_control_t`:

### Button (`mla_ui_button.h`)

Interactive button with multiple visual styles and click event support.

```cpp
#include "mla_ui_button.h"

// Create a primary-style button
mla_ui_control_t button = mla_ui_button();
mla_ui_button_set_text(button, mla_string_const("Click Me"));
mla_ui_button_set_style(button, MLA_UI_BUTTON_STYLE_PRIMARY);
mla_ui_button_set_click_event(button, my_click_handler, user_data);
```

**Styles**: `PRIMARY`, `SECONDARY`, `TERTIARY`, `LINK`, `CUSTOM`

### Label (`mla_ui_label.h`)

Text display control with configurable font size and text styling.

```cpp
#include "mla_ui_label.h"

mla_ui_control_t label = mla_ui_label();
mla_ui_label_set_text(label, mla_string_const("Hello, World!"));
mla_ui_label_set_text_kind(label, MLA_UI_TEXT_KIND_PRIMARY);
mla_ui_label_set_font_size(label, MLA_UI_FONT_SIZE_PAGE_TITLE);
```

**Text Kinds**: `PRIMARY`, `SECONDARY`, `DISABLED`, `LINK`, `ERROR`, `WARNING`, `SUCCESS`, `INFO`, `CUSTOM`

### Text Edit (`mla_ui_text_edit.h`)

Editable text input with cursor positioning, text selection, and change events.

```cpp
#include "mla_ui_text_edit.h"

mla_ui_control_t textEdit = mla_ui_text_edit();
mla_ui_text_edit_set_text(textEdit, mla_string_const("Initial text"));
mla_ui_text_edit_set_style(textEdit, MLA_UI_TEXT_EDIT_STYLE_STANDARD);
```

**Styles**: `STANDARD`, `PASSWORD`

### Window (`mla_ui_window.h`)

Top-level container control with title, resize support, and close event.

```cpp
#include "mla_ui_window.h"

mla_ui_control_t window = mla_ui_window();
mla_ui_window_set_title(window, mla_string_const("My Application"));
mla_ui_window_set_resizable(window, true);
```

### Loading Indicator (`mla_ui_loading_indicator.h`)

Simple animated loading indicator.

```cpp
#include "mla_ui_loading_indicator.h"

mla_ui_control_t loader = mla_ui_loading_indicator();
```

## Drawing Primitives

The `mla_ui_surface_draw.h` header defines all rendering primitives used by the UI framework. Draw commands are serializable, enabling remote rendering over WebSocket.

**Supported Draw Commands**:
- `RECT` - Rectangles with optional rounded corners, fill, and stroke
- `CIRCLE` - Circles with fill and stroke
- `ELLIPSE` - Ellipses with fill and stroke
- `LINE` - Lines with stroke color and width
- `POLYLINE` - Connected line segments
- `POLYGON` - Filled polygons
- `PATH` - Complex paths (move, line, quadratic/cubic curves, arcs)
- `TEXT` - Text rendering with font type and color
- `LINEAR_GRADIENT` / `RADIAL_GRADIENT` - Gradient definitions with color stops

## Surface Backends

### Native Display Surface (`mla_ui_display_surface.h`)

Creates a platform-native rendering surface (e.g., Windows GDI+/Direct2D, Linux X11).

```cpp
#include "mla_ui_display_surface.h"

mla_ui_surface_t surface = mla_ui_display_surface_create();
```

### Web Remote Surface (`mla_ui_web_remote_surface.h`)

Creates a remote rendering surface over a WebSocket connection, allowing browser-based UI interaction.

```cpp
#include "mla_ui_web_remote_surface.h"

mla_ui_surface_t surface = mla_ui_web_remote_surface_create(websocket_connection);
```

## Control Surface

The `mla_ui_control_surface_t` bridges the control tree with a rendering surface. It manages the render loop, dispatches input events to controls, and supports both multi-threaded and single-threaded operation.

```cpp
#include "mla_ui_control_surface.h"

// Create a control surface with a render callback
mla_ui_control_surface_t controlSurface = mla_ui_control_surface_create(surface, render_callback, user_data);

// Start the render loop (multi-threaded)
mla_ui_control_surface_start(controlSurface);

// Or use single-threaded mode
mla_ui_control_surface_start_single_threaded_mode(controlSurface);
```

## Web Integration

The web layer enables browser-based access to UI surfaces:

- **`mla_ui_http_server.h`**: Initializes HTTP endpoints and WebSocket handlers for serving UI surfaces.
- **`mla_ui_rpc.h`**: Exposes an RPC procedure (`ui/surfaces`) for discovering available UI surfaces.
- **`mla_ui_web_embedded.h`**: Contains the embedded web client (HTML/JS/CSS) as a compressed byte array, auto-generated from the `core-os-web-ui` project.

```cpp
#include "mla_ui_http_server.h"

// Initialize the UI HTTP server on an existing HTTP server
mla_ui_http_server_initialize(server);

// Add a web surface accessible via browser
mla_ui_http_server_add_web_surface(server, mla_string_const("Main Window"), mla_string_const("main"), render_callback, user_data);
```

## Styling

The `mla_ui_style.h` header defines font sizes and color constants used across all controls:

- **Font Sizes**: `DEFAULT` (12), `SECONDARY` (11), `CAPTION` (10), `SECTION` (14), `PAGE_TITLE` (18), `PAGE_TITLE_LG` (22)
- **Text Colors**: Primary, secondary, disabled, link, error, warning, success, info
- **Button Colors**: Per-style colors for enabled, hovered, disabled, and pressed states

## Integration with Other Modules

The UI module integrates with several other CoreOS modules:

- **HTTP Module** - Serves UI web client and handles WebSocket connections
- **RPC Module** - Exposes surface discovery and interaction procedures
- **Serializer Module** - Serializes draw commands and input events for network transmission
- **Task Module** - Manages render loop scheduling and thread-safe control access
