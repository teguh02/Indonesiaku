// Integration smoke test for the standalone Indonesiaku LSP server.
// Uses only Node's built-in modules; no VS Code process or npm dependencies.
"use strict";

const assert = require("assert");
const { spawn } = require("child_process");
const fs = require("fs");
const os = require("os");
const path = require("path");
const { pathToFileURL } = require("url");

const root = path.resolve(__dirname, "../..");
const indk = process.argv[2] || path.join(root, process.platform === "win32" ? "indk.exe" : "indk");
const serverPath = path.join(__dirname, "server.js");
const temp = fs.mkdtempSync(path.join(os.tmpdir(), "indonesiaku-lsp-test-"));
const sourcePath = path.join(temp, "uji.idk");
const uri = pathToFileURL(sourcePath).toString();

const child = spawn(process.execPath, [serverPath], {
  env: { ...process.env, INDONESIAKU_INDK: indk },
  stdio: ["pipe", "pipe", "pipe"]
});

let output = Buffer.alloc(0);
let stderr = "";
const messages = [];
const waiters = [];

function deliver(message) {
  const index = waiters.findIndex(w => w.predicate(message));
  if (index >= 0) {
    const [waiter] = waiters.splice(index, 1);
    clearTimeout(waiter.timer);
    waiter.resolve(message);
  } else {
    messages.push(message);
  }
}

child.stdout.on("data", chunk => {
  output = Buffer.concat([output, chunk]);
  for (;;) {
    const split = output.indexOf("\r\n\r\n");
    if (split < 0) break;
    const header = output.subarray(0, split).toString("ascii");
    const match = header.match(/Content-Length:\s*(\d+)/i);
    if (!match) throw new Error(`Invalid LSP header: ${header}`);
    const length = Number(match[1]);
    const bodyStart = split + 4;
    if (output.length < bodyStart + length) break;
    const body = output.subarray(bodyStart, bodyStart + length).toString("utf8");
    output = output.subarray(bodyStart + length);
    deliver(JSON.parse(body));
  }
});
child.stderr.on("data", chunk => { stderr += chunk.toString("utf8"); });

function send(message) {
  const body = Buffer.from(JSON.stringify(message), "utf8");
  child.stdin.write(`Content-Length: ${body.length}\r\n\r\n`);
  child.stdin.write(body);
}

function nextMessage(predicate, timeoutMs = 5000) {
  const index = messages.findIndex(predicate);
  if (index >= 0) return Promise.resolve(messages.splice(index, 1)[0]);
  return new Promise((resolve, reject) => {
    const waiter = { predicate, resolve, timer: null };
    waiter.timer = setTimeout(() => {
      const i = waiters.indexOf(waiter);
      if (i >= 0) waiters.splice(i, 1);
      reject(new Error(`Timed out waiting for LSP message. stderr:\n${stderr}`));
    }, timeoutMs);
    waiters.push(waiter);
  });
}

function response(id) { return m => m.id === id && ("result" in m || "error" in m); }
function notification(method, uriFilter) {
  return m => m.method === method &&
    (!uriFilter || (m.params && m.params.uri === uriFilter));
}

async function request(id, method, params) {
  send({ jsonrpc: "2.0", id, method, params });
  const message = await nextMessage(response(id));
  if (message.error) throw new Error(`${method}: ${JSON.stringify(message.error)}`);
  return message.result;
}

