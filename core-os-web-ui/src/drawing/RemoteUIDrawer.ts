import {
    ClientMessage,
    ClientTextSizeEntry,
    Color,
    DrawCommand,
    DrawCommandKind,
    DrawCommands,
    FontType,
    InputEvent,
    InputEventCharInputKind,
    InputEventClickButton,
    InputEventControlCharKind,
    InputEventKind,
    InputStates,
    PathCommandKind,
    Size,
    SurfaceService
} from "../api/SurfaceService";

const CONST_SEND_CLIENT_MESSAGE_EVERY_MS = 100;

export class RemoteUIDrawer {

    private ctx: CanvasRenderingContext2D;
    private canvas: HTMLCanvasElement;
    private surfaceName: string;
    private webSocket: WebSocket | null = null;
    private isConnectedFlag: boolean = false;
    private lastServerFrameTime: number = 0;
    private lastServerFrameData: DrawCommand[] = null;
    private needRedraw: boolean = false;
    private nextClientStateMessage: ClientMessage
    private lastClientStateSentTime: number = 0;

    // FPS counter state (render loop)
    private _fpsFrameCount: number = 0;
    private _fpsWindowStart: number = 0;
    private _fpsCurrentValue: number = 0;

    // Backend FPS counter state (frames received from server)
    private _serverFpsFrameCount: number = 0;
    private _serverFpsWindowStart: number = 0;
    private _serverFpsCurrentValue: number = 0;

    // Input state tracking
    private _inputStates: InputStates = {
        cursorPosition: { x: 0, y: 0 },
        leftMouseButtonDown: false,
        rightMouseButtonDown: false,
        middleMouseButtonDown: false,
        shiftKeyDown: false,
        ctrlKeyDown: false,
        altKeyDown: false,
        metaKeyDown: false,
        keyCodeDown: 0,
    };
    private _pendingInputEvents: InputEvent[] = [];

    private _inputStateDirty: boolean = false;

    // Tracks font keys already sent to the server since last connect
    private _sentFontKeys: Set<string> = new Set();
    private _hasMissingFontMetrics: boolean = false;

    // Bound event handler references (needed for removeEventListener)
    private _onMouseMove: (e: MouseEvent) => void;
    private _onMouseDown: (e: MouseEvent) => void;
    private _onMouseUp: (e: MouseEvent) => void;
    private _onMouseLeave: () => void;
    private _onKeyDown: (e: KeyboardEvent) => void;
    private _onKeyUp: (e: KeyboardEvent) => void;

