export class Utils {

    public static isDefined(input: any) {
        return (input != null) && (input != undefined);
    }

    public static formatBytes(bytes?: number, decimals = 2): string {

        if (!Utils.isDefined(bytes)) {
            return '';
        }

        if (bytes === 0) return '0 Bytes';

        const k = 1024;
        const dm = decimals < 0 ? 0 : decimals;
        const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];

        const i = Math.floor(Math.log(bytes!) / Math.log(k));

        return parseFloat((bytes! / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
    }

    public static async sleep(ms: number) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    public static joinPath(path: string, dir: string) {
        if (path.endsWith('/')) {
            return path + dir;
        } else {
            return path + '/' + dir;
        }
    }

    public static getFileName(path: string): string {

        const seperatorIndex = path.lastIndexOf('/');

        if (seperatorIndex === -1) {
            return path;
        }

        return path.substring(seperatorIndex + 1);

    }

    public static async blobToBase64(blob: Blob) {

        return new Promise<string>((resolve, _) => {
            const reader = new FileReader();
            reader.onloadend = () => {

                const data = (reader.result as string).split(',')[1];
                resolve(data);

            };
            reader.readAsDataURL(blob);
        });
    }

    public static async selectFile(): Promise<File | undefined> {

        return new Promise<File | undefined>((resolve) => {
            const input = document.createElement("input");
            input.type = "file"
            input.onchange = () =>{
                resolve(input.files![0]);
            };
            input.click();
        });
    }

    public static base64ToArrayBuffer(p_Content: string): Uint8Array {

        const binary_string = window.atob(p_Content);
        const len = binary_string.length;
        const bytes = new Uint8Array(len);
        for (let i = 0; i < len; i++) {
            bytes[i] = binary_string.charCodeAt(i);
        }
        return bytes;
    }

    public static saveAs(blob: Blob, filename: string) {
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = filename;
        a.click();
        window.URL.revokeObjectURL(url);
    }
}
