"use strict";

// Small dependency-free JSON-RPC/LSP server. It compiles documents with
// `indk --dis` (compile-only), never executes user programs.
const fs = require("fs");
const os = require("os");
const path = require("path");
const { spawn } = require("child_process");
const { fileURLToPath } = require("url");
const extensionPackage = require("./package.json");
const { KEYWORDS, BUILTINS, MAGIC_METHODS, DESCRIPTIONS } = require("./language-data");

const MAX_MESSAGE_BYTES = 2 * 1024 * 1024;
const MAX_DOCUMENT_BYTES = 512 * 1024;
const COMPILE_TIMEOUT_MS = 5000;
const documents = new Map();
const timers = new Map();
const compilers = new Map();
let input = Buffer.alloc(0);
let nextTempId = 1;
let shuttingDown = false;
let workspaceRoot = process.cwd();
const indk = process.env.INDONESIAKU_INDK || (process.platform === "win32" ? "indk.exe" : "indk");

function log(message) {
  process.stderr.write(`[indk-lsp] ${message}\n`);
}

function send(message) {
  const body = Buffer.from(JSON.stringify(message), "utf8");
  process.stdout.write(`Content-Length: ${body.length}\r\n\r\n`);
  process.stdout.write(body);
}

function respond(id, result) {
  send({ jsonrpc: "2.0", id, result });
}

function respondError(id, code, message) {
  send({ jsonrpc: "2.0", id, error: { code, message } });
}

function parseFrames() {
  for (;;) {
    const headerEnd = input.indexOf("\r\n\r\n");
    if (headerEnd < 0) {
      if (input.length > 8192) {
        log("discarding oversized LSP header");
        process.exitCode = 1;
        process.stdin.pause();
      }
      return;
    }
    const headers = input.subarray(0, headerEnd).toString("ascii");
    const match = headers.match(/(?:^|\r\n)Content-Length:\s*(\d+)/i);
    if (!match) {
      log("discarding malformed message without Content-Length");
      input = input.subarray(headerEnd + 4);
      continue;
    }
    const length = Number(match[1]);
    if (!Number.isSafeInteger(length) || length < 0 || length > MAX_MESSAGE_BYTES) {
      log(`discarding message with invalid length ${length}`);
      process.exitCode = 1;
      process.stdin.pause();
      return;
    }
    const bodyStart = headerEnd + 4;
    if (input.length < bodyStart + length) return;
    const body = input.subarray(bodyStart, bodyStart + length).toString("utf8");
    input = input.subarray(bodyStart + length);
    try {
      handleMessage(JSON.parse(body));
    } catch (error) {
      log(`message handling failed: ${error.stack || error}`);
    }
  }
}