    constructor(surfaceName: string, ctx: CanvasRenderingContext2D, canvas: HTMLCanvasElement) {
        this.surfaceName = surfaceName;
        this.ctx = ctx;
        this.canvas = canvas;
        this._onMouseMove = (e: MouseEvent) => {
            const { x, y } = this.clientToCanvasCoords(e.clientX, e.clientY);
            const s = this._inputStates;
            if (s.cursorPosition.x !== x || s.cursorPosition.y !== y ||
                s.shiftKeyDown !== e.shiftKey || s.ctrlKeyDown !== e.ctrlKey ||
                s.altKeyDown !== e.altKey || s.metaKeyDown !== e.metaKey) {
                s.cursorPosition = { x, y };
                s.shiftKeyDown = e.shiftKey;
                s.ctrlKeyDown = e.ctrlKey;
                s.altKeyDown = e.altKey;
                s.metaKeyDown = e.metaKey;
                this._inputStateDirty = true;
            }
        };
        this._onMouseDown = (e: MouseEvent) => {
            const { x, y } = this.clientToCanvasCoords(e.clientX, e.clientY);
            const s = this._inputStates;
            const newLeft = e.button === 0 ? true : s.leftMouseButtonDown;
            const newMiddle = e.button === 1 ? true : s.middleMouseButtonDown;
            const newRight = e.button === 2 ? true : s.rightMouseButtonDown;
            if (s.cursorPosition.x !== x || s.cursorPosition.y !== y ||
                s.leftMouseButtonDown !== newLeft || s.middleMouseButtonDown !== newMiddle || s.rightMouseButtonDown !== newRight ||
                s.shiftKeyDown !== e.shiftKey || s.ctrlKeyDown !== e.ctrlKey ||
                s.altKeyDown !== e.altKey || s.metaKeyDown !== e.metaKey) {
                s.leftMouseButtonDown = newLeft;
                s.middleMouseButtonDown = newMiddle;
                s.rightMouseButtonDown = newRight;
                s.cursorPosition = { x, y };
                s.shiftKeyDown = e.shiftKey;
                s.ctrlKeyDown = e.ctrlKey;
                s.altKeyDown = e.altKey;
                s.metaKeyDown = e.metaKey;
                this._inputStateDirty = true;
            }
        };
        this._onMouseUp = (e: MouseEvent) => {
            const { x, y } = this.clientToCanvasCoords(e.clientX, e.clientY);
            const s = this._inputStates;
            const newLeft = e.button === 0 ? false : s.leftMouseButtonDown;
            const newMiddle = e.button === 1 ? false : s.middleMouseButtonDown;
            const newRight = e.button === 2 ? false : s.rightMouseButtonDown;
            if (s.cursorPosition.x !== x || s.cursorPosition.y !== y ||
                s.leftMouseButtonDown !== newLeft || s.middleMouseButtonDown !== newMiddle || s.rightMouseButtonDown !== newRight ||
                s.shiftKeyDown !== e.shiftKey || s.ctrlKeyDown !== e.ctrlKey ||
                s.altKeyDown !== e.altKey || s.metaKeyDown !== e.metaKey) {
                s.leftMouseButtonDown = newLeft;
                s.middleMouseButtonDown = newMiddle;
                s.rightMouseButtonDown = newRight;
                s.cursorPosition = { x, y };
                s.shiftKeyDown = e.shiftKey;
                s.ctrlKeyDown = e.ctrlKey;
                s.altKeyDown = e.altKey;
                s.metaKeyDown = e.metaKey;
                this._inputStateDirty = true;
            }

            // Enqueue a click event for the released button
            let clickButton = InputEventClickButton.None;
            if (e.button === 0) clickButton = InputEventClickButton.Left;
            else if (e.button === 2) clickButton = InputEventClickButton.Right;
            else if (e.button === 1) clickButton = InputEventClickButton.Middle;

            if (clickButton !== InputEventClickButton.None) {
                this._pendingInputEvents.push({
                    kind: InputEventKind.Click,
                    click: {
                        position: { x, y },
                        button: clickButton,
                    },
                });
                this._inputStateDirty = true;
            }
        };
        this._onMouseLeave = () => {
            const s = this._inputStates;
            if (s.leftMouseButtonDown || s.rightMouseButtonDown || s.middleMouseButtonDown) {
                s.leftMouseButtonDown = false;
                s.rightMouseButtonDown = false;
                s.middleMouseButtonDown = false;
                this._inputStateDirty = true;
            }
        };
        this._onKeyDown = (e: KeyboardEvent) => {
            const s = this._inputStates;
            if (s.shiftKeyDown !== e.shiftKey || s.ctrlKeyDown !== e.ctrlKey ||
                s.altKeyDown !== e.altKey || s.metaKeyDown !== e.metaKey ||
                s.keyCodeDown !== e.keyCode) {
                s.shiftKeyDown = e.shiftKey;
                s.ctrlKeyDown = e.ctrlKey;
                s.altKeyDown = e.altKey;
                s.metaKeyDown = e.metaKey;
                s.keyCodeDown = e.keyCode;
                this._inputStateDirty = true;
            }

            // Build pressed control-key bitmask (mirrors the C OR logic)
            let pressedControlKeys: number = InputEventControlCharKind.None;
            if (e.shiftKey) pressedControlKeys |= InputEventControlCharKind.Shift;
            if (e.ctrlKey)  pressedControlKeys |= InputEventControlCharKind.Ctrl;
            if (e.altKey)   pressedControlKeys |= InputEventControlCharKind.Alt;

            // Map special keys to their InputEventCharInputKind equivalents
            const specialKeyMap: { [key: string]: InputEventCharInputKind } = {
                'Enter':      InputEventCharInputKind.Enter,
                'Backspace':  InputEventCharInputKind.Backspace,
                'Delete':     InputEventCharInputKind.Delete,
                'Tab':        InputEventCharInputKind.Tab,
                'Escape':     InputEventCharInputKind.Escape,
                'ArrowUp':    InputEventCharInputKind.ArrowUp,
                'ArrowDown':  InputEventCharInputKind.ArrowDown,
                'ArrowLeft':  InputEventCharInputKind.ArrowLeft,
                'ArrowRight': InputEventCharInputKind.ArrowRight,
            };

            const specialKind = specialKeyMap[e.key];
            if (specialKind !== undefined) {
                // Special key — enqueue with empty character bytes
                this._pendingInputEvents.push({
                    kind: InputEventKind.Char,
                    charInput: {
                        kind: specialKind,
                        character0: 0,
                        character1: 0,
                        character2: 0,
                        character3: 0,
                        pressedControlKeys,
                    },
                });
                this._inputStateDirty = true;
            } else if (e.key.length === 1) {
                // Printable character — encode as UTF-8 bytes (up to 4)
                const bytes = RemoteUIDrawer.encodeUtf8Bytes(e.key);
                if (bytes !== null) {
                    this._pendingInputEvents.push({
                        kind: InputEventKind.Char,
                        charInput: {
                            kind: InputEventCharInputKind.CharInput,
                            character0: bytes[0] ?? 0,
                            character1: bytes[1] ?? 0,
                            character2: bytes[2] ?? 0,
                            character3: bytes[3] ?? 0,
                            pressedControlKeys,
                        },
                    });
                    this._inputStateDirty = true;
                }
            }
        };
        this._onKeyUp = (e: KeyboardEvent) => {
            const s = this._inputStates;
            if (s.shiftKeyDown !== e.shiftKey || s.ctrlKeyDown !== e.ctrlKey ||
                s.altKeyDown !== e.altKey || s.metaKeyDown !== e.metaKey ||
                s.keyCodeDown !== 0) {
                s.shiftKeyDown = e.shiftKey;
                s.ctrlKeyDown = e.ctrlKey;
                s.altKeyDown = e.altKey;
                s.metaKeyDown = e.metaKey;
                s.keyCodeDown = 0;
                this._inputStateDirty = true;
            }
        };
    }

