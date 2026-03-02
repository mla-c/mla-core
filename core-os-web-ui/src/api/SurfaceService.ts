export interface Surface {
    surfaceName: string;
    displayName: string;
}

export interface Surfaces {
    surfaces: Surface[];
}

export interface Color {
    r: number;
    g: number;
    b: number;
    a: number;
}

export interface Point {
    x: number;
    y: number;
}

export interface Size {
    width: number;
    height: number;
}

export interface FontType {
    family: string;
    size: number;
    bold: boolean;
    italic: boolean;
}

export interface InputStates {
    cursorPosition: Point;
    leftMouseButtonDown: boolean;
    rightMouseButtonDown: boolean;
    middleMouseButtonDown: boolean;
    shiftKeyDown: boolean;
    ctrlKeyDown: boolean;
    altKeyDown: boolean;
    metaKeyDown: boolean;
    keyCodeDown: number;
}

export enum DrawCommandKind {
    None = 0,
    Rect = 1,
    Circle = 2,
    Ellipse = 3,
    Line = 4,
    Polyline = 5,
    Polygon = 6,
    Path = 7,
    Text = 8,
    LinearGradient = 9,
    RadialGradient = 10,
    Stop = 11,
}

export interface DrawCommandRect {
    x: number;
    y: number;
    width: number;
    height: number;
    rx: number;
    ry: number;
    color: Color;stroke: Color;
    stroke_width: number;
}

export interface DrawCommandCircle {
    cx: number;
    cy: number;
    r: number;
    fill: Color;
    stroke: Color;
    stroke_width: number;
}

export interface DrawCommandEllipse {
    cx: number;
    cy: number;
    rx: number;
    ry: number;
    fill: Color;
    stroke: Color;
    stroke_width: number;
}

export interface DrawCommandLine {
    x1: number;
    y1: number;
    x2: number;
    y2: number;
    stroke: Color;
    stroke_width: number;
}

export interface DrawCommandPolyline {
    points: Point[];
    fill: Color;
    stroke: Color;
    stroke_width: number;
}

export interface DrawCommandPolygon {
    points: Point[];
    fill: Color;
    stroke: Color;
    stroke_width: number;
}

export enum PathCommandKind {
    MoveTo = 0,
    LineTo = 1,
    QuadraticCurveTo = 2,
    CubicCurveTo = 3,
    ArcTo = 4,
    ClosePath = 5,
}

export interface PathCommandMoveTo {
    x: number;
    y: number;
}

export interface PathCommandLineTo {
    x: number;
    y: number;
}

export interface PathCommandQuadraticCurveTo {
    cpx: number;
    cpy: number;
    x: number;
    y: number;
}

export interface PathCommandCubicCurveTo {
    cp1x: number;
    cp1y: number;
    cp2x: number;
    cp2y: number;
    x: number;
    y: number;
}

export interface PathCommandArcTo {
    rx: number;
    ry: number;
    x_axis_rotation: number;
    large_arc_flag: boolean;
    sweep_flag: boolean;
    x: number;
    y: number;
}

export interface PathCommand {
    kind: PathCommandKind;
    moveTo?: PathCommandMoveTo;
    lineTo?: PathCommandLineTo;
    quadraticCurveTo?: PathCommandQuadraticCurveTo;
    cubicCurveTo?: PathCommandCubicCurveTo;
    arcTo?: PathCommandArcTo;
}

export interface DrawCommandPath {
    commands: PathCommand[];
    fill: Color;
    stroke: Color;
    stroke_width: number;
}

export interface DrawCommandText {
    x: number;
    y: number;
    content: string;
    font_type: FontType;
    fill: Color;
}

export interface DrawCommandLinearGradient {
    x1: number;
    y1: number;
    x2: number;
    y2: number;
}

export interface DrawCommandRadialGradient {
    cx: number;
    cy: number;
    r: number;
    fx: number;
    fy: number;
}

export interface DrawCommandStop {
    offset: number;
    stop_color: Color;
    stop_opacity: number;
}

export interface DrawCommand {
    kind: DrawCommandKind;
    rect?: DrawCommandRect;
    circle?: DrawCommandCircle;
    ellipse?: DrawCommandEllipse;
    line?: DrawCommandLine;
    polyline?: DrawCommandPolyline;
    polygon?: DrawCommandPolygon;
    path?: DrawCommandPath;
    text?: DrawCommandText;
    linearGradient?: DrawCommandLinearGradient;
    radialGradient?: DrawCommandRadialGradient;
    stop?: DrawCommandStop;
}

export interface DrawCommands {
    drawCommands: DrawCommand[];
}

export enum InputEventClickButton {
    None = 0,
    Left = 1,
    Right = 2,
    Middle = 3,
}

export interface InputEventClick {
    position: Point;
    button: InputEventClickButton;
}

export enum InputEventCharInputKind {
    CharInput = 0,
    Enter = 1,
    Backspace = 2,
    Delete = 3,
    Tab = 4,
    Escape = 5,
    ArrowUp = 6,
    ArrowDown = 7,
    ArrowLeft = 8,
    ArrowRight = 9,
}

export enum InputEventControlCharKind {
    None = 0,
    Shift = 1,
    Ctrl = 2,
    Alt = 3,
}

export interface InputEventCharInput {
    kind: InputEventCharInputKind;
    character0: number;
    character1: number;character2: number;
    character3: number;
    pressedControlKeys: InputEventControlCharKind;
}

export enum InputEventKind {
    None = 0,
    Click = 1,
    Char = 2,
}

export interface InputEvent {
    kind: InputEventKind;
    click?: InputEventClick;
    charInput?: InputEventCharInput;
}

export interface ClientTextSizeEntry {
    fontType: FontType;
    size_per_char: Size[];
}

export interface ClientMessage {
    surface_size: Size;
    inputEvents: InputEvent[];
    inputStates: InputStates;
    textSize: ClientTextSizeEntry;
}


export class SurfaceService {
    //static BASE_URL = './';
    static BASE_URL = 'http://localhost:8081/ui/';

    static async getSurfaces(): Promise<Surfaces> {
        const response = await fetch(this.BASE_URL+ 'surfaces.json');
        if (!response.ok) {
            throw new Error(`Failed to fetch surfaces with status ${response.status}`);
        }
        const data = await response.json();
        return data as Surfaces;
    }

    static createWebSocket(surfaceName: string): WebSocket {
        const wsUrl = this.BASE_URL.replace(/^http/, 'ws') + `../ws/surface/${surfaceName}`;
        return new WebSocket(wsUrl);
    }


}