/**
 * Config file for vite
 * Also archives and compresses generated files with a custom FGZ format.
 */
import fs from "fs";
import path from "path";
import { defineConfig } from "vite";
import { gzip } from "node-gzip";

class FFile {
  constructor(file, data) {
    this.offset = 0;
    this.buffer = new ArrayBuffer(8 + file.length + data.byteLength);
    this.writeSize(file.length);
    this.writeData(Buffer.from(file));
    this.writeSize(data.byteLength);
    this.writeData(data);
  }
  writeSize(size) {
    new DataView(this.buffer, this.offset).setUint32(0, size, false);
    this.offset += 4;
  }
  writeData(data) {
    new Int8Array(this.buffer, this.offset).set(data);
    this.offset += data.byteLength;
  }
}

class FGZip {
  constructor() {
    this.files = [];
    this.offset = 0;
  }
  append(file, data) {
    this.files.push(new FFile(file, data));
  }
  async finish() {
    return await gzip(await new Blob(this.files.map(file => file.buffer)).arrayBuffer());
  }
}

const archive = () => {
  return {
    name: "archive",
    apply: "build",
    closeBundle: async () => {
      if (!process.env.hasOwnProperty('ARCHIVE_OUT'))
        throw new Error("Missing ARCHIVE_OUT");
      const out_file = process.env.ARCHIVE_OUT;
      const out_dir = path.dirname(out_file);
      console.log(`Packaging ${out_file}`);
      const fgz = new FGZip();
      fs.readdirSync("dist")
        .forEach((file) => {
          const data = fs.readFileSync(path.join("dist", file));
          fgz.append(file, data);
        });
      const data = await fgz.finish();
      if (!fs.existsSync(out_dir)) {
        fs.mkdirSync(out_dir);
      }
      fs.writeFileSync(out_file, data);
    }
  };
};

export default defineConfig({
  build: {
    sourcemap: false,
    rollupOptions: {
      output: {
        entryFileNames: "[name].js",
        chunkFileNames: "[name].js",
        assetFileNames: "[name].[ext]",
      }
    },
  },
  plugins: [archive()]
});