    private registerInputListeners(): void {
        this.canvas.addEventListener('mousemove', this._onMouseMove);
        this.canvas.addEventListener('mousedown', this._onMouseDown);
        this.canvas.addEventListener('mouseup', this._onMouseUp);
        this.canvas.addEventListener('mouseleave', this._onMouseLeave);
        window.addEventListener('keydown', this._onKeyDown);
        window.addEventListener('keyup', this._onKeyUp);
    }

    private unregisterInputListeners(): void {
        this.canvas.removeEventListener('mousemove', this._onMouseMove);
        this.canvas.removeEventListener('mousedown', this._onMouseDown);
        this.canvas.removeEventListener('mouseup', this._onMouseUp);
        this.canvas.removeEventListener('mouseleave', this._onMouseLeave);
        window.removeEventListener('keydown', this._onKeyDown);
        window.removeEventListener('keyup', this._onKeyUp);
        this._pendingInputEvents = [];
    }

    public forceRedraw() {
        this.needRedraw = true;
    }

    public connect() {
        if (this.webSocket) {
            console.warn('WebSocket already connected');
            return;
        }
        this._sentFontKeys.clear();
        this.registerInputListeners();

        this.webSocket = SurfaceService.createWebSocket(this.surfaceName);
        this.webSocket.onopen = () => {
            this.isConnectedFlag = true;
            console.log('WebSocket connected');
        };
        this.webSocket.onmessage = (event) => {
            const data = JSON.parse(event.data) as DrawCommands;
            this.lastServerFrameTime = new Date().getTime();
            this.lastServerFrameData = data.drawCommands;
            this._serverFpsFrameCount++;
            this.needRedraw = true;
        };
        this.webSocket.onclose = () => {
            this.isConnectedFlag = false;
            console.log('WebSocket disconnected');
            this.webSocket = null;
            this.unregisterInputListeners();
        };
        this.webSocket.onerror = (error) => {
            console.error('WebSocket error:', error);
        };
    }

    public disconnect() {
        if (this.webSocket) {
            this.isConnectedFlag = false;
            this.webSocket.close();
            this.webSocket = null;
            this.unregisterInputListeners();
        }
    }

    public isConnected(): boolean {
        return this.webSocket !== null && this.isConnectedFlag;
    }

