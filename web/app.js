const PRESETS = {
  hello: `# CineBrew - Hello World
POUR "Hello, World!";
`,
  factorial: `# CineBrew - Recursive Factorial
SCENE factorial(n) {
    IF n <= 1 {
        SHOT 1;
    }
    SHOT n * factorial(n - 1);
}

TAKE n = 5;
TAKE result = factorial(n);

POUR "Factorial of ";
POUR n;
POUR " = ";
POUR result;
`,
  loop: `# CineBrew - Loop Counter
TAKE count = 1;

LOOP count <= 5 {
    POUR count;
    count = count + 1;
}

POUR "Done.";
`,
  math: `# CineBrew - Built-ins & Integer Math
TAKE a = -42;
TAKE b = 17;

POUR "abs(-42) =";
POUR abs(a);

POUR "max(-42, 17) =";
POUR max(a, b);

POUR "random(100) =";
POUR random(100);
`,
  vars: `# Variable Declaration vs Reassignment
TAKE score = 10;
score = score + 5;
POUR score;
`
};

const vmStatus = document.getElementById("vmStatus");
const vmStatusText = document.getElementById("vmStatusText");
const presetSelect = document.getElementById("presetSelect");
const resetBtn = document.getElementById("resetBtn");
const runBtn = document.getElementById("runBtn");

const leftPane = document.getElementById("leftPane");
const rightColumn = document.getElementById("rightColumn");
const editorPane = document.getElementById("editorPane");
const workspace = document.getElementById("workspace");
const resizerX = document.getElementById("resizerX");
const resizerY = document.getElementById("resizerY");

const editorEl = document.getElementById("codeEditor");
const lineNumbersEl = document.getElementById("lineNumbers");
const editorShell = document.getElementById("editorShell");

const terminalShell = document.getElementById("terminalShell");
const termHistory = document.getElementById("termHistory");
const termInput = document.getElementById("termInput");
const clearTermBtn = document.getElementById("clearTermBtn");
const exitBadge = document.getElementById("exitBadge");
const execTimeEl = document.getElementById("execTime");

let cinebrewModule = null;
let jar = null;
let currentPreset = "hello";
const cmdHistory = [];
let cmdHistoryIdx = -1;

function escapeHtml(str) {
  return str
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;");
}