function handleMessage(message) {
  if (!message || message.jsonrpc !== "2.0") return;
  const method = message.method;
  const params = message.params || {};

  if (method === "initialize") {
    try {
      if (params.rootUri && params.rootUri.startsWith("file:")) {
        workspaceRoot = fileURLToPath(params.rootUri);
      }
    } catch (_) { /* retain process cwd for non-file workspaces */ }
    respond(message.id, {
      capabilities: {
        textDocumentSync: { openClose: true, change: 1 },
        completionProvider: { resolveProvider: false, triggerCharacters: ["."] },
        hoverProvider: true,
        definitionProvider: true
      },
      serverInfo: { name: "indk-lsp", version: extensionPackage.version }
    });
    return;
  }
  if (method === "initialized") return;
  if (method === "shutdown") {
    shuttingDown = true;
    respond(message.id, null);
    return;
  }
  if (method === "exit") {
    process.exit(shuttingDown ? 0 : 1);
    return;
  }
  if (method === "$/cancelRequest") return;

  if (method === "textDocument/didOpen") {
    const doc = params.textDocument;
    if (doc && typeof doc.uri === "string" && typeof doc.text === "string") {
      documents.set(doc.uri, { uri: doc.uri, version: doc.version || 0, text: doc.text });
      scheduleDiagnostics(doc.uri);
    }
    return;
  }
  if (method === "textDocument/didChange") {
    const id = params.textDocument && params.textDocument.uri;
    const doc = documents.get(id);
    const changes = params.contentChanges;
    if (doc && Array.isArray(changes) && changes.length && typeof changes[changes.length - 1].text === "string") {
      doc.text = changes[changes.length - 1].text;
      doc.version = params.textDocument.version || doc.version;
      scheduleDiagnostics(id);
    }
    return;
  }
  if (method === "textDocument/didClose") {
    const id = params.textDocument && params.textDocument.uri;
    documents.delete(id);
    clearTimeout(timers.get(id));
    timers.delete(id);
    cancelCompile(id);
    send({ jsonrpc: "2.0", method: "textDocument/publishDiagnostics", params: { uri: id, diagnostics: [] } });
    return;
  }

  if (method === "textDocument/completion") {
    const doc = documents.get(params.textDocument && params.textDocument.uri);
    const symbols = doc ? collectDefinitions(doc.text).map(symbol => symbol.name) : [];
    const labels = new Set([...KEYWORDS, ...BUILTINS, ...MAGIC_METHODS]);
    const items = [
      ...KEYWORDS.map(label => ({ label, kind: 14, detail: "kata kunci Indonesiaku" })),
      ...BUILTINS.map(label => ({ label, kind: 3, detail: "fungsi bawaan Indonesiaku", insertText: `${label}($0)` })),
      ...MAGIC_METHODS.map(label => ({ label, kind: 2, detail: "metode ajaib Indonesiaku" })),
      ...[...new Set(symbols)].filter(label => !labels.has(label))
        .map(label => ({ label, kind: 6, detail: "simbol dalam dokumen" }))
    ];
    respond(message.id, { isIncomplete: false, items });
    return;
  }
  if (method === "textDocument/hover") {
    const doc = documents.get(params.textDocument && params.textDocument.uri);
    const word = doc && wordAt(doc.text, params.position);
    const description = word && (DESCRIPTIONS[word] || (BUILTINS.includes(word) ? `Fungsi bawaan ${word} di Indonesiaku.` : null));
    respond(message.id, description ? { contents: { kind: "markdown", value: `**${word}**\n\n${description}` } } : null);
    return;
  }
  if (method === "textDocument/definition") {
    const doc = documents.get(params.textDocument && params.textDocument.uri);
    const word = doc && wordAt(doc.text, params.position);
    const definition = doc && word && findDefinition(doc, word, params.position);
    respond(message.id, definition || null);
    return;
  }
  if (method === "textDocument/references") {
    respond(message.id, []);
    return;
  }

  if (Object.prototype.hasOwnProperty.call(message, "id")) {
    respondError(message.id, -32601, `Method not found: ${method}`);
  }
}

function scheduleDiagnostics(uri) {
  clearTimeout(timers.get(uri));
  cancelCompile(uri);
  timers.set(uri, setTimeout(async () => {
    timers.delete(uri);
    const doc = documents.get(uri);
    if (doc) await publishDiagnostics(doc);
  }, 80));
}

function cancelCompile(uri) {
  const active = compilers.get(uri);
  if (active) {
    active.cancelled = true;
    active.child.kill();
    compilers.delete(uri);
  }
}

async function publishDiagnostics(doc) {
  const diagnostics = await compileDiagnostics(doc);
  const current = documents.get(doc.uri);
  // Drop stale results if a newer edit arrived while indk was compiling.
  if (!current || current.version !== doc.version) return;
  send({
    jsonrpc: "2.0",
    method: "textDocument/publishDiagnostics",
    params: { uri: doc.uri, version: doc.version, diagnostics }
  });
}