    public drawFrame() {

        const w = this.canvas.offsetWidth;
        const h = this.canvas.offsetHeight;
        const now = Date.now();

        if ((this._inputStateDirty || this._hasMissingFontMetrics) && (now - this.lastClientStateSentTime > CONST_SEND_CLIENT_MESSAGE_EVERY_MS)) {

            // Send an updated client state to the server whenever we detect a change in input state
            this.sendClientState(w, h);

            this._inputStateDirty = false;
            this._hasMissingFontMetrics = false;
            this.lastClientStateSentTime = now;
        }


        if (!this.needRedraw) {
            // Still draw the FPS overlay even when no new frame data, to keep it updated
            this.drawFpsOverlay(w, h);
            return;
        }

        this.ctx.clearRect(0, 0, w, h);

        if (!this.lastServerFrameData || this.lastServerFrameData.length === 0) {
            this.needRedraw = false;
            return;
        }

        const commands = this.lastServerFrameData;
        const ctx = this.ctx;

        // We walk the command list.  Gradient commands are "look-ahead" markers:
        // when we encounter one we pre-build the CanvasGradient (collecting the
        // following Stop commands), store it, and use it as fill/stroke for the
        // very next shape command.
        let pendingGradient: CanvasGradient | null = null;

        let i = 0;
        while (i < commands.length) {
            const cmd = commands[i];

            switch (cmd.kind) {

                // ---- Gradient definitions (SVG <linearGradient> / <radialGradient>) ----
                case DrawCommandKind.LinearGradient:
                case DrawCommandKind.RadialGradient: {
                    pendingGradient = this.buildPendingGradient(commands, i);
                    // Skip past all following Stop commands
                    i++;
                    while (i < commands.length && commands[i].kind === DrawCommandKind.Stop) {
                        i++;
                    }
                    continue; // do NOT increment i again at the bottom
                }

                // ---- Stop commands are consumed by buildPendingGradient; skip standalone ones ----
                case DrawCommandKind.Stop: {
                    i++;
                    continue;
                }

                // ---- Rect ----
                case DrawCommandKind.Rect: {
                    if (!cmd.rect) break;
                    const r = cmd.rect;

                    ctx.beginPath();

                    if (r.rx > 0 || r.ry > 0) {
                        // Rounded rectangle (SVG rx/ry — use the same radius for both axes when ry is 0)
                        const rx = r.rx > 0 ? r.rx : r.ry;
                        const ry = r.ry > 0 ? r.ry : r.rx;
                        ctx.moveTo(r.x + rx, r.y);
                        ctx.lineTo(r.x + r.width - rx, r.y);
                        ctx.ellipse(r.x + r.width - rx, r.y + ry, rx, ry, 0, -Math.PI / 2, 0);
                        ctx.lineTo(r.x + r.width, r.y + r.height - ry);
                        ctx.ellipse(r.x + r.width - rx, r.y + r.height - ry, rx, ry, 0, 0, Math.PI / 2);
                        ctx.lineTo(r.x + rx, r.y + r.height);
                        ctx.ellipse(r.x + rx, r.y + r.height - ry, rx, ry, 0, Math.PI / 2, Math.PI);
                        ctx.lineTo(r.x, r.y + ry);
                        ctx.ellipse(r.x + rx, r.y + ry, rx, ry, 0, Math.PI, -Math.PI / 2);
                        ctx.closePath();
                    } else {
                        ctx.rect(r.x, r.y, r.width, r.height);
                    }

                    ctx.fillStyle = pendingGradient ?? this.colorToCss(r.color);
                    ctx.fill();

                    if (r.stroke_width > 0) {
                        ctx.strokeStyle = this.colorToCss(r.stroke);
                        ctx.lineWidth = r.stroke_width;
                        ctx.stroke();
                    }

                    pendingGradient = null;
                    break;
                }

                // ---- Circle ----
                case DrawCommandKind.Circle: {
                    if (!cmd.circle) break;
                    const c = cmd.circle;

                    ctx.beginPath();
                    ctx.arc(c.cx, c.cy, c.r, 0, Math.PI * 2);

                    ctx.fillStyle = pendingGradient ?? this.colorToCss(c.fill);
                    ctx.fill();

                    if (c.stroke_width > 0) {
                        ctx.strokeStyle = this.colorToCss(c.stroke);
                        ctx.lineWidth = c.stroke_width;
                        ctx.stroke();
                    }

                    pendingGradient = null;
                    break;
                }

                // ---- Ellipse ----
                case DrawCommandKind.Ellipse: {
                    if (!cmd.ellipse) break;
                    const e = cmd.ellipse;

                    ctx.beginPath();
                    ctx.ellipse(e.cx, e.cy, e.rx, e.ry, 0, 0, Math.PI * 2);

                    ctx.fillStyle = pendingGradient ?? this.colorToCss(e.fill);
                    ctx.fill();

                    if (e.stroke_width > 0) {
                        ctx.strokeStyle = this.colorToCss(e.stroke);
                        ctx.lineWidth = e.stroke_width;
                        ctx.stroke();
                    }

                    pendingGradient = null;
                    break;
                }

                // ---- Line ----
                case DrawCommandKind.Line: {
                    if (!cmd.line) break;
                    const l = cmd.line;

                    ctx.beginPath();
                    ctx.moveTo(l.x1, l.y1);
                    ctx.lineTo(l.x2, l.y2);

                    ctx.strokeStyle = this.colorToCss(l.stroke);
                    ctx.lineWidth = l.stroke_width;
                    ctx.stroke();

                    pendingGradient = null;
                    break;
                }

                // ---- Polyline ----
                case DrawCommandKind.Polyline: {
                    if (!cmd.polyline || cmd.polyline.points.length < 2) break;
                    const pl = cmd.polyline;

                    ctx.beginPath();
                    ctx.moveTo(pl.points[0].x, pl.points[0].y);
                    for (let j = 1; j < pl.points.length; j++) {
                        ctx.lineTo(pl.points[j].x, pl.points[j].y);
                    }

                    // Polyline has a fill in the MLA model (hollow by default when alpha=0)
                    if (pl.fill.a > 0) {
                        ctx.fillStyle = pendingGradient ?? this.colorToCss(pl.fill);
                        ctx.fill();
                    }

                    if (pl.stroke_width > 0) {
                        ctx.strokeStyle = this.colorToCss(pl.stroke);
                        ctx.lineWidth = pl.stroke_width;
                        ctx.stroke();
                    }

                    pendingGradient = null;
                    break;
                }

                // ---- Polygon ----
                case DrawCommandKind.Polygon: {
                    if (!cmd.polygon || cmd.polygon.points.length < 3) break;
                    const pg = cmd.polygon;

                    ctx.beginPath();
                    ctx.moveTo(pg.points[0].x, pg.points[0].y);
                    for (let j = 1; j < pg.points.length; j++) {
                        ctx.lineTo(pg.points[j].x, pg.points[j].y);
                    }
                    ctx.closePath();

                    ctx.fillStyle = pendingGradient ?? this.colorToCss(pg.fill);
                    ctx.fill();

                    if (pg.stroke_width > 0) {
                        ctx.strokeStyle = this.colorToCss(pg.stroke);
                        ctx.lineWidth = pg.stroke_width;
                        ctx.stroke();
                    }

                    pendingGradient = null;
                    break;
                }

                // ---- Path ----
                case DrawCommandKind.Path: {
                    if (!cmd.path || cmd.path.commands.length === 0) break;
                    const p = cmd.path;

                    ctx.beginPath();
                    this._currentPathX = 0;
                    this._currentPathY = 0;

                    for (const pathCmd of p.commands) {
                        switch (pathCmd.kind) {
                            case PathCommandKind.MoveTo:
                                if (pathCmd.moveTo) {
                                    ctx.moveTo(pathCmd.moveTo.x, pathCmd.moveTo.y);
                                    this._currentPathX = pathCmd.moveTo.x;
                                    this._currentPathY = pathCmd.moveTo.y;
                                }
                                break;

                            case PathCommandKind.LineTo:
                                if (pathCmd.lineTo) {
                                    ctx.lineTo(pathCmd.lineTo.x, pathCmd.lineTo.y);
                                    this._currentPathX = pathCmd.lineTo.x;
                                    this._currentPathY = pathCmd.lineTo.y;
                                }
                                break;

                            case PathCommandKind.QuadraticCurveTo:
                                if (pathCmd.quadraticCurveTo) {
                                    const q = pathCmd.quadraticCurveTo;
                                    ctx.quadraticCurveTo(q.cpx, q.cpy, q.x, q.y);
                                    this._currentPathX = q.x;
                                    this._currentPathY = q.y;
                                }
                                break;

                            case PathCommandKind.CubicCurveTo:
                                if (pathCmd.cubicCurveTo) {
                                    const cu = pathCmd.cubicCurveTo;
                                    ctx.bezierCurveTo(cu.cp1x, cu.cp1y, cu.cp2x, cu.cp2y, cu.x, cu.y);
                                    this._currentPathX = cu.x;
                                    this._currentPathY = cu.y;
                                }
                                break;

                            case PathCommandKind.ArcTo: {
                                if (pathCmd.arcTo) {
                                    // SVG arc-to → Canvas arc conversion
                                    const a = pathCmd.arcTo;
                                    this.canvasArcTo(
                                        a.rx, a.ry,
                                        a.x_axis_rotation,
                                        a.large_arc_flag,
                                        a.sweep_flag,
                                        a.x, a.y
                                    );
                                    // _currentPathX/_currentPathY updated inside canvasArcTo
                                }
                                break;
                            }

                            case PathCommandKind.ClosePath:
                                ctx.closePath();
                                break;
                        }
                    }

                    ctx.fillStyle = pendingGradient ?? this.colorToCss(p.fill);
                    ctx.fill();

                    if (p.stroke_width > 0) {
                        ctx.strokeStyle = this.colorToCss(p.stroke);
                        ctx.lineWidth = p.stroke_width;
                        ctx.stroke();
                    }

                    pendingGradient = null;
                    break;
                }

                // ---- Text ----
                case DrawCommandKind.Text: {
                    if (!cmd.text || !cmd.text.content) break;
                    const t = cmd.text;
                    const ft = t.font_type;

                    // Detect unseen fonts while rendering — cheaper than scanning the whole frame on every message
                    const key = this.buildFontCacheKey(ft);
                    if (!this._sentFontKeys.has(key)) {
                        this._hasMissingFontMetrics = true;
                    }

                    ctx.font = `${ft.italic ? 'italic ' : ''}${ft.bold ? 'bold ' : ''}${ft.size}px ${ft.family}`;
                    ctx.fillStyle = this.colorToCss(t.fill);
                    ctx.textBaseline = 'top';
                    ctx.fillText(t.content, t.x, t.y);

                    pendingGradient = null;
                    break;
                }

                default:
                    break;
            }

            i++;
        }

        this.drawFpsOverlay(w, h);
        this.needRedraw = false;
    }