const TOKEN_REGEX =
  /(#[^\n]*)|("(?:[^"\\]|\\.)*")|\b(TAKE|POUR|SCENE|SHOT|IF|ELSE|LOOP|BREAK|CONTINUE|true|false)\b|\b(print|input|random|time|abs|min|max)\b|(-?\d+)|(==|!=|<=|>=|[+\-*/=<>])|([{}();,])/g;

function highlightCineBrew(code) {
  let out = "";
  let lastIndex = 0;

  for (const match of code.matchAll(TOKEN_REGEX)) {
    const idx = match.index;
    if (idx > lastIndex) {
      out += escapeHtml(code.slice(lastIndex, idx));
    }

    const full = match[0];
    let cls = "";
    if (match[1]) cls = "tok-comment";
    else if (match[2]) cls = "tok-str";
    else if (match[3]) cls = "tok-kw";
    else if (match[4]) cls = "tok-builtin";
    else if (match[5]) cls = "tok-num";
    else if (match[6]) cls = "tok-op";
    else if (match[7]) cls = "tok-punct";

    out += `<span class="${cls}">${escapeHtml(full)}</span>`;
    lastIndex = idx + full.length;
  }

  if (lastIndex < code.length) {
    out += escapeHtml(code.slice(lastIndex));
  }

  return out;
}

function updateLineNumbers(code) {
  const normalized = code.endsWith("\n") ? code.slice(0, -1) : code;
  const count = Math.max(1, normalized.split("\n").length);
  const lines = [];
  for (let i = 1; i <= count; i++) {
    lines.push(i);
  }
  lineNumbersEl.textContent = lines.join("\n");
}

function getCode() {
  return jar ? jar.toString() : editorEl.textContent;
}

function setCode(code) {
  if (jar) {
    jar.updateCode(code);
  } else {
    editorEl.innerHTML = highlightCineBrew(code);
  }
  updateLineNumbers(code);
}

function getCaretOffset() {
  const sel = window.getSelection();
  if (!sel || sel.rangeCount === 0) return null;
  const range = sel.getRangeAt(0);
  if (!editorEl.contains(range.startContainer)) return null;
  const pre = range.cloneRange();
  pre.selectNodeContents(editorEl);
  pre.setEnd(range.startContainer, range.startOffset);
  return pre.toString().length;
}

function scrollCaretIntoView() {
  const offset = getCaretOffset();
  if (offset === null) return;

  const code = editorEl.textContent;
  const before = code.slice(0, offset);
  const lastNl = before.lastIndexOf("\n");
  const lineIndex = before.split("\n").length - 1;
  const colIndex = lastNl === -1 ? offset : offset - lastNl - 1;

  const style = getComputedStyle(editorEl);
  const lineHeight = parseFloat(style.lineHeight) || 21.6;
  const paddingTop = parseFloat(style.paddingTop) || 14;
  const paddingBottom = parseFloat(style.paddingBottom) || 14;
  const paddingLeft = parseFloat(style.paddingLeft) || 16;

  // Keep caret line visible vertically inside editorShell
  const caretTop = paddingTop + lineIndex * lineHeight;
  const caretBottom = caretTop + lineHeight;
  const viewTop = editorShell.scrollTop;
  const viewBottom = viewTop + editorShell.clientHeight;

  if (caretBottom + paddingBottom > viewBottom) {
    editorShell.scrollTop = caretBottom + paddingBottom - editorShell.clientHeight;
  } else if (caretTop - paddingTop < viewTop) {
    editorShell.scrollTop = Math.max(0, caretTop - paddingTop);
  }

  // Keep caret visible horizontally past the sticky gutter
  const gutterWidth = lineNumbersEl.offsetWidth || 46;
  const sel = window.getSelection();
  const range = sel && sel.rangeCount > 0 ? sel.getRangeAt(0) : null;
  const rect = range ? range.getBoundingClientRect() : null;
  const shellRect = editorShell.getBoundingClientRect();

  if (colIndex === 0) {
    editorShell.scrollLeft = 0;
  } else if (rect && rect.height > 0) {
    const relLeft = rect.left - shellRect.left + editorShell.scrollLeft;
    const visibleLeft = editorShell.scrollLeft + gutterWidth + paddingLeft;
    const visibleRight = editorShell.scrollLeft + editorShell.clientWidth - 20;
    if (relLeft > visibleRight) {
      editorShell.scrollLeft = relLeft - editorShell.clientWidth + 20;
    } else if (relLeft < visibleLeft) {
      editorShell.scrollLeft = Math.max(0, relLeft - gutterWidth - paddingLeft);
    }
  }
}

async function initEditor() {
  const syncEditorUI = () => {
    updateLineNumbers(editorEl.textContent);
    scrollCaretIntoView();
  };

  // Render initial preset + line numbers immediately without waiting on CDN
  editorEl.innerHTML = highlightCineBrew(PRESETS[currentPreset]);
  updateLineNumbers(PRESETS[currentPreset]);

  const highlighter = (node) => {
    const code = node.textContent;
    node.innerHTML = highlightCineBrew(code);
    updateLineNumbers(code);
  };

  try {
    const { CodeJar } = await import("https://cdn.jsdelivr.net/npm/codejar@4.2.0/+esm");
    jar = CodeJar(editorEl, highlighter, {
      tab: "    ",
      addClosing: true
    });
    jar.onUpdate((code) => updateLineNumbers(code));
  } catch (err) {
    // Graceful offline fallback using contenteditable
    console.warn("CodeJar CDN unavailable, using plain editable fallback:", err);
    editorEl.contentEditable = "plaintext-only";
  }

  // Bypass CodeJar's ~300ms debounce so line numbers & scroll stay instant
  editorEl.addEventListener("input", syncEditorUI);
  editorEl.addEventListener("keydown", (e) => {
    if (
      e.key === "Enter" ||
      e.key === "Backspace" ||
      e.key === "Delete" ||
      e.key === "ArrowUp" ||
      e.key === "ArrowDown" ||
      e.key === "ArrowLeft" ||
      e.key === "ArrowRight" ||
      e.key === "Home" ||
      e.key === "End" ||
      e.key === "PageUp" ||
      e.key === "PageDown"
    ) {
      requestAnimationFrame(syncEditorUI);
    }
  });

  setCode(PRESETS[currentPreset]);
}

// Toggle # comment on current line (Ctrl+/ or Cmd+/)
function toggleLineComment() {
  const offset = getCaretOffset();
  if (offset === null) return;
  const code = getCode();

  const lines = code.split("\n");
  let charAcc = 0;
  let targetIdx = 0;
  for (let i = 0; i < lines.length; i++) {
    const end = charAcc + lines[i].length;
    if (offset <= end || i === lines.length - 1) {
      targetIdx = i;
      break;
    }
    charAcc = end + 1;
  }

  const line = lines[targetIdx];
  const trimmed = line.trimStart();
  const indentLen = line.length - trimmed.length;
  let delta = 0;

  if (trimmed.startsWith("# ")) {
    lines[targetIdx] = line.slice(0, indentLen) + trimmed.slice(2);
    delta = -2;
  } else if (trimmed.startsWith("#")) {
    lines[targetIdx] = line.slice(0, indentLen) + trimmed.slice(1);
    delta = -1;
  } else {
    lines[targetIdx] = line.slice(0, indentLen) + "# " + trimmed;
    delta = 2;
  }

  const updated = lines.join("\n");
  const pos = jar ? jar.save() : null;
  setCode(updated);
  if (jar && pos) {
    pos.start = Math.max(0, pos.start + delta);
    pos.end = Math.max(0, pos.end + delta);
    jar.restore(pos);
  }
}

document.querySelectorAll(".tab-btn[data-doc]").forEach((btn) => {
  btn.addEventListener("click", () => {
    const target = btn.dataset.doc;
    document.querySelectorAll(".tab-btn[data-doc]").forEach((b) => b.classList.remove("active"));
    document.querySelectorAll(".doc-section").forEach((sec) => sec.classList.remove("active"));
    btn.classList.add("active");
    document.getElementById(`doc-${target}`).classList.add("active");
  });
});

document.querySelectorAll(".load-snippet-btn").forEach((btn) => {
  btn.addEventListener("click", () => {
    const key = btn.dataset.snippet;
    if (PRESETS[key]) {
      setCode(PRESETS[key]);
      if (presetSelect.querySelector(`option[value="${key}"]`)) {
        presetSelect.value = key;
        currentPreset = key;
      }
      editorEl.focus();
    }
  });
});

function setupResizers() {
  resizerX.addEventListener("pointerdown", (e) => {
    e.preventDefault();
    resizerX.classList.add("active");
    resizerX.setPointerCapture(e.pointerId);

    const onMove = (moveEvent) => {
      const rect = workspace.getBoundingClientRect();
      const newWidth = moveEvent.clientX - rect.left;
      const minW = 280;
      const maxW = rect.width - 320;
      if (newWidth >= minW && newWidth <= maxW) {
        leftPane.style.width = `${(newWidth / rect.width) * 100}%`;
      }
    };

    const onUp = () => {
      resizerX.classList.remove("active");
      resizerX.removeEventListener("pointermove", onMove);
      resizerX.removeEventListener("pointerup", onUp);
    };

    resizerX.addEventListener("pointermove", onMove);
    resizerX.addEventListener("pointerup", onUp);
  });

  resizerY.addEventListener("pointerdown", (e) => {
    e.preventDefault();
    resizerY.classList.add("active");
    resizerY.setPointerCapture(e.pointerId);

    const onMove = (moveEvent) => {
      const rect = rightColumn.getBoundingClientRect();
      const newHeight = moveEvent.clientY - rect.top;
      const minH = 150;
      const maxH = rect.height - 130;
      if (newHeight >= minH && newHeight <= maxH) {
        editorPane.style.height = `${(newHeight / rect.height) * 100}%`;
      }
    };

    const onUp = () => {
      resizerY.classList.remove("active");
      resizerY.removeEventListener("pointermove", onMove);
      resizerY.removeEventListener("pointerup", onUp);
    };

    resizerY.addEventListener("pointermove", onMove);
    resizerY.addEventListener("pointerup", onUp);
  });
}

function appendTermPromptLine(command) {
  const div = document.createElement("div");
  div.className = "term-line cmd";
  div.innerHTML =
    `<span class="term-user">guest@cinebrew</span>` +
    `<span class="term-sep">:</span>` +
    `<span class="term-path">~/sandbox</span>` +
    `<span class="term-sep">$ </span>` +
    `${escapeHtml(command)}`;
  termHistory.appendChild(div);
}

function appendTermLine(text, kind = "stdout") {
  const div = document.createElement("div");
  div.className = `term-line ${kind}`;
  div.textContent = text;
  termHistory.appendChild(div);
}

function scrollTerminalToBottom() {
  terminalShell.scrollTop = terminalShell.scrollHeight;
}

function clearTerminal() {
  termHistory.innerHTML = "";
  exitBadge.textContent = "idle";
  exitBadge.className = "badge-exit";
  execTimeEl.textContent = "0.00 ms";
}

function classifyOutputLine(line) {
  if (
    line.startsWith("--- Compilation Errors ---") ||
    line.startsWith("Semantic Error:") ||
    line.startsWith("Parse Error:") ||
    line.startsWith("Lexer Error:") ||
    line.startsWith("CodeGen Error:") ||
    line.startsWith("Runtime Exception:") ||
    line.startsWith("ERROR:") ||
    line.startsWith("Error:")
  ) {
    return "err";
  }
  if (line.startsWith("WARNING:")) return "warn";
  if (line.startsWith("[DEBUG]")) return "debug";
  return "stdout";
}

function runCode() {
  if (!cinebrewModule) return;

  vmStatus.className = "vm-status running";
  vmStatusText.textContent = "Running...";
  appendTermPromptLine("cinebrew run main.cb");

  const code = getCode();
  const t0 = performance.now();
  let hadError = false;

  try {
    const rawOutput = cinebrewModule.runCineBrew(code);
    const elapsed = (performance.now() - t0).toFixed(2);
    execTimeEl.textContent = `${elapsed} ms`;

    const lines = rawOutput.replace(/\r\n/g, "\n").split("\n");
    // Drop single trailing newline from std::endl
    if (lines.length > 0 && lines[lines.length - 1] === "") {
      lines.pop();
    }

    if (lines.length === 0) {
      appendTermLine("(no stdout)", "meta");
    } else {
      for (const line of lines) {
        const kind = classifyOutputLine(line);
        if (kind === "err") hadError = true;
        appendTermLine(line, kind);
      }
    }

    const exitCode = hadError ? 1 : 0;
    exitBadge.textContent = `exit ${exitCode}`;
    exitBadge.className = `badge-exit ${hadError ? "fail" : "ok"}`;
    appendTermLine(`[process exited with code ${exitCode} in ${elapsed} ms]`, "meta");
  } catch (err) {
    const elapsed = (performance.now() - t0).toFixed(2);
    execTimeEl.textContent = `${elapsed} ms`;
    appendTermLine(`VM Abort: ${err}`, "err");
    exitBadge.textContent = "exit 1";
    exitBadge.className = "badge-exit fail";
  } finally {
    vmStatus.className = "vm-status ready";
    vmStatusText.textContent = "VM Ready";
    scrollTerminalToBottom();
  }
}

function handleTerminalCommand(raw) {
  const cmd = raw.trim();
  appendTermPromptLine(cmd);
  if (!cmd) {
    scrollTerminalToBottom();
    return;
  }

  cmdHistory.unshift(cmd);
  cmdHistoryIdx = -1;

  const lower = cmd.toLowerCase();
  if (lower === "clear" || lower === "cls") {
    clearTerminal();
    return;
  } else if (lower === "run" || lower === "cinebrew run main.cb") {
    // Remove duplicate prompt line since runCode prints its own
    termHistory.lastChild.remove();
    runCode();
    return;
  } else if (lower === "cat" || lower === "cat main.cb") {
    getCode().split("\n").forEach((l) => appendTermLine(l, "stdout"));
  } else if (lower === "ls") {
    appendTermLine("main.cb  cinebrew.wasm", "stdout");
  } else if (lower === "help") {
    appendTermLine("Available commands:", "meta");
    appendTermLine("  run            Compile and execute main.cb", "stdout");
    appendTermLine("  cat            Print contents of main.cb", "stdout");
    appendTermLine("  ls             List workspace files", "stdout");
    appendTermLine("  clear          Clear terminal history", "stdout");
  } else {
    appendTermLine(`bash: ${cmd.split(/\s+/)[0]}: command not found (type 'help')`, "err");
  }

  scrollTerminalToBottom();
}

presetSelect.addEventListener("change", (e) => {
  currentPreset = e.target.value;
  if (PRESETS[currentPreset]) {
    setCode(PRESETS[currentPreset]);
  }
});

resetBtn.addEventListener("click", () => {
  if (PRESETS[currentPreset]) {
    setCode(PRESETS[currentPreset]);
  }
});

runBtn.addEventListener("click", runCode);
clearTermBtn.addEventListener("click", clearTerminal);

terminalShell.addEventListener("click", (e) => {
  if (window.getSelection().toString() === "" && e.target !== termInput) {
    termInput.focus();
  }
});

termInput.addEventListener("keydown", (e) => {
  if (e.key === "Enter") {
    e.preventDefault();
    const val = termInput.value;
    termInput.value = "";
    handleTerminalCommand(val);
  } else if (e.key === "ArrowUp") {
    e.preventDefault();
    if (cmdHistoryIdx + 1 < cmdHistory.length) {
      cmdHistoryIdx++;
      termInput.value = cmdHistory[cmdHistoryIdx];
    }
  } else if (e.key === "ArrowDown") {
    e.preventDefault();
    if (cmdHistoryIdx > 0) {
      cmdHistoryIdx--;
      termInput.value = cmdHistory[cmdHistoryIdx];
    } else {
      cmdHistoryIdx = -1;
      termInput.value = "";
    }
  }
});

window.addEventListener("keydown", (e) => {
  const mod = e.ctrlKey || e.metaKey;
  if (mod && e.key === "Enter") {
    e.preventDefault();
    if (!runBtn.disabled) runCode();
  } else if (mod && e.key === "/") {
    if (document.activeElement === editorEl || editorShell.contains(document.activeElement)) {
      e.preventDefault();
      toggleLineComment();
    }
  } else if (mod && e.key.toLowerCase() === "l") {
    e.preventDefault();
    clearTerminal();
  }
});

async function boot() {
  setupResizers();
  await initEditor();

  try {
    cinebrewModule = await createCineBrewModule();
    vmStatus.className = "vm-status ready";
    vmStatusText.textContent = "VM Ready";
    runBtn.disabled = false;
    appendTermLine("CineBrew WASM runtime initialized. Type 'help' or press Ctrl+Enter to run.", "meta");
  } catch (err) {
    vmStatus.className = "vm-status error";
    vmStatusText.textContent = "VM Error";
    appendTermLine(`Failed to load cinebrew.wasm: ${err}`, "err");
    console.error(err);
  }
}

boot();
