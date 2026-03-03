import './SurfaceUIPage.css'
import {Surface} from "../api/SurfaceService";
import {useEffect, useRef} from "preact/hooks";
import {LoadingDrawer} from "../drawing/LoadingDrawer";
import {RemoteUIDrawer} from "../drawing/RemoteUIDrawer";
import {Utils} from "../utils/Utils";

type SurfaceUIPageProps = {
    surface: Surface
}

export default function SurfaceUIPage({surface}: SurfaceUIPageProps) {

    const canvasRef = useRef<HTMLCanvasElement>(null);

    useEffect(() => {

        const canvas = canvasRef.current;
        if (!canvas)
            return;

        const ctx = canvas.getContext('2d');
        if (!ctx)
            return;


        let animationFrameId: number;

        const remoteUIDrawer = new RemoteUIDrawer(surface.surfaceName, ctx, canvas);

        const resize = () => {
            const dpr = window.devicePixelRatio || 1;

            // Round the DPI-scaled size to integers so comparisons with
            // canvas.width / canvas.height (which are integers) are consistent.
            const newWidth = Math.round(canvas.offsetWidth * dpr);
            const newHeight = Math.round(canvas.offsetHeight * dpr);
            if (canvas.width !== newWidth || canvas.height !== newHeight) {
                canvas.width = newWidth;
                canvas.height = newHeight;
                remoteUIDrawer.forceRedraw();
            }
        };

        const resizeDebounced = Utils.debounce(resize, 100);

        // Keep the resize listener so the canvas updates immediately on window resize
        window.addEventListener('resize', resizeDebounced);

        remoteUIDrawer.connect();


        const draw = () => {

            if (remoteUIDrawer.isConnected()) {
                resize();
                remoteUIDrawer.drawFrame();
            } else {
                LoadingDrawer.drawLoadingIndicator(ctx, canvas, 'Loading');
            }

            animationFrameId = requestAnimationFrame(draw);
        };

        LoadingDrawer.drawLoadingIndicator(ctx, canvas, 'Loading');
        animationFrameId = requestAnimationFrame(draw);

        return () => {
            cancelAnimationFrame(animationFrameId);
            window.removeEventListener('resize', resize);
            remoteUIDrawer.disconnect();
        };
    }, []);

    return (<canvas
        ref={canvasRef}
        class="surface-layout">

    </canvas>)


}