    private sendClientState(width: number, height: number) {

        if (!this.isConnected()) {
            return;
        }

        if (this.nextClientStateMessage == null) {
            this.nextClientStateMessage = {}
        }

        this.nextClientStateMessage.inputStates = this.buildInputStates();
        this.nextClientStateMessage.textSize = this.buildNeedTextSize();
        this.nextClientStateMessage.surface_size = { width, height };

        // Attach any queued discrete input events and flush the queue
        if (this._pendingInputEvents.length > 0) {
            this.nextClientStateMessage.inputEvents = this._pendingInputEvents.slice();
            this._pendingInputEvents = [];
        }

        const message = JSON.stringify(this.nextClientStateMessage);
        this.webSocket.send(message);

        // Clear the message after sending to avoid resending the same state repeatedly
        this.nextClientStateMessage = {};
    }

    /**
     * Builds the current input states from tracked mouse/keyboard state.
     */
    private buildInputStates(): InputStates {
        return this._inputStates;
    }


    /**
     * Measures text sizes for all unique fonts found across ALL text commands in
     * the current frame. Fonts that have already been sent to the server since
     * the last connect are skipped. Returns null when there is nothing new to send.
     */
    private buildNeedTextSize(): ClientTextSizeEntry[] | null {
        if (!this.lastServerFrameData || this.lastServerFrameData.length === 0) {
            return null;
        }

        // Collect all unique fonts from every text command in the current frame
        const newFonts = new Map<string, FontType>();
        for (const cmd of this.lastServerFrameData) {
            if (cmd.kind === DrawCommandKind.Text && cmd.text && cmd.text.font_type) {
                const ft = cmd.text.font_type;
                const key = this.buildFontCacheKey(ft);
                if (!newFonts.has(key) && !this._sentFontKeys.has(key)) {
                    newFonts.set(key, ft);
                }
            }
        }

        if (newFonts.size === 0) {
            return null;
        }

        const ctx = this.ctx;
        const ASCII_START = 32;
        const ASCII_END = 126;
        const entries: ClientTextSizeEntry[] = [];

        for (const [key, fontType] of newFonts.entries()) {
            ctx.save();
            ctx.font = `${fontType.italic ? 'italic ' : ''}${fontType.bold ? 'bold ' : ''}${fontType.size}px ${fontType.family}`;

            const fallbackMetrics = ctx.measureText('M');
            const fallbackSize: Size = {
                width: fallbackMetrics.width,
                height: fontType.size,
            };

            const size_per_char: Size[] = [];
            for (let code = ASCII_START; code <= ASCII_END; code++) {
                const char = String.fromCharCode(code);
                const metrics = ctx.measureText(char);
                const charHeight =
                    metrics.actualBoundingBoxAscent !== undefined && metrics.actualBoundingBoxDescent !== undefined
                        ? metrics.actualBoundingBoxAscent + metrics.actualBoundingBoxDescent
                        : fontType.size;
                size_per_char.push({ width: metrics.width, height: charHeight });
            }

            // Fallback entry for characters outside ASCII 32–126
            size_per_char.push(fallbackSize);

            ctx.restore();

            // Mark this font as sent so we don't measure and re-send it again
            this._sentFontKeys.add(key);

            entries.push({ fontType, size_per_char });
        }

        return entries;
    }