async function run() {
  assert(fs.existsSync(indk), `indk binary not found: ${indk}`);
  fs.writeFileSync(sourcePath, "", "utf8");

  const init = await request(1, "initialize", {
    processId: process.pid,
    rootUri: `file://${temp.replace(/\\/g, "/")}`,
    capabilities: {}
  });
  assert(init.capabilities.textDocumentSync, "textDocumentSync capability missing");
  assert(init.capabilities.completionProvider, "completionProvider capability missing");
  assert(init.capabilities.hoverProvider, "hoverProvider capability missing");
  assert(init.capabilities.definitionProvider, "definitionProvider capability missing");
  send({ jsonrpc: "2.0", method: "initialized", params: {} });

  // Compiler errors from indk --dis must be published with LSP (0-based) ranges.
  send({ jsonrpc: "2.0", method: "textDocument/didOpen", params: {
    textDocument: { uri, languageId: "indonesiaku", version: 1,
      text: 'fungsi salam() {\n  cetak("halo")\n' }
  }});
  const broken = await nextMessage(notification("textDocument/publishDiagnostics", uri));
  assert(broken.params.diagnostics.length > 0, "expected a diagnostic for missing closing brace");
  assert(Number.isInteger(broken.params.diagnostics[0].range.start.line), "diagnostic line must be integer");
  for (const item of broken.params.diagnostics) {
    const line = 'fungsi salam() {\n  cetak("halo")\n'.split("\n")[item.range.start.line] || "";
    assert(item.range.start.character <= line.length, "diagnostic start must be within the document line");
    assert(item.range.end.character <= line.length, "diagnostic end must be within the document line");
  }

  // Compiler columns count UTF-8 bytes; LSP positions count UTF-16 code units.
  const unicodeUri = pathToFileURL(path.join(temp, "unicode.idk")).toString();
  const unicodeSource = 'cetak("🙂") fungsi salah( {\n';
  fs.writeFileSync(path.join(temp, "unicode.idk"), unicodeSource, "utf8");
  send({ jsonrpc: "2.0", method: "textDocument/didOpen", params: {
    textDocument: { uri: unicodeUri, languageId: "indonesiaku", version: 1, text: unicodeSource }
  }});
  const unicodeDiagnostics = await nextMessage(notification("textDocument/publishDiagnostics", unicodeUri));
  assert(unicodeDiagnostics.params.diagnostics.length > 0, "expected Unicode source syntax diagnostic");
  assert.strictEqual(unicodeDiagnostics.params.diagnostics[0].range.start.character, 19,
    "compiler UTF-8 byte column must be converted to the VS Code UTF-16 character offset");

  // Unsaved editor buffers use untitled: URIs and still need diagnostics.
  const untitledUri = "untitled:Indonesiaku-1";
  send({ jsonrpc: "2.0", method: "textDocument/didOpen", params: {
    textDocument: { uri: untitledUri, languageId: "indonesiaku", version: 1,
      text: "fungsi belum_selesai( {\n" }
  }});
  const untitledDiagnostics = await nextMessage(notification("textDocument/publishDiagnostics", untitledUri));
  assert(untitledDiagnostics.params.diagnostics.length > 0,
    "unsaved/untitled documents should also receive compiler diagnostics");

  // Existing compiler lint findings are surfaced as LSP warnings (not errors).
  const lintUri = pathToFileURL(path.join(temp, "lint.idk")).toString();
  const lintSource = "fungsi uji() {\n  variabel lokal_tidak_dipakai = 7\n}\n";
  fs.writeFileSync(path.join(temp, "lint.idk"), lintSource, "utf8");
  send({ jsonrpc: "2.0", method: "textDocument/didOpen", params: {
    textDocument: { uri: lintUri, languageId: "indonesiaku", version: 1, text: lintSource }
  }});
  const lintDiagnostics = await nextMessage(notification("textDocument/publishDiagnostics", lintUri));
  const unusedWarning = lintDiagnostics.params.diagnostics.find(d => d.message.includes("lokal_tidak_dipakai"));
  assert(unusedWarning, "LSP should surface the existing unused-local lint warning");
  assert.strictEqual(unusedWarning.severity, 2, "unused-local finding should be a warning");

  // Completion returns language keywords and builtin names.
  const completion = await request(2, "textDocument/completion", {
    textDocument: { uri }, position: { line: 0, character: 0 }
  });
  const labels = completion.items.map(item => item.label);
  assert(labels.includes("fungsi"), "completion missing keyword 'fungsi'");
  assert(labels.includes("panjang"), "completion missing builtin 'panjang'");
  assert(labels.includes("salam"), "completion missing local document symbol 'salam'");

  // Hover returns documentation for a known builtin.
  const hover = await request(3, "textDocument/hover", {
    textDocument: { uri }, position: { line: 1, character: 4 }
  });
  assert(hover && hover.contents, "hover should return contents for a builtin");

  // Go-to-definition resolves a local function declaration in the same file.
  const definitionUri = pathToFileURL(path.join(temp, "def.idk")).toString();
  const definitionSource = "fungsi sapa() { }\nsapa()\nvariabel nilai = 1\ncetak(nilai == 1)\n";
  fs.writeFileSync(path.join(temp, "def.idk"), definitionSource, "utf8");
  send({ jsonrpc: "2.0", method: "textDocument/didOpen", params: {
    textDocument: { uri: definitionUri, languageId: "indonesiaku", version: 1,
      text: definitionSource }
  }});
  await nextMessage(notification("textDocument/publishDiagnostics", definitionUri));
  const definition = await request(4, "textDocument/definition", {
    textDocument: { uri: definitionUri }, position: { line: 1, character: 2 }
  });
  assert(definition && definition.uri === definitionUri, "definition URI should match source document");
  assert(definition.range.start.line === 0, "definition should point to declaration line");
  const variableDefinition = await request(5, "textDocument/definition", {
    textDocument: { uri: definitionUri }, position: { line: 3, character: 9 }
  });
  assert(variableDefinition && variableDefinition.range.start.line === 2,
    "comparison usage must resolve to the variable declaration, not be mistaken for an assignment");

  // Compile-only diagnostics must never execute user code (keluar would
  // terminate the interpreter if the server accidentally used normal run).
  const noExecutionUri = pathToFileURL(path.join(temp, "no-execute.idk")).toString();
  send({ jsonrpc: "2.0", method: "textDocument/didOpen", params: {
    textDocument: { uri: noExecutionUri, languageId: "indonesiaku", version: 1, text: "keluar()\n" }
  }});
  const noExecutionDiagnostics = await nextMessage(notification("textDocument/publishDiagnostics", noExecutionUri));
  assert.strictEqual(noExecutionDiagnostics.params.diagnostics.length, 0, "valid call to keluar should compile cleanly");
  const afterNoExecution = await request(6, "textDocument/completion", {
    textDocument: { uri }, position: { line: 0, character: 0 }
  });
  assert(afterNoExecution.items.length > 0, "LSP process must remain alive and must not execute document code");

  // A valid change clears diagnostics.
  send({ jsonrpc: "2.0", method: "textDocument/didChange", params: {
    textDocument: { uri, version: 2 },
    contentChanges: [{ text: 'cetak("aman")\n' }]
  }});
  const fixed = await nextMessage(notification("textDocument/publishDiagnostics", uri));
  assert.strictEqual(fixed.params.diagnostics.length, 0, "valid document must clear diagnostics");

  // A quick subsequent edit cancels/debounces obsolete diagnostics; only the
  // current document version should be published.
  const cancelUri = pathToFileURL(path.join(temp, "cancel.idk")).toString();
  send({ jsonrpc: "2.0", method: "textDocument/didOpen", params: {
    textDocument: { uri: cancelUri, languageId: "indonesiaku", version: 1, text: "jika benar {\n" }
  }});
  send({ jsonrpc: "2.0", method: "textDocument/didChange", params: {
    textDocument: { uri: cancelUri, version: 2 },
    contentChanges: [{ text: "cetak(1)\n" }]
  }});
  const afterCancel = await nextMessage(notification("textDocument/publishDiagnostics", cancelUri));
  assert.strictEqual(afterCancel.params.version, 2, "stale diagnostics for version 1 must not publish");
  assert.strictEqual(afterCancel.params.diagnostics.length, 0, "latest valid version should publish no errors");

  send({ jsonrpc: "2.0", method: "textDocument/didClose", params: {
    textDocument: { uri }
  }});
  const closed = await nextMessage(notification("textDocument/publishDiagnostics", uri));
  assert.strictEqual(closed.params.diagnostics.length, 0, "closing a document must clear its diagnostics");

  await request(7, "shutdown", null);
  send({ jsonrpc: "2.0", method: "exit" });
  await new Promise(resolve => child.once("exit", resolve));
  assert.strictEqual(child.exitCode, 0, `LSP server exit ${child.exitCode}; stderr:\n${stderr}`);
  console.log("LSP integration tests passed");
}

run().catch(error => {
  console.error(error.stack || error);
  child.kill();
  process.exitCode = 1;
}).finally(() => {
  fs.rmSync(temp, { recursive: true, force: true });
});
