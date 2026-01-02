const fileInput = document.getElementById("map-file");
const dropZone = document.getElementById("drop-zone");
const browseBtn = document.getElementById("browse-btn");
const analyzeBtn = document.getElementById("analyze-btn");
const downloadBtn = document.getElementById("download-btn");
const topInput = document.getElementById("top-n");
const statusEl = document.getElementById("status");
const reportOutput = document.getElementById("report-output");

let currentReport = "";
let currentFileName = "";
let currentFile = null;

const setStatus = (message, isError = false) => {
  statusEl.textContent = message;
  statusEl.style.color = isError ? "#b23624" : "";
};

const humanizeBytes = (value) => `${(value / 1024).toFixed(2)} KB`;

const isFullWidthCodePoint = (codePoint) => {
  return (
    codePoint >= 0x1100 &&
    (codePoint <= 0x115f ||
      codePoint === 0x2329 ||
      codePoint === 0x232a ||
      (codePoint >= 0x2e80 && codePoint <= 0xa4cf && codePoint !== 0x303f) ||
      (codePoint >= 0xac00 && codePoint <= 0xd7a3) ||
      (codePoint >= 0xf900 && codePoint <= 0xfaff) ||
      (codePoint >= 0xfe10 && codePoint <= 0xfe19) ||
      (codePoint >= 0xfe30 && codePoint <= 0xfe6f) ||
      (codePoint >= 0xff00 && codePoint <= 0xff60) ||
      (codePoint >= 0xffe0 && codePoint <= 0xffe6) ||
      (codePoint >= 0x1f300 && codePoint <= 0x1f64f) ||
      (codePoint >= 0x1f900 && codePoint <= 0x1f9ff) ||
      (codePoint >= 0x20000 && codePoint <= 0x3fffd))
  );
};

const stringWidth = (value) => {
  let width = 0;
  for (const char of String(value)) {
    const codePoint = char.codePointAt(0);
    width += isFullWidthCodePoint(codePoint) ? 2 : 1;
  }
  return width;
};

const pad = (value, width, alignRight = true) => {
  const text = String(value);
  const textWidth = stringWidth(text);
  if (textWidth >= width) {
    return text;
  }
  const padSize = width - textWidth;
  const padding = " ".repeat(padSize);
  return alignRight ? `${padding}${text}` : `${text}${padding}`;
};

const parseComponentTable = (text) => {
  const lines = text.split(/\r?\n/);
  const startIndex = lines.findIndex((line) =>
    line.includes("Image component sizes")
  );
  if (startIndex === -1) {
    throw new Error("未找到 'Image component sizes' 段落，请确认 map 文件。");
  }

  const entries = [];
  for (let i = startIndex + 4; i < lines.length; i += 1) {
    const line = lines[i];
    const trimmed = line.trim();
    if (!trimmed) {
      continue;
    }
    if (trimmed.startsWith("-")) {
      break;
    }

    const tokens = trimmed.split(/\s+/);
    if (tokens.length < 2) {
      continue;
    }

    const name = tokens[tokens.length - 1];
    if (!name.endsWith(".o")) {
      continue;
    }

    const numberTokens = tokens.slice(0, -1);
    if (!numberTokens.every((token) => /^-?\d+$/.test(token))) {
      continue;
    }

    const numbers = numberTokens.map((token) => Number.parseInt(token, 10));
    let code = 0;
    let ro = 0;
    let rw = 0;
    let zi = 0;
    let debug = 0;
    let extra = 0;

    if (numbers.length === 6) {
      [code, ro, rw, extra, zi, debug] = numbers;
    } else if (numbers.length === 5) {
      [code, ro, rw, zi, debug] = numbers;
    } else {
      continue;
    }

    entries.push({
      object: name,
      code,
      ro,
      rw,
      zi,
      debug,
      extra,
      flash: code + ro + rw,
      ram: rw + zi,
    });
  }

  if (!entries.length) {
    throw new Error("未解析到对象条目，请检查 map 文件格式。");
  }

  return entries;
};