    // -------------------------------------------------------------------------
    // Private helpers
    // -------------------------------------------------------------------------

    private clientToCanvasCoords(clientX: number, clientY: number): { x: number; y: number } {
        const rect = this.canvas.getBoundingClientRect();
        return {
            x: clientX - rect.left,
            y: clientY - rect.top,
        };
    }

    /**
     * Encodes a single character into up to 4 UTF-8 bytes.
     * Returns a number[] of length 1–4, or null if encoding failed or exceeded 4 bytes.
     */
    private static encodeUtf8Bytes(char: string): number[] | null {
        const encoded = new TextEncoder().encode(char);
        if (encoded.length === 0 || encoded.length > 4) return null;
        return Array.from(encoded);
    }

    /** Convert an MLA Color (0-255 channels) to a CSS rgba() string. */
    private colorToCss(color: Color): string {
        return `rgba(${color.r},${color.g},${color.b},${color.a / 255})`;
    }

    /**
     * Build a CanvasGradient from consecutive LinearGradient / RadialGradient +
     * Stop commands that appear before the current index.
     *
     * Returns null when no gradient is pending.
     */
    private buildPendingGradient(
        commands: DrawCommand[],
        gradientCmdIndex: number
    ): CanvasGradient | null {
        const gradCmd = commands[gradientCmdIndex];
        const ctx = this.ctx;

        let gradient: CanvasGradient | null = null;

        if (gradCmd.kind === DrawCommandKind.LinearGradient && gradCmd.linearGradient) {
            const g = gradCmd.linearGradient;
            gradient = ctx.createLinearGradient(g.x1, g.y1, g.x2, g.y2);
        } else if (gradCmd.kind === DrawCommandKind.RadialGradient && gradCmd.radialGradient) {
            const g = gradCmd.radialGradient;
            gradient = ctx.createRadialGradient(g.fx, g.fy, 0, g.cx, g.cy, g.r);
        }

        if (!gradient) return null;

        // Collect the Stop commands that immediately follow
        for (let k = gradientCmdIndex + 1; k < commands.length; k++) {
            const next = commands[k];
            if (next.kind === DrawCommandKind.Stop && next.stop) {
                const s = next.stop;
                const alpha = Math.max(0, Math.min(1, s.stop_opacity));
                gradient.addColorStop(
                    s.offset,
                    `rgba(${s.stop_color.r},${s.stop_color.g},${s.stop_color.b},${alpha})`
                );
            } else {
                break;
            }
        }

        return gradient;
    }

