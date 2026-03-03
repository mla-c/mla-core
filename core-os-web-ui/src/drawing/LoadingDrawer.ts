export class LoadingDrawer {

    private static readonly green = '#27ae60';
    private static readonly trackColor = 'rgba(51, 51, 51, 0.15)';

    public static drawLoadingIndicator(ctx: CanvasRenderingContext2D, canvas: HTMLCanvasElement, label: string) {

        const w = canvas.width
        const h = canvas.height


        ctx.clearRect(0, 0, w, h);

        const cx = w / 2;
        const cy = h / 2;
        const radius = Math.min(w, h) * 0.06;
        const lineWidth = radius * 0.20;

        const angle = (performance.now() / 1000) * Math.PI * 2 * 0.5; // full rotation every 2 seconds

        // Track ring
        ctx.beginPath();
        ctx.arc(cx, cy, radius, 0, Math.PI * 2);
        ctx.strokeStyle = this.trackColor;
        ctx.lineWidth = lineWidth;
        ctx.lineCap = 'round';
        ctx.stroke();

        // Spinning arc — matches .pl-button.loading::after style
        ctx.beginPath();
        ctx.arc(cx, cy, radius, angle, angle + Math.PI * 1.4);
        ctx.strokeStyle = this.green;
        ctx.lineWidth = lineWidth;
        ctx.lineCap = 'round';
        ctx.stroke();

        // Optional: subtle label using Roboto-style font
        ctx.fillStyle = 'rgba(51, 51, 51, 0.4)';
        ctx.font = `300 ${Math.max(12, radius * 0.38)}px "Roboto", "Helvetica Neue", Arial, sans-serif`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(label + '…', cx, cy + radius + lineWidth * 2.5);
    }
}