async function compileDiagnostics(doc) {
  if (Buffer.byteLength(doc.text, "utf8") > MAX_DOCUMENT_BYTES) {
    return [diagnostic(0, 0, "Dokumen terlalu besar untuk dianalisis (batas 512 KiB).")];
  }

  let filePath;
  try {
    let sourcePath;
    try {
      sourcePath = fileURLToPath(doc.uri);
    } catch (_) {
      // Untitled documents have no file URI. Place their temporary compiler
      // input under the workspace root so relative imports still resolve.
      sourcePath = path.join(workspaceRoot, "untitled.idk");
    }
    const dir = path.dirname(sourcePath);
    // Sibling temp preserves relative-import lookup. A non-.idk suffix keeps
    // VS Code from treating the temporary file as another source document.
    filePath = path.join(dir, `.indk-lsp-${process.pid}-${nextTempId++}.tmp`);
    fs.writeFileSync(filePath, doc.text, "utf8");
  } catch (error) {
    log(`cannot create compiler input for ${doc.uri}: ${error.message}`);
    return [diagnostic(0, 0, `Tidak dapat menyiapkan analisis dokumen: ${error.message}`)];
  }

  const state = { child: null, cancelled: false };
  compilers.set(doc.uri, state);
  try {
    const compile = await runIndk(["--dis", filePath], filePath, state);
    if (state.cancelled) return [];
    if (compile.error) {
      log(`compiler invocation failed: ${compile.error.message}`);
      return [diagnostic(0, 0, `Tidak dapat menjalankan '${indk} --dis': ${compile.error.message}`)];
    }
    if (compile.timedOut) return [diagnostic(0, 0, "Analisis sintaks melewati batas waktu 5 detik.")];

    const diagnostics = parseCompilerDiagnostics(compile.stderr, doc.text);
    if (compile.code !== 0) return diagnostics;

    // `periksa` shares the compiler's static scope analysis and never runs the
    // source. Surface its definite findings as LSP warnings.
    const lint = await runIndk(["periksa", filePath], filePath, state);
    if (state.cancelled) return [];
    if (lint.error) {
      log(`linter invocation failed: ${lint.error.message}`);
      return diagnostics;
    }
    if (lint.timedOut) return diagnostics;
    return diagnostics.concat(parseLintDiagnostics(lint.stderr, doc.text));
  } finally {
    if (compilers.get(doc.uri) === state) compilers.delete(doc.uri);
    try { fs.unlinkSync(filePath); } catch (_) { /* already removed */ }
  }
}

function runIndk(args, cwd, state) {
  return new Promise(resolve => {
    let stderr = "";
    let outputBytes = 0;
    let settled = false;
    const child = spawn(indk, args, {
      cwd: path.dirname(cwd),
      windowsHide: true,
      stdio: ["ignore", "pipe", "pipe"]
    });
    state.child = child;
    let timedOut = false;
    const finish = result => {
      if (settled) return;
      settled = true;
      clearTimeout(timeout);
      resolve({ ...result, stderr, timedOut });
    };
    child.stdout.on("data", chunk => {
      outputBytes += chunk.length;
      if (outputBytes > 1024 * 1024) child.kill();
    });
    child.stderr.on("data", chunk => {
      if (stderr.length < 1024 * 1024) stderr += chunk.toString("utf8");
      if (stderr.length >= 1024 * 1024) child.kill();
    });
    child.on("error", error => finish({ error, code: -1 }));
    child.on("close", code => finish({ code }));
    const timeout = setTimeout(() => {
      timedOut = true;
      child.kill();
    }, COMPILE_TIMEOUT_MS);
  });
}

function diagnostic(line, character, message, severity = 1, endCharacter = character + 1) {
  return {
    range: { start: { line, character }, end: { line, character: Math.max(character, endCharacter) } },
    severity,
    source: "indk",
    message
  };
}

function parseCompilerDiagnostics(stderr, source) {
  const lines = source.split("\n");
  const result = [];
  for (const line of stderr.split(/\r?\n/)) {
    const match = line.match(/^\[baris (\d+)(?:, kolom (\d+))?\] Kesalahan(?: di .*?)?: (.*)$/);
    if (!match) continue;
    const sourceLine = Math.max(0, Math.min(lines.length - 1, Number(match[1]) - 1));
    const byteColumn = Math.max(0, Number(match[2] || 1) - 1);
    const character = byteColumnToUtf16(lines[sourceLine], byteColumn);
    result.push(diagnostic(sourceLine, character, match[3], 1,
      Math.min(lines[sourceLine].length, character + 1)));
  }
  // Keep the error actionable if compiler output format changes unexpectedly.
  if (!result.length && stderr.trim()) result.push(diagnostic(0, 0, stderr.trim().split(/\r?\n/)[0]));
  return result;
}

function parseLintDiagnostics(stderr, source) {
  const lines = source.split("\n");
  const diagnostics = [];
  for (const line of stderr.split(/\r?\n/)) {
    const match = line.match(/^.*:(\d+)\s+variabel lokal '([^']+)' tidak pernah dipakai$/);
    if (!match) continue;
    const sourceLine = Math.max(0, Math.min(lines.length - 1, Number(match[1]) - 1));
    const found = lines[sourceLine].indexOf(match[2]);
    const start = Math.max(0, found);
    diagnostics.push(diagnostic(sourceLine, start,
      `Variabel lokal '${match[2]}' tidak pernah dipakai.`, 2,
      Math.min(lines[sourceLine].length, start + match[2].length)));
  }
  return diagnostics;
}

