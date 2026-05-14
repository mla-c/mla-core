# mla-c Web UI

A Preact/TypeScript web application that provides a browser-based interface for interacting with mla-c applications. The built output is embedded directly into the mla-c C++ library as a compressed byte array, served by the mla-c HTTP server at runtime.

## Architecture

The web UI communicates with the mla-c backend via two channels:

- **RPC over HTTP**: JSON-based remote procedure calls to `/rpc/` endpoints for logging, configuration, and surface discovery.
- **WebSocket**: Real-time bidirectional communication for UI surface rendering, enabling remote display of native UI controls in the browser.

### Build Pipeline

```
Preact/TypeScript Source
        ↓ (Vite build)
Single HTML file (bundled JS/CSS)
        ↓ (vite-plugin-singlefile)
Gzip-compressed byte array
        ↓ (custom Vite plugin: generateCHeader)
C header: core/ui/web/mla_ui_web_embedded.h
        ↓ (compiled into mla-c)
Served via mla_ui_http_server at runtime
```

## Components

### Pages

- **HomePage**: Main landing page
- **SurfaceUIPage**: Remote UI surface rendering via WebSocket with canvas-based drawing

### Services (API Layer)

- **ApiService**: Generic RPC client for calling backend procedures via `/rpc/` endpoints.
- **LogService**: Logging control (get/set log level, fetch log messages).
- **SurfaceService**: Surface discovery and WebSocket-based remote rendering. Handles draw commands (rect, circle, ellipse, line, polyline, polygon, path, text, gradients), input events (click, keyboard), and text metrics.

### UI Components

- **MainLayout**: Application shell with navigation
- **Menu / MenuItem**: Sidebar navigation menu
- **LogViewer**: Real-time log message display with level filtering

### Hooks

- **useLoadingState**: Loading state management
- **useLogService**: Log service integration
- **useBootstrap**: Application initialization
- **useCancellableEffect**: Cancellable side effects for cleanup

### Drawing

- **RemoteUIDrawer**: Renders draw commands from the backend onto an HTML5 canvas
- **LoadingDrawer**: Loading state animation

## Development

### Prerequisites

- Node.js (with npm)

### Getting Started

```bash
npm install
```

- `npm run dev` - Starts a dev server at http://localhost:5173/
- `npm run build` - Builds for production and generates the embedded C header
- `npm run preview` - Starts a server at http://localhost:4173/ to test the production build locally

### Tech Stack

- **Preact** 10.x - Lightweight React alternative
- **TypeScript** 5.x - Type-safe development
- **Vite** 7.x - Fast build tool
- **vite-plugin-singlefile** - Bundles all assets into a single HTML file
- **vite-plugin-singlefile-compression** - Gzip compression for minimal embedded size

## Integration with mla-c

The production build generates `core/ui/web/mla_ui_web_embedded.h`, which contains the entire web application as a gzip-compressed byte array. This header is included in the mla-c library and served by `mla_ui_http_server` when a client connects via browser.

For more information on the server-side integration, see the [UI Module Documentation](../core/ui/readme.md).
