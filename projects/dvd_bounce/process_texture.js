// usage: bun run script.js image.png

import fs from "fs";
import { PNG } from "pngjs";

const filePath = process.argv[2];

if (!filePath) {
  console.error("Usage: bun run script.js <image.png>");
  process.exit(1);
}

// read file
const buffer = fs.readFileSync(filePath);

// decode PNG
const png = PNG.sync.read(buffer);

// pixel data is RGBA packed: [R,G,B,A,R,G,B,A,...]
const data = png.data;

console.log(`Texture TEX_IMG = Texture(${png.width}, ${png.height}, {`);
for (let i = 0; i < data.length; i += 4) {
  const r = data[i];
  const g = data[i + 1];
  const b = data[i + 2];
  const a = data[i + 3];

  console.log(`    Colour::FromRgb(${r},${g},${b},${a}),`);
}
console.log("});");