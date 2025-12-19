import {defineConfig, PluginOption} from 'vite';
import preact from '@preact/preset-vite';
import singleFileCompression from 'vite-plugin-singlefile-compression'
import { writeFileSync } from 'node:fs';
import { resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = dirname(fileURLToPath(import.meta.url));

function generateCHeader(): PluginOption {
	return {
		name: 'generate-c-header',
		writeBundle(options, bundle) {
			const htmlFile = Object.keys(bundle).find(key => key.endsWith('.html'));
			if (!htmlFile) return;

			const content = bundle[htmlFile].source;
			const buffer = Buffer.from(content);

			const arrayName = 'mla_ui_web_embedded_index_html';
			let header = `// Auto-generated file - do not edit\n`;
			header += `#ifndef COREOS_MLA_UI_WEB_EMBEDDED_H\n`;
			header += `#define COREOS_MLA_UI_WEB_EMBEDDED_H\n\n`;
			header += `mla_byte_t ${arrayName}[] = {\n`;

			for (let i = 0; i < buffer.length; i++) {
				if (i % 12 === 0) header += '  ';
				header += `0x${buffer[i].toString(16).padStart(2, '0')}`;
				if (i < buffer.length - 1) header += ',';
				if (i % 12 === 11 || i === buffer.length - 1) header += '\n';
				else header += ' ';
			}

			header += `};\n`;
			header += `mla_size_t ${arrayName}_len = ${buffer.length};\n\n`;
			header += `#endif\n`;

			const outputPath = resolve(__dirname, '../core-os/ui/mla_ui_web_embedded.h');
			writeFileSync(outputPath, header);
			console.log(`Generated C header: ${outputPath}`);
		}
	};
}

// https://vitejs.dev/config/
export default defineConfig({
	plugins: [preact(), singleFileCompression(), generateCHeader()],
});
