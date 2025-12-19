import { defineConfig } from 'vite';
import preact from '@preact/preset-vite';
import singleFileCompression from 'vite-plugin-singlefile-compression'

// https://vitejs.dev/config/
export default defineConfig({
	plugins: [preact(), singleFileCompression()],
});