const buildTable = (entries, sortKey, title, count, valueLabel) => {
  const sorted = [...entries].sort((a, b) => b[sortKey] - a[sortKey]);
  const safeCount = count <= 0 || count > sorted.length ? sorted.length : count;
  const lines = [
    title,
    `${pad("排行", 4)}  ${pad("对象", 35, false)}  ${pad("Code", 8)}  ${pad(
      "RO",
      8
    )}  ${pad("RW", 8)}  ${pad("ZI", 8)}  ${pad(
      valueLabel,
      12
    )}  ${pad("Size", 10)}`,
  ];

  for (let i = 0; i < safeCount; i += 1) {
    const entry = sorted[i];
    const objectName =
      entry.object.length > 35
        ? `${entry.object.slice(0, 32)}...`
        : entry.object;
    lines.push(
      `${pad(i + 1, 4)}  ${pad(objectName, 35, false)}  ${pad(
        entry.code,
        8
      )}  ${pad(entry.ro, 8)}  ${pad(entry.rw, 8)}  ${pad(
        entry.zi,
        8
      )}  ${pad(entry[sortKey], 12)}  ${pad(
        humanizeBytes(entry[sortKey]),
        10
      )}`
    );
  }

  const remaining = sorted.slice(safeCount).reduce((sum, entry) => {
    return sum + entry[sortKey];
  }, 0);
  const remainingObjects = Math.max(0, sorted.length - safeCount);
  lines.push(
    `...  剩余 ${remainingObjects} 个对象合计 ${remaining} 字节 (${humanizeBytes(
      remaining
    )})`
  );
  lines.push("");
  return lines;
};

const renderReport = (entries, topN, fileName) => {
  const totalFlash = entries.reduce((sum, entry) => sum + entry.flash, 0);
  const totalRam = entries.reduce((sum, entry) => sum + entry.ram, 0);

  const reportLines = [
    `RocketPi 内存使用报告（来源：${fileName || "map 文件"}）`,
    `解析对象数: ${entries.length}`,
    `总 Flash（Code + RO + RW）: ${totalFlash} 字节 (${humanizeBytes(
      totalFlash
    )})`,
    `总 RAM（RW + ZI）: ${totalRam} 字节 (${humanizeBytes(totalRam)})`,
    "列说明: Code=代码, RO=只读, RW=读写, ZI=零初始化, Flash 字节=Code+RO+RW, Size=KB",
    "",
    "按 Flash 占用排序：",
  ];
  reportLines.push(
    ...buildTable(entries, "flash", "Flash 重点", topN, "Flash 字节")
  );
  reportLines.push("按 RAM 占用排序：");
  reportLines.push(
    ...buildTable(entries, "ram", "RAM 重点", topN, "RAM 字节")
  );
  return reportLines.join("\n");
};

const handleReport = (entries, fileName, topN, file) => {
  currentReport = renderReport(entries, topN, fileName);
  reportOutput.textContent = currentReport;
  currentFileName = fileName || "memory_usage_report";
  downloadBtn.disabled = false;
};

const analyzeFile = async (file) => {
  if (!file) {
    setStatus("请先选择一个 map 文件。", true);
    return;
  }

  try {
    const text = await file.text();
    const entries = parseComponentTable(text);
    const rawTop = Number.parseInt(topInput.value, 10);
    const topN = Number.isNaN(rawTop) ? 0 : rawTop;
    currentFile = file;
    handleReport(entries, file.name, topN, file);
    setStatus(`已从 ${file.name} 解析 ${entries.length} 个对象。`);
  } catch (error) {
    setStatus(error.message, true);
    reportOutput.textContent = "解析失败，请检查 map 文件格式。";
    downloadBtn.disabled = true;
  }
};

const activateBrowse = () => fileInput.click();

browseBtn.addEventListener("click", activateBrowse);
dropZone.addEventListener("click", activateBrowse);

fileInput.addEventListener("change", () => {
  const file = fileInput.files[0];
  if (file) {
    analyzeFile(file);
  }
});

analyzeBtn.addEventListener("click", () => {
  const file = fileInput.files[0] || currentFile;
  analyzeFile(file);
});

downloadBtn.addEventListener("click", () => {
  if (!currentReport) {
    return;
  }
  const baseName = currentFileName.replace(/\.[^/.]+$/, "");
  const outputName = baseName
    ? `${baseName}_memory_report.txt`
    : "memory_usage_report.txt";
  const blob = new Blob([currentReport], { type: "text/plain" });
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");
  link.href = url;
  link.download = outputName;
  document.body.appendChild(link);
  link.click();
  link.remove();
  URL.revokeObjectURL(url);
});

const preventDefaults = (event) => {
  event.preventDefault();
  event.stopPropagation();
};

["dragenter", "dragover", "dragleave", "drop"].forEach((eventName) => {
  dropZone.addEventListener(eventName, preventDefaults, false);
});

["dragenter", "dragover"].forEach((eventName) => {
  dropZone.addEventListener(
    eventName,
    () => dropZone.classList.add("is-dragover"),
    false
  );
});

["dragleave", "drop"].forEach((eventName) => {
  dropZone.addEventListener(
    eventName,
    () => dropZone.classList.remove("is-dragover"),
    false
  );
});

dropZone.addEventListener("drop", (event) => {
  const file = event.dataTransfer.files[0];
  if (file) {
    analyzeFile(file);
  }
});
