import {defineConfig, PluginOption} from 'vite';
import preact from '@preact/preset-vite';
import singleFileCompression from 'vite-plugin-singlefile-compression'
import { viteSingleFile } from "vite-plugin-singlefile"
import { writeFileSync } from 'node:fs';
import { resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { gzipSync } from 'node:zlib';

const __dirname = dirname(fileURLToPath(import.meta.url));

function generateCHeader(compress: boolean = false): PluginOption {
	return {
		name: 'generate-c-header',
		writeBundle(options, bundle) {
			const htmlFile = Object.keys(bundle).find(key => key.endsWith('.html'));
			if (!htmlFile) return;

			const outputAsset = bundle[htmlFile];
			if (outputAsset.type !== 'asset') return;

			let content = outputAsset.source;
			let buffer = typeof content === 'string' ? Buffer.from(content) : Buffer.from(content);

			// Apply maximum gzip compression if enabled
			if (compress) {
				buffer = gzipSync(buffer, { level: 9 }); // Level 9 = maximum compression
			}

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
			header += `mla_size_t ${arrayName}_len = ${buffer.length};\n`;
			header += `const char* ${arrayName}_content_encoding = ${compress ? '"gzip"' : '""'};\n\n`;
			header += `#endif\n`;

			const outputPath = resolve(__dirname, '../core-os/ui/mla_ui_web_embedded.h');
			writeFileSync(outputPath, header);
			console.log(`Generated C header: ${outputPath}${compress ? ' (gzipped, level 9)' : ''}`);
		}
	};
}


const singleFileCompressed = singleFileCompression({
	compressFormat: "gzip",
});

const singleFile = viteSingleFile();

// https://vitejs.dev/config/
export default defineConfig({
	plugins: [preact(), singleFile, generateCHeader(true)],
});