    private buildFontCacheKey(ft: FontType) {
        return `${ft.family}|${ft.size}|${ft.bold ? 1 : 0}|${ft.italic ? 1 : 0}`
    }

    // -------------------------------------------------------------------------
    // Public draw entry-point
    // -------------------------------------------------------------------------


    // -------------------------------------------------------------------------
    // FPS overlay
    // -------------------------------------------------------------------------

    private drawFpsOverlay(canvasWidth: number, canvasHeight: number): void {
        if (this.lastServerFrameTime === 0) {
            return;
        }

        const now = this.lastServerFrameTime;

        // --- Render FPS (how often drawFrame is called) ---
        if (this._fpsWindowStart === 0) {
            this._fpsWindowStart = now;
        }
        this._fpsFrameCount++;
        const renderElapsed = now - this._fpsWindowStart;
        if (renderElapsed >= 1000) {
            this._fpsCurrentValue = Math.round(this._fpsFrameCount * 1000 / renderElapsed);
            this._fpsFrameCount = 0;
            this._fpsWindowStart = now;
        }

        // --- Backend FPS (how many frames arrived from the server) ---
        if (this._serverFpsWindowStart === 0) {
            this._serverFpsWindowStart = now;
        }
        const serverElapsed = now - this._serverFpsWindowStart;
        if (serverElapsed >= 1000) {
            this._serverFpsCurrentValue = Math.round(this._serverFpsFrameCount * 1000 / serverElapsed);
            this._serverFpsFrameCount = 0;
            this._serverFpsWindowStart = now;
        }

        const lines = [
            `Render FPS:  ${this._fpsCurrentValue}`,
            `Server FPS:  ${this._serverFpsCurrentValue}`,
        ];

        const ctx = this.ctx;
        const fontSize = 13;
        const lineHeight = fontSize + 4;
        const padding = 6;

        ctx.save();
        ctx.font = `bold ${fontSize}px monospace`;

        const maxTextWidth = lines.reduce((max, l) => Math.max(max, ctx.measureText(l).width), 0);
        const boxW = maxTextWidth + padding * 2;
        const boxH = lineHeight * lines.length + padding * 2;
        const boxX = canvasWidth - boxW - 4;
        const boxY = canvasHeight - boxH - 4;

        // Background box
        ctx.fillStyle = 'rgba(0,0,0,1)';
        ctx.fillRect(boxX, boxY, boxW, boxH);

        // Text lines
        ctx.fillStyle = 'rgba(255,80,80,1)';
        ctx.textBaseline = 'top';
        lines.forEach((line, idx) => {
            ctx.fillText(line, boxX + padding, boxY + padding + idx * lineHeight);
        });

        ctx.restore();
    }

