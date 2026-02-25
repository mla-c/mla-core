import './SurfaceUIPage.css'
import {Surface} from "../api/SurfaceService";
import {useEffect, useRef} from "preact/hooks";
import {LoadingDrawer} from "../drawing/LoadingDrawer";
import {RemoteUIDrawer} from "../drawing/RemoteUIDrawer";

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

        const resize = () => {
            const dpr = window.devicePixelRatio || 1;
            canvas.width = canvas.offsetWidth * dpr;
            canvas.height = canvas.offsetHeight * dpr;
            ctx.scale(dpr, dpr);
        };

        resize();
        window.addEventListener('resize', resize);

        const remoteUIDrawer = new RemoteUIDrawer(surface.surfaceName, ctx, canvas);
        remoteUIDrawer.connect();

        const draw = () => {

            if (remoteUIDrawer.isConnected()) {
                remoteUIDrawer.drawFrame();
            } else {
                LoadingDrawer.drawLoadingIndicator(ctx, canvas, 'Loading');
            }

            animationFrameId = requestAnimationFrame(draw);
        };

        draw();

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