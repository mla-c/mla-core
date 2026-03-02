import {
    Color,
    DrawCommand,
    DrawCommandKind,
    DrawCommands,
    PathCommandKind,
    SurfaceService
} from "../api/SurfaceService";

export class RemoteUIDrawer {

    private ctx: CanvasRenderingContext2D;
    private canvas: HTMLCanvasElement;
    private surfaceName: string;
    private webSocket: WebSocket | null = null;
    private isConnectedFlag: boolean = false;
    private lastServerFrameTime: number = 0;
    private lastServerFrameData: DrawCommand[] = null;
    private needRedraw: boolean = false;

    // FPS counter state (render loop)
    private _fpsFrameCount: number = 0;
    private _fpsWindowStart: number = 0;
    private _fpsCurrentValue: number = 0;

    // Backend FPS counter state (frames received from server)
    private _serverFpsFrameCount: number = 0;
    private _serverFpsWindowStart: number = 0;
    private _serverFpsCurrentValue: number = 0;

    constructor(surfaceName: string, ctx: CanvasRenderingContext2D, canvas: HTMLCanvasElement) {
        this.surfaceName = surfaceName;
        this.ctx = ctx;
        this.canvas = canvas;
    }

    public forceRedraw() {
        this.needRedraw = true;
    }

    public connect() {
        if (this.webSocket) {
            console.warn('WebSocket already connected');
            return;
        }
        this.webSocket = SurfaceService.createWebSocket(this.surfaceName);
        this.webSocket.onopen = () => {
            this.isConnectedFlag = true;
            console.log('WebSocket connected');
        };
        this.webSocket.onmessage = (event) => {
            // Assuming the server sends JSON-encoded draw commands
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
        }
    }

    public isConnected(): boolean {
        return this.webSocket !== null && this.isConnectedFlag;
    }

    // -------------------------------------------------------------------------
    // Private helpers
    // -------------------------------------------------------------------------

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
            // focal point (fx,fy) → inner circle with radius 0; outer circle at (cx,cy) with radius r
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

    // -------------------------------------------------------------------------
    // Public draw entry-point
    // -------------------------------------------------------------------------

    public drawFrame() {

        const w = this.canvas.offsetWidth;
        const h = this.canvas.offsetHeight;

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