function byteColumnToUtf16(line, byteColumn) {
  let bytes = 0;
  let utf16 = 0;
  for (const codePoint of line) {
    if (bytes >= byteColumn) break;
    bytes += Buffer.byteLength(codePoint, "utf8");
    utf16 += codePoint.length;
  }
  return utf16;
}

function positionToOffset(text, position) {
  const lines = text.split("\n");
  const targetLine = Math.max(0, Math.min(lines.length - 1, position.line || 0));
  let offset = 0;
  for (let i = 0; i < targetLine; i++) offset += lines[i].length + 1;
  return offset + Math.min(lines[targetLine].length, Math.max(0, position.character || 0));
}

function wordAt(text, position) {
  const offset = positionToOffset(text, position);
  let start = offset;
  let end = offset;
  const isIdentifier = c => Boolean(c && /[A-Za-z0-9_]/.test(c));
  while (start > 0 && isIdentifier(text[start - 1])) start--;
  while (end < text.length && isIdentifier(text[end])) end++;
  const word = text.slice(start, end);
  return /^[A-Za-z_][A-Za-z0-9_]*$/.test(word) ? word : null;
}

function maskCommentsAndStrings(text) {
  const chars = text.split("");
  let quote = "";
  let triple = false;
  let comment = false;
  for (let i = 0; i < text.length; i++) {
    const c = text[i];
    if (comment) {
      if (c === "\n") comment = false;
      else chars[i] = " ";
      continue;
    }
    if (quote) {
      if (c !== "\n") chars[i] = " ";
      if (c === "\\" && i + 1 < text.length) {
        if (text[i + 1] !== "\n") chars[i + 1] = " ";
        i++;
        continue;
      }
      if (triple && text.slice(i, i + 3) === quote.repeat(3)) {
        chars[i] = chars[i + 1] = chars[i + 2] = " ";
        i += 2;
        quote = "";
        triple = false;
      } else if (!triple && c === quote) {
        quote = "";
      }
      continue;
    }
    if (c === "#") {
      comment = true;
      chars[i] = " ";
    } else if ((c === "\"" || c === "'") && text.slice(i, i + 3) === c.repeat(3)) {
      quote = c;
      triple = true;
      chars[i] = chars[i + 1] = chars[i + 2] = " ";
      i += 2;
    } else if (c === "\"" || c === "'") {
      quote = c;
      chars[i] = " ";
    }
  }
  return chars.join("");
}

function findDefinition(doc, name, position) {
  const defs = collectDefinitions(doc.text).filter(item => item.name === name);
  if (!defs.length) return null;
  const useOffset = positionToOffset(doc.text, position);
  const def = defs.filter(item => item.start <= useOffset).pop() || defs[0];
  const start = offsetToPosition(doc.text, def.start);
  const end = offsetToPosition(doc.text, def.end);
  return { uri: doc.uri, range: { start, end } };
}

function collectDefinitions(text) {
  const masked = maskCommentsAndStrings(text);
  const tokens = [];
  const re = /[A-Za-z_][A-Za-z0-9_]*/g;
  let match;
  while ((match = re.exec(masked))) tokens.push({ name: match[0], start: match.index, end: re.lastIndex });
  const keywordDefs = new Set(["fungsi", "kelas", "variabel", "global", "nonlokal"]);
  const defs = [];
  for (let i = 0; i < tokens.length; i++) {
    const token = tokens[i];
    const previous = tokens[i - 1];
    let declaration = previous && keywordDefs.has(previous.name);
    if (!declaration) {
      const tail = masked.slice(token.end).match(/^\s*(?:\*\*=|\/\/=|<<=|>>=|[+\-*/%&|^]=|=(?!=))/);
      declaration = Boolean(tail);
    }
    if (declaration) defs.push(token);
  }
  return defs;
}

function offsetToPosition(text, offset) {
  const before = text.slice(0, offset);
  const lines = before.split("\n");
  return { line: lines.length - 1, character: lines[lines.length - 1].length };
}

process.stdin.on("data", chunk => {
  input = Buffer.concat([input, chunk]);
  parseFrames();
});
process.stdin.on("end", () => process.exit(shuttingDown ? 0 : 0));