    // -------------------------------------------------------------------------
    // SVG arc-to → Canvas 2D conversion
    // -------------------------------------------------------------------------
    // Based on the W3C SVG specification's "Conversion from endpoint to center parameterization"
    // https://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
    private canvasArcTo(
        rx: number, ry: number,
        xAxisRotationDeg: number,
        largeArcFlag: boolean,
        sweepFlag: boolean,
        x2: number, y2: number
    ): void {
        const ctx = this.ctx;
        const phi = (xAxisRotationDeg * Math.PI) / 180;
        const cosPhi = Math.cos(phi);
        const sinPhi = Math.sin(phi);

        // (x1, y1) is tracked by drawFrame via _currentPathX/_currentPathY
        const x1 = this._currentPathX;
        const y1 = this._currentPathY;

        // Clamp radii
        rx = Math.abs(rx);
        ry = Math.abs(ry);

        if (rx === 0 || ry === 0) {
            ctx.lineTo(x2, y2);
            this._currentPathX = x2;
            this._currentPathY = y2;
            return;
        }

        // Step 1: Compute (x1', y1')
        const dx2 = (x1 - x2) / 2;
        const dy2 = (y1 - y2) / 2;
        const x1p = cosPhi * dx2 + sinPhi * dy2;
        const y1p = -sinPhi * dx2 + cosPhi * dy2;

        // Step 2: Compute (cx', cy')
        let x1pSq = x1p * x1p;
        let y1pSq = y1p * y1p;
        let rxSq = rx * rx;
        let rySq = ry * ry;

        // Ensure radii are large enough
        const lambda = Math.sqrt(x1pSq / rxSq + y1pSq / rySq);
        if (lambda > 1) {
            rx *= lambda;
            ry *= lambda;
            rxSq = rx * rx;
            rySq = ry * ry;
            x1pSq = x1p * x1p;
            y1pSq = y1p * y1p;
        }

        const num = Math.max(0, rxSq * rySq - rxSq * y1pSq - rySq * x1pSq);
        const den = rxSq * y1pSq + rySq * x1pSq;
        const sq = den === 0 ? 0 : Math.sqrt(num / den);
        const sign = largeArcFlag === sweepFlag ? -1 : 1;
        const cxp = sign * sq * (rx * y1p) / ry;
        const cyp = sign * sq * -(ry * x1p) / rx;

        // Step 3: Compute (cx, cy) from (cx', cy')
        const cx = cosPhi * cxp - sinPhi * cyp + (x1 + x2) / 2;
        const cy = sinPhi * cxp + cosPhi * cyp + (y1 + y2) / 2;

        // Step 4: Compute θ1 and dθ
        const ux = (x1p - cxp) / rx;
        const uy = (y1p - cyp) / ry;
        const vx = (-x1p - cxp) / rx;
        const vy = (-y1p - cyp) / ry;

        const angle = (u: number[], v: number[]) => {
            const dot = u[0] * v[0] + u[1] * v[1];
            const len = Math.sqrt(u[0] * u[0] + u[1] * u[1]) * Math.sqrt(v[0] * v[0] + v[1] * v[1]);
            return Math.sign(u[0] * v[1] - u[1] * v[0]) * Math.acos(Math.max(-1, Math.min(1, dot / len)));
        };

        let theta1 = angle([1, 0], [ux, uy]);
        let dTheta = angle([ux, uy], [vx, vy]);

        if (!sweepFlag && dTheta > 0) dTheta -= 2 * Math.PI;
        if (sweepFlag && dTheta < 0) dTheta += 2 * Math.PI;

        const theta2 = theta1 + dTheta;
        const counterClockwise = !sweepFlag;

        // Canvas ellipse uses the un-rotated coordinate system; we handle rotation via transform
        ctx.save();
        ctx.translate(cx, cy);
        ctx.rotate(phi);
        ctx.scale(1, ry / rx);   // squish to make it a unit circle in x
        ctx.arc(0, 0, rx, theta1, theta2, counterClockwise);
        ctx.restore();

        // After restore the path is still connected because arc() appends to the current path
        this._currentPathX = x2;
        this._currentPathY = y2;
    }

    // Tracked current path cursor (needed for SVG arc-to conversion)
    private _currentPathX: number = 0;
    private _currentPathY: number = 0;
}

