import {SurfaceService} from "../api/SurfaceService";

export class RemoteUIDrawer {

    private ctx: CanvasRenderingContext2D;
    private canvas: HTMLCanvasElement;
    private surfaceName: string;
    private webSocket: WebSocket | null = null;
    private isConnectedFlag: boolean = false;

    constructor(surfaceName: string, ctx: CanvasRenderingContext2D, canvas: HTMLCanvasElement) {
        this.surfaceName = surfaceName;
        this.ctx = ctx;
        this.canvas = canvas;
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
            // Handle incoming messages (e.g., UI updates)
            console.log('Received message:', event.data);
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


    public drawFrame() {

        const w = this.canvas.offsetWidth
        const h = this.canvas.offsetHeight

        this.ctx.clearRect(0, 0, w, h);


    }
}