export interface Surface {
    surfaceName: string;
    displayName: string;
}

export interface Surfaces {
    surfaces: Surface[];
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