import React, { useState, useEffect, useRef } from 'react';
import {
  Play,
  CheckCircle2,
  Cpu,
  Download,
  FileCode,
  BookOpen,
  Terminal,
  Layers,
  Sparkles,
  RefreshCw,
  AlertTriangle,
  Code2,
  Clock,
  Binary,
  GitBranch,
  Copy,
  Check,
  ChevronRight,
  ShieldCheck,
  Info
} from 'lucide-react';

interface Diagnostic {
  level: 'error' | 'warning' | 'info';
  message: string;
  line: number;
  column: number;
  suggestion?: string;
}

interface Example {
  id: string;
  name: string;
  code: string;
}

const DEFAULT_CODE = `# Welcome to HUMA X
# Human-Oriented Universal Machine Language

make name = "Manojit"
make age = 18

show "Hello, HUMA X!"
show "Developer:", name

change age to age + 1

if age is greater than 18 then
    show "Status: Adult"
otherwise
    show "Status: Underage"
end

show "--- Running Loop ---"
repeat 3 times
    show "HUMA X is human-oriented & high-performance!"
end

create function calculate_area using radius
    fixed PI = 3.14159265
    give back PI * (radius ^ 2)
end

make circle_area = call calculate_area using 5
show "Area of circle with radius 5:", circle_area
`;

export default function App() {
  const [code, setCode] = useState<string>(DEFAULT_CODE);
  const [output, setOutput] = useState<string>('');
  const [errorOutput, setErrorOutput] = useState<string>('');
  const [diagnostics, setDiagnostics] = useState<Diagnostic[]>([]);
  const [target, setTarget] = useState<'vm' | 'native' | 'wasm'>('vm');
  const [activeTab, setActiveTab] = useState<'console' | 'bytecode' | 'ir' | 'ast' | 'repl' | 'docs'>('console');
  const [bytecode, setBytecode] = useState<string>('');
  const [irCode, setIrCode] = useState<string>('');
  const [astCode, setAstCode] = useState<string>('');
  const [isRunning, setIsRunning] = useState<boolean>(false);
  const [isCompiling, setIsCompiling] = useState<boolean>(false);
  const [executionTime, setExecutionTime] = useState<number | null>(null);
  const [examples, setExamples] = useState<Example[]>([]);
  const [selectedExampleId, setSelectedExampleId] = useState<string>('default');
  const [copied, setCopied] = useState<boolean>(false);
  const [showMobileModal, setShowMobileModal] = useState<boolean>(false);
  const [mobileTab, setMobileTab] = useState<'termux' | 'runner' | 'github'>('runner');

  // REPL state
  const [replInput, setReplInput] = useState<string>('');
  const [replHistory, setReplHistory] = useState<Array<{ cmd: string; res: string; isError?: boolean }>>([
    { cmd: 'make greeting = "Hello REPL"', res: '' },
    { cmd: 'show greeting', res: 'Hello REPL' },
  ]);

  const textareaRef = useRef<HTMLTextAreaElement>(null);

  // Load examples from backend
  useEffect(() => {
    fetch('/api/examples')
      .then(res => res.json())
      .then(data => {
        if (data.examples && data.examples.length > 0) {
          setExamples(data.examples);
        }
      })
      .catch(() => {
        // Fallback examples
        setExamples([
          { id: 'hello', name: 'Hello World', code: DEFAULT_CODE },
          { id: 'variables', name: 'Variables & Constants', code: 'fixed PI = 3.14159\nmake radius = 10\nmake area = PI * (radius ^ 2)\nshow "Area:", area\n' },
          { id: 'fibonacci', name: 'Fibonacci Sequence', code: 'create function fib using n\n    if n is less than or equal to 1 then\n        give back n\n    otherwise\n        give back (call fib using (n - 1)) + (call fib using (n - 2))\n    end\nend\n\nrepeat for each i in [0, 1, 2, 3, 4, 5, 6, 7]\n    show "fib(", i, ") = ", call fib using i\nend\n' }
        ]);
      });
  }, []);

  const handleRun = async () => {
    setIsRunning(true);
    setOutput('');
    setErrorOutput('');
    setDiagnostics([]);
    try {
      const res = await fetch('/api/run', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ source: code, target }),
      });
      const data = await res.json();
      setOutput(data.stdout || '');
      setErrorOutput(data.stderr || '');
      setExecutionTime(data.execution_time_ms ?? 0);
      setActiveTab('console');
    } catch (err: any) {
      setErrorOutput(err.message || 'Execution request failed');
    } finally {
      setIsRunning(false);
    }
  };

  const handleCheck = async () => {
    setIsRunning(true);
    try {
      const res = await fetch('/api/check', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ source: code }),
      });
      const data = await res.json();
      setDiagnostics(data.diagnostics || []);
      if (data.success) {
        setOutput('✓ Syntax & Semantics check passed! No errors detected.');
      } else {
        setErrorOutput('Semantic or syntax errors detected. Review diagnostics below.');
      }
      setActiveTab('console');
    } catch (err: any) {
      setErrorOutput(err.message || 'Validation request failed');
    } finally {
      setIsRunning(false);
    }
  };

  const handleNativeBuild = async () => {
    setIsCompiling(true);
    try {
      const res = await fetch('/api/compile', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ source: code, target: 'native' }),
      });
      const data = await res.json();
      if (data.success) {
        setOutput(`✓ Native compilation successful!\nTarget: Linux x86-64 ELF Binary\nTime: ${data.compilation_time_ms}ms\n\n${data.details || ''}`);
      } else {
        setErrorOutput(data.error || 'Compilation failed');
      }
      setActiveTab('console');
    } catch (err: any) {
      setErrorOutput(err.message || 'Build request failed');
    } finally {
      setIsCompiling(false);
    }
  };

  const handleInspectBytecode = async () => {
    try {
      const res = await fetch('/api/bytecode', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ source: code }),
      });
      const data = await res.json();
      setBytecode(data.bytecode || data.error || 'No bytecode generated');
      setActiveTab('bytecode');
    } catch (err: any) {
      setBytecode(err.message);
    }
  };

  const handleInspectIr = async () => {
    try {
      const res = await fetch('/api/ir', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ source: code }),
      });
      const data = await res.json();
      setIrCode(data.ir || data.error || 'No IR generated');
      setActiveTab('ir');
    } catch (err: any) {
      setIrCode(err.message);
    }
  };

  const handleInspectAst = async () => {
    try {
      const res = await fetch('/api/ast', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ source: code }),
      });
      const data = await res.json();
      setAstCode(data.ast || data.error || 'No AST generated');
      setActiveTab('ast');
    } catch (err: any) {
      setAstCode(err.message);
    }
  };

  const handleFormat = async () => {
    try {
      const res = await fetch('/api/format', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ source: code }),
      });
      const data = await res.json();
      if (data.formatted) {
        setCode(data.formatted);
      }
    } catch {}
  };

  const handleReplSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!replInput.trim()) return;
    const currentInput = replInput;
    setReplInput('');

    try {
      const res = await fetch('/api/run', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ source: currentInput, target: 'vm' }),
      });
      const data = await res.json();
      setReplHistory(prev => [
        ...prev,
        {
          cmd: currentInput,
          res: data.stdout || data.stderr || (data.status === 'success' ? 'ok' : 'error'),
          isError: !!data.stderr || data.status === 'error',
        },
      ]);
    } catch (err: any) {
      setReplHistory(prev => [
        ...prev,
        { cmd: currentInput, res: err.message, isError: true },
      ]);
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLTextAreaElement>) => {
    if (e.key === 'Tab') {
      e.preventDefault();
      const targetEl = e.currentTarget;
      const start = targetEl.selectionStart;
      const end = targetEl.selectionEnd;
      const newCode = code.substring(0, start) + '    ' + code.substring(end);
      setCode(newCode);
      setTimeout(() => {
        targetEl.selectionStart = targetEl.selectionEnd = start + 4;
      }, 0);
    }
  };

  const copyCode = () => {
    navigator.clipboard.writeText(code);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  return (
    <div className="flex flex-col h-screen w-full bg-[#0d1117] text-slate-100 font-sans overflow-hidden">
      {/* Top Header / Control Bar */}
      <header className="flex items-center justify-between px-4 py-2.5 bg-[#161b22] border-b border-slate-800 shrink-0">
        <div className="flex items-center space-x-3">
          <div className="flex items-center space-x-2">
            <div className="w-8 h-8 rounded-lg bg-gradient-to-tr from-cyan-600 to-indigo-600 flex items-center justify-center font-bold text-white shadow-md shadow-cyan-900/40">
              HX
            </div>
            <div>
              <div className="flex items-center space-x-2">
                <span className="font-extrabold text-base tracking-tight text-white">HUMA X</span>
                <span className="px-1.5 py-0.5 text-[10px] font-semibold tracking-wider uppercase bg-cyan-950 text-cyan-400 border border-cyan-800/60 rounded">
                  v1.0.0 Real C++20
                </span>
              </div>
              <span className="text-[11px] text-slate-400 hidden md:inline">
                Human-Oriented Universal Machine Language
              </span>
            </div>
          </div>
        </div>

        {/* Center / Action Toolbar */}
        <div className="flex items-center space-x-2">
          {/* Target Selector */}
          <div className="flex items-center bg-[#0d1117] border border-slate-700/80 rounded-md p-0.5 text-xs">
            <button
              onClick={() => setTarget('vm')}
              className={`px-2.5 py-1 rounded transition font-medium ${
                target === 'vm' ? 'bg-cyan-600 text-white shadow-sm' : 'text-slate-400 hover:text-white'
              }`}
            >
              VM Engine
            </button>
            <button
              onClick={() => setTarget('native')}
              className={`px-2.5 py-1 rounded transition font-medium ${
                target === 'native' ? 'bg-emerald-600 text-white shadow-sm' : 'text-slate-400 hover:text-white'
              }`}
            >
              Native (ELF)
            </button>
            <button
              onClick={() => setTarget('wasm')}
              className={`px-2.5 py-1 rounded transition font-medium ${
                target === 'wasm' ? 'bg-purple-600 text-white shadow-sm' : 'text-slate-400 hover:text-white'
              }`}
            >
              WASM / JS
            </button>
          </div>

          {/* Run Button */}
          <button
            onClick={handleRun}
            disabled={isRunning || isCompiling}
            className="flex items-center space-x-1.5 px-3.5 py-1.5 rounded-md bg-gradient-to-r from-emerald-600 to-teal-600 hover:from-emerald-500 hover:to-teal-500 font-semibold text-xs text-white shadow-md shadow-emerald-950 transition disabled:opacity-50 cursor-pointer"
          >
            <Play className={`w-3.5 h-3.5 fill-current ${isRunning ? 'animate-spin' : ''}`} />
            <span>{isRunning ? 'Running...' : 'Run Program'}</span>
          </button>

          {/* Check Button */}
          <button
            onClick={handleCheck}
            disabled={isRunning}
            className="hidden sm:flex items-center space-x-1 px-2.5 py-1.5 rounded-md bg-slate-800 hover:bg-slate-700 text-slate-200 text-xs font-medium border border-slate-700 transition cursor-pointer"
            title="Validate syntax & type semantics"
          >
            <CheckCircle2 className="w-3.5 h-3.5 text-cyan-400" />
            <span>Check</span>
          </button>

          {/* Native Build Button */}
          <button
            onClick={handleNativeBuild}
            disabled={isCompiling}
            className="hidden md:flex items-center space-x-1 px-2.5 py-1.5 rounded-md bg-slate-800 hover:bg-slate-700 text-slate-200 text-xs font-medium border border-slate-700 transition cursor-pointer"
            title="Compile directly to native binary using host GCC 12"
          >
            <Cpu className="w-3.5 h-3.5 text-amber-400" />
            <span>Build Binary</span>
          </button>

          {/* Format Button */}
          <button
            onClick={handleFormat}
            className="hidden lg:flex items-center space-x-1 px-2.5 py-1.5 rounded-md bg-slate-800 hover:bg-slate-700 text-slate-200 text-xs font-medium border border-slate-700 transition cursor-pointer"
            title="Auto-format HUMA X code"
          >
            <Code2 className="w-3.5 h-3.5 text-indigo-400" />
            <span>Format</span>
          </button>
        </div>

        {/* Right Tools / Zip Download & Mobile */}
        <div className="flex items-center space-x-2">
          <button
            onClick={() => setShowMobileModal(true)}
            className="flex items-center space-x-1.5 px-3 py-1.5 rounded-md bg-gradient-to-r from-emerald-600 to-teal-600 hover:from-emerald-500 hover:to-teal-500 text-white text-xs font-semibold shadow-md shadow-emerald-950 transition cursor-pointer"
          >
            <span>📱</span>
            <span>মোবাইলে চালান / Mobile</span>
          </button>

          <a
            href="/api/download-zip"
            download="huma-x-complete-v1.0.0.zip"
            className="flex items-center space-x-1.5 px-3 py-1.5 rounded-md bg-gradient-to-r from-indigo-600 to-blue-600 hover:from-indigo-500 hover:to-blue-500 text-white text-xs font-semibold shadow-md shadow-indigo-950 transition cursor-pointer"
            title="Download complete offline C++ compiler repository & test suite as .zip"
          >
            <Download className="w-3.5 h-3.5" />
            <span className="hidden sm:inline">Download (.zip)</span>
          </a>
        </div>
      </header>

      {/* Main Workspace Body */}
      <div className="flex flex-1 overflow-hidden">
        {/* Left Sidebar: Examples & Presets */}
        <div className="w-56 bg-[#11161d] border-r border-slate-800/80 flex flex-col shrink-0">
          <div className="p-3 border-b border-slate-800 text-[11px] font-bold text-slate-400 uppercase tracking-wider flex items-center justify-between">
            <span>Example Programs</span>
            <Sparkles className="w-3.5 h-3.5 text-cyan-400" />
          </div>

          <div className="flex-1 overflow-y-auto p-2 space-y-1 text-xs">
            {examples.map(ex => (
              <button
                key={ex.id}
                onClick={() => {
                  setSelectedExampleId(ex.id);
                  setCode(ex.code);
                }}
                className={`w-full text-left px-2.5 py-2 rounded flex items-center space-x-2 transition ${
                  selectedExampleId === ex.id
                    ? 'bg-cyan-950/80 text-cyan-300 font-medium border border-cyan-800/50'
                    : 'text-slate-400 hover:bg-slate-800/60 hover:text-slate-200'
                }`}
              >
                <FileCode className="w-3.5 h-3.5 shrink-0 opacity-70" />
                <span className="truncate">{ex.name}</span>
              </button>
            ))}

            <div className="pt-3 pb-1 px-1">
              <span className="text-[10px] font-bold uppercase tracking-wider text-slate-500">
                Typo Diagnostics Demo
              </span>
            </div>

            <button
              onClick={() => {
                setSelectedExampleId('typo_demo');
                setCode(`# Levenshtein Typo Suggestion Demonstration
make user_amount = 500
show user_amout   # Notice the typo: 'user_amout'
`);
              }}
              className="w-full text-left px-2.5 py-1.5 rounded flex items-center space-x-2 text-amber-400/90 hover:bg-amber-950/30 transition text-xs"
            >
              <AlertTriangle className="w-3.5 h-3.5 shrink-0" />
              <span className="truncate">Typo Diagnostic Demo</span>
            </button>

            <button
              onClick={() => {
                setSelectedExampleId('fixed_demo');
                setCode(`# Immutability Enforcement Demo
fixed SPEED_OF_LIGHT = 299792458
show "Speed of light:", SPEED_OF_LIGHT
change SPEED_OF_LIGHT to 300000000  # Will trigger error
`);
              }}
              className="w-full text-left px-2.5 py-1.5 rounded flex items-center space-x-2 text-amber-400/90 hover:bg-amber-950/30 transition text-xs"
            >
              <ShieldCheck className="w-3.5 h-3.5 shrink-0" />
              <span className="truncate">Fixed Constant Check</span>
            </button>
          </div>

          {/* Bottom Engine Specs */}
          <div className="p-2.5 bg-[#0e131a] border-t border-slate-800/80 text-[11px] text-slate-400">
            <div className="flex items-center justify-between mb-1">
              <span className="text-slate-500">Compiler:</span>
              <span className="text-emerald-400 font-mono text-[10px]">GCC 12.3.0 C++20</span>
            </div>
            <div className="flex items-center justify-between mb-1">
              <span className="text-slate-500">Target:</span>
              <span className="text-cyan-400 font-mono text-[10px]">x86-64 ELF</span>
            </div>
            <div className="flex items-center justify-between">
              <span className="text-slate-500">Test Suite:</span>
              <span className="text-emerald-400 font-mono text-[10px]">30/30 Passed</span>
            </div>
          </div>
        </div>

        {/* Center: Code Editor Area */}
        <div className="flex-1 flex flex-col min-w-0 bg-[#0d1117]">
          {/* Editor Sub-header */}
          <div className="flex items-center justify-between px-3 py-1.5 bg-[#161b22] border-b border-slate-800 text-xs text-slate-400">
            <div className="flex items-center space-x-2">
              <Code2 className="w-3.5 h-3.5 text-cyan-400" />
              <span className="font-mono text-slate-300">program.huma</span>
              <span className="text-[11px] text-slate-500">({code.split('\n').length} lines)</span>
            </div>

            <div className="flex items-center space-x-3">
              <button
                onClick={copyCode}
                className="flex items-center space-x-1 hover:text-white transition cursor-pointer"
                title="Copy Code"
              >
                {copied ? <Check className="w-3.5 h-3.5 text-emerald-400" /> : <Copy className="w-3.5 h-3.5" />}
                <span className="text-[11px]">{copied ? 'Copied' : 'Copy'}</span>
              </button>

              <button
                onClick={() => setCode(DEFAULT_CODE)}
                className="flex items-center space-x-1 hover:text-white transition cursor-pointer"
                title="Reset to default code"
              >
                <RefreshCw className="w-3.5 h-3.5" />
                <span className="text-[11px]">Reset</span>
              </button>
            </div>
          </div>

          {/* Editor Container with Line Numbers */}
          <div className="flex-1 relative flex overflow-hidden">
            <div className="w-12 bg-[#0e131a] py-3 select-none text-right pr-2.5 font-mono text-xs text-slate-600 border-r border-slate-800/80 leading-6 shrink-0">
              {code.split('\n').map((_, i) => (
                <div key={i}>{i + 1}</div>
              ))}
            </div>

            <textarea
              ref={textareaRef}
              value={code}
              onChange={e => setCode(e.target.value)}
              onKeyDown={handleKeyDown}
              spellCheck={false}
              className="flex-1 p-3 bg-transparent text-slate-100 font-mono text-xs leading-6 outline-none resize-none overflow-y-auto selection:bg-cyan-900/60"
              placeholder="Type HUMA X program here..."
            />
          </div>

          {/* Quick Syntax Snippets Bar */}
          <div className="px-3 py-1 bg-[#11161d] border-t border-slate-800 flex items-center space-x-2 overflow-x-auto text-[11px] text-slate-400 shrink-0">
            <span className="text-slate-500 text-[10px] uppercase font-bold shrink-0">Insert:</span>
            <button
              onClick={() => setCode(prev => prev + '\nmake var_name = "value"\n')}
              className="px-2 py-0.5 rounded bg-slate-800/80 hover:bg-slate-700 text-slate-300 shrink-0 font-mono text-[10px]"
            >
              + make
            </button>
            <button
              onClick={() => setCode(prev => prev + '\nfixed PI = 3.14159\n')}
              className="px-2 py-0.5 rounded bg-slate-800/80 hover:bg-slate-700 text-slate-300 shrink-0 font-mono text-[10px]"
            >
              + fixed
            </button>
            <button
              onClick={() => setCode(prev => prev + '\nif age is greater than 18 then\n    show "Adult"\notherwise\n    show "Underage"\nend\n')}
              className="px-2 py-0.5 rounded bg-slate-800/80 hover:bg-slate-700 text-slate-300 shrink-0 font-mono text-[10px]"
            >
              + if/otherwise
            </button>
            <button
              onClick={() => setCode(prev => prev + '\nrepeat 5 times\n    show "Looping"\nend\n')}
              className="px-2 py-0.5 rounded bg-slate-800/80 hover:bg-slate-700 text-slate-300 shrink-0 font-mono text-[10px]"
            >
              + repeat N times
            </button>
            <button
              onClick={() => setCode(prev => prev + '\ncreate function add using a, b\n    give back a + b\nend\n')}
              className="px-2 py-0.5 rounded bg-slate-800/80 hover:bg-slate-700 text-slate-300 shrink-0 font-mono text-[10px]"
            >
              + create function
            </button>
          </div>
        </div>

        {/* Right / Output & Inspection Panel */}
        <div className="w-[45%] bg-[#0f141c] border-l border-slate-800/80 flex flex-col shrink-0">
          {/* Tabs Navigation */}
          <div className="flex items-center justify-between px-2 bg-[#161b22] border-b border-slate-800 text-xs">
            <div className="flex items-center space-x-1">
              <button
                onClick={() => setActiveTab('console')}
                className={`px-3 py-2 border-b-2 font-medium transition flex items-center space-x-1.5 ${
                  activeTab === 'console'
                    ? 'border-cyan-500 text-cyan-400 bg-slate-800/30'
                    : 'border-transparent text-slate-400 hover:text-slate-200'
                }`}
              >
                <Terminal className="w-3.5 h-3.5" />
                <span>Console</span>
              </button>

              <button
                onClick={() => {
                  handleInspectBytecode();
                  setActiveTab('bytecode');
                }}
                className={`px-3 py-2 border-b-2 font-medium transition flex items-center space-x-1.5 ${
                  activeTab === 'bytecode'
                    ? 'border-cyan-500 text-cyan-400 bg-slate-800/30'
                    : 'border-transparent text-slate-400 hover:text-slate-200'
                }`}
              >
                <Binary className="w-3.5 h-3.5" />
                <span>Bytecode</span>
              </button>

              <button
                onClick={() => {
                  handleInspectIr();
                  setActiveTab('ir');
                }}
                className={`px-3 py-2 border-b-2 font-medium transition flex items-center space-x-1.5 ${
                  activeTab === 'ir'
                    ? 'border-cyan-500 text-cyan-400 bg-slate-800/30'
                    : 'border-transparent text-slate-400 hover:text-slate-200'
                }`}
              >
                <GitBranch className="w-3.5 h-3.5" />
                <span>TAC IR</span>
              </button>

              <button
                onClick={() => {
                  handleInspectAst();
                  setActiveTab('ast');
                }}
                className={`px-3 py-2 border-b-2 font-medium transition flex items-center space-x-1.5 ${
                  activeTab === 'ast'
                    ? 'border-cyan-500 text-cyan-400 bg-slate-800/30'
                    : 'border-transparent text-slate-400 hover:text-slate-200'
                }`}
              >
                <Layers className="w-3.5 h-3.5" />
                <span>AST</span>
              </button>

              <button
                onClick={() => setActiveTab('repl')}
                className={`px-3 py-2 border-b-2 font-medium transition flex items-center space-x-1.5 ${
                  activeTab === 'repl'
                    ? 'border-cyan-500 text-cyan-400 bg-slate-800/30'
                    : 'border-transparent text-slate-400 hover:text-slate-200'
                }`}
              >
                <Play className="w-3.5 h-3.5" />
                <span>REPL</span>
              </button>

              <button
                onClick={() => setActiveTab('docs')}
                className={`px-3 py-2 border-b-2 font-medium transition flex items-center space-x-1.5 ${
                  activeTab === 'docs'
                    ? 'border-cyan-500 text-cyan-400 bg-slate-800/30'
                    : 'border-transparent text-slate-400 hover:text-slate-200'
                }`}
              >
                <BookOpen className="w-3.5 h-3.5" />
                <span>Docs</span>
              </button>
            </div>

            {executionTime !== null && (
              <div className="flex items-center space-x-1 text-[11px] text-slate-400 px-2">
                <Clock className="w-3 h-3 text-cyan-400" />
                <span>{executionTime}ms</span>
              </div>
            )}
          </div>

          {/* Tab Content Display */}
          <div className="flex-1 p-3 overflow-y-auto font-mono text-xs">
            {activeTab === 'console' && (
              <div className="space-y-3">
                {/* Diagnostics box */}
                {diagnostics.length > 0 && (
                  <div className="p-3 bg-amber-950/40 border border-amber-800/60 rounded-md text-amber-200 space-y-2">
                    <div className="flex items-center space-x-1.5 font-bold text-amber-400">
                      <AlertTriangle className="w-4 h-4" />
                      <span>Compiler Diagnostics ({diagnostics.length})</span>
                    </div>
                    {diagnostics.map((d, idx) => (
                      <div key={idx} className="pl-4 border-l-2 border-amber-600/60 text-[11px] space-y-1">
                        <div className="font-semibold text-white">{d.message}</div>
                        <div className="text-slate-400">Location: line {d.line}, column {d.column}</div>
                        {d.suggestion && (
                          <div className="text-cyan-300 font-sans italic bg-cyan-950/40 p-1.5 rounded">
                            💡 Suggestion: {d.suggestion}
                          </div>
                        )}
                      </div>
                    ))}
                  </div>
                )}

                {/* Stderr box */}
                {errorOutput && (
                  <div className="p-3 bg-rose-950/40 border border-rose-800/60 rounded-md text-rose-300 whitespace-pre-wrap">
                    {errorOutput}
                  </div>
                )}

                {/* Stdout box */}
                {output ? (
                  <div className="p-3 bg-[#0a0d13] border border-slate-800 rounded-md text-emerald-300 whitespace-pre-wrap leading-5">
                    {output}
                  </div>
                ) : !errorOutput && diagnostics.length === 0 ? (
                  <div className="text-slate-500 italic p-4 text-center">
                    Click "Run Program" or "Check" to execute and inspect results.
                  </div>
                ) : null}
              </div>
            )}

            {activeTab === 'bytecode' && (
              <div>
                <div className="text-[11px] text-slate-400 mb-2 font-sans flex items-center justify-between">
                  <span>Stack-based Virtual Machine Instruction Set:</span>
                  <button onClick={handleInspectBytecode} className="hover:text-cyan-400 text-xs">Refresh</button>
                </div>
                <pre className="p-3 bg-[#0a0d13] border border-slate-800 rounded-md text-cyan-300 whitespace-pre-wrap leading-5 overflow-x-auto">
                  {bytecode || 'Click "Refresh" to disassemble bytecode.'}
                </pre>
              </div>
            )}

            {activeTab === 'ir' && (
              <div>
                <div className="text-[11px] text-slate-400 mb-2 font-sans flex items-center justify-between">
                  <span>Three-Address Code (TAC) with Constant Folding & Dead Code Optimization:</span>
                  <button onClick={handleInspectIr} className="hover:text-cyan-400 text-xs">Refresh</button>
                </div>
                <pre className="p-3 bg-[#0a0d13] border border-slate-800 rounded-md text-purple-300 whitespace-pre-wrap leading-5 overflow-x-auto">
                  {irCode || 'Click "Refresh" to view Intermediate Representation.'}
                </pre>
              </div>
            )}

            {activeTab === 'ast' && (
              <div>
                <div className="text-[11px] text-slate-400 mb-2 font-sans flex items-center justify-between">
                  <span>Abstract Syntax Tree (AST) Hierarchy:</span>
                  <button onClick={handleInspectAst} className="hover:text-cyan-400 text-xs">Refresh</button>
                </div>
                <pre className="p-3 bg-[#0a0d13] border border-slate-800 rounded-md text-indigo-300 whitespace-pre-wrap leading-5 overflow-x-auto">
                  {astCode || 'Click "Refresh" to view Abstract Syntax Tree.'}
                </pre>
              </div>
            )}

            {activeTab === 'repl' && (
              <div className="flex flex-col h-full space-y-2">
                <div className="text-[11px] text-slate-400 font-sans">
                  Interactive HUMA X REPL. Enter expressions or statements:
                </div>

                <div className="flex-1 bg-[#0a0d13] border border-slate-800 rounded-md p-2.5 overflow-y-auto space-y-2">
                  {replHistory.map((h, i) => (
                    <div key={i} className="space-y-0.5">
                      <div className="text-cyan-400">huma&gt; {h.cmd}</div>
                      {h.res && (
                        <div className={h.isError ? 'text-rose-400 pl-4' : 'text-emerald-300 pl-4'}>
                          {h.res}
                        </div>
                      )}
                    </div>
                  ))}
                </div>

                <form onSubmit={handleReplSubmit} className="flex space-x-2 pt-1">
                  <span className="text-cyan-400 font-bold self-center">huma&gt;</span>
                  <input
                    type="text"
                    value={replInput}
                    onChange={e => setReplInput(e.target.value)}
                    placeholder="e.g. make x = 10 * 5"
                    className="flex-1 bg-slate-900 border border-slate-700 rounded px-2.5 py-1 text-slate-200 outline-none focus:border-cyan-500 font-mono text-xs"
                  />
                  <button
                    type="submit"
                    className="px-3 py-1 bg-cyan-600 hover:bg-cyan-500 text-white rounded font-medium text-xs cursor-pointer"
                  >
                    Eval
                  </button>
                </form>
              </div>
            )}

            {activeTab === 'docs' && (
              <div className="font-sans space-y-4 text-slate-300 text-xs leading-relaxed p-1">
                <div className="border-b border-slate-800 pb-2">
                  <h3 className="text-base font-bold text-white">HUMA X Quick Reference</h3>
                  <p className="text-slate-400">Human-Oriented Universal Machine Language</p>
                </div>

                <div>
                  <h4 className="font-bold text-cyan-400 mb-1">1. Variables & Constants</h4>
                  <pre className="bg-slate-900 p-2 rounded text-[11px] text-slate-200 font-mono">
{`make age = 18          # Mutable variable
fixed PI = 3.14159     # Immutable constant
change age to age + 1  # Reassignment`}
                  </pre>
                </div>

                <div>
                  <h4 className="font-bold text-cyan-400 mb-1">2. Conditions</h4>
                  <pre className="bg-slate-900 p-2 rounded text-[11px] text-slate-200 font-mono">
{`if score is greater than 90 then
    show "Grade A"
else if score is greater than 80 then
    show "Grade B"
otherwise
    show "Pass"
end`}
                  </pre>
                </div>

                <div>
                  <h4 className="font-bold text-cyan-400 mb-1">3. Loops</h4>
                  <pre className="bg-slate-900 p-2 rounded text-[11px] text-slate-200 font-mono">
{`repeat 5 times
    show "Hello"
end

repeat for each item in [1, 2, 3]
    show item
end`}
                  </pre>
                </div>

                <div>
                  <h4 className="font-bold text-cyan-400 mb-1">4. Functions</h4>
                  <pre className="bg-slate-900 p-2 rounded text-[11px] text-slate-200 font-mono">
{`create function add using a, b
    give back a + b
end

make res = call add using 10, 20`}
                  </pre>
                </div>

                <div>
                  <h4 className="font-bold text-cyan-400 mb-1">5. Built-in Modules</h4>
                  <ul className="list-disc list-inside space-y-1 text-slate-400">
                    <li><strong className="text-slate-200">huma.math:</strong> sqrt, pow, abs, sin, cos, floor, ceil, round, min, max, random</li>
                    <li><strong className="text-slate-200">huma.text:</strong> to_upper, to_lower, trim, split, join, contains, starts_with</li>
                    <li><strong className="text-slate-200">huma.collections:</strong> push, pop, length, keys, values</li>
                    <li><strong className="text-slate-200">huma.system:</strong> platform(), now(), sleep(ms), clock()</li>
                  </ul>
                </div>
              </div>
            )}
          </div>
        </div>
      </div>

      {/* Mobile & Downloads Modal */}
      {showMobileModal && (
        <div className="fixed inset-0 z-50 bg-black/80 backdrop-blur-sm flex items-center justify-center p-4">
          <div className="bg-[#161b22] border border-slate-700 rounded-xl max-w-2xl w-full max-h-[90vh] flex flex-col shadow-2xl overflow-hidden">
            {/* Modal Header */}
            <div className="px-5 py-4 border-b border-slate-800 flex items-center justify-between bg-[#11161d]">
              <div className="flex items-center space-x-2">
                <span className="text-xl">📱</span>
                <div>
                  <h3 className="text-base font-bold text-white">মোবাইলে HUMA X চালানো ও ডাউনলোড</h3>
                  <p className="text-xs text-slate-400">Run & Download HUMA X on Mobile (Android & iOS)</p>
                </div>
              </div>
              <button
                onClick={() => setShowMobileModal(false)}
                className="text-slate-400 hover:text-white p-1 rounded-md text-lg cursor-pointer"
              >
                ✕
              </button>
            </div>

            {/* Modal Tabs */}
            <div className="flex border-b border-slate-800 bg-[#0e131a] px-4">
              <button
                onClick={() => setMobileTab('runner')}
                className={`py-2.5 px-3 text-xs font-semibold border-b-2 transition cursor-pointer ${
                  mobileTab === 'runner'
                    ? 'border-emerald-500 text-emerald-400'
                    : 'border-transparent text-slate-400 hover:text-slate-200'
                }`}
              >
                ১. অফলাইন মোবাইল রানার (সবচেয়ে সহজ)
              </button>
              <button
                onClick={() => setMobileTab('termux')}
                className={`py-2.5 px-3 text-xs font-semibold border-b-2 transition cursor-pointer ${
                  mobileTab === 'termux'
                    ? 'border-cyan-500 text-cyan-400'
                    : 'border-transparent text-slate-400 hover:text-slate-200'
                }`}
              >
                ২. Android Termux (আসল কম্পাইলার)
              </button>
              <button
                onClick={() => setMobileTab('github')}
                className={`py-2.5 px-3 text-xs font-semibold border-b-2 transition cursor-pointer ${
                  mobileTab === 'github'
                    ? 'border-purple-500 text-purple-400'
                    : 'border-transparent text-slate-400 hover:text-slate-200'
                }`}
              >
                ৩. GitHub Actions প্যাকেজ
              </button>
            </div>

            {/* Modal Body */}
            <div className="p-5 overflow-y-auto space-y-4 text-xs">
              {mobileTab === 'runner' && (
                <div className="space-y-4">
                  <div className="p-3 bg-emerald-950/30 border border-emerald-800/50 rounded-lg text-emerald-200 space-y-1">
                    <div className="font-bold text-sm text-emerald-400">
                      ✓ কোনো ইন্সটলেশন বা ইন্টারনেটের প্রয়োজন নেই!
                    </div>
                    <p className="text-[11px] text-emerald-300/90 leading-relaxed">
                      এটি একটি একক সিঙ্গেল-ফাইল HTML অ্যাপ। এটি একবার ডাউনলোড করে আপনার মোবাইলের Chrome, Samsung Internet বা Safari ব্রাউজারে খুললে ১০০% অফলাইনে কাজ করবে।
                    </p>
                  </div>

                  <div className="space-y-2">
                    <h4 className="font-bold text-white text-xs">কীভাবে মোবাইলে চালু করবেন:</h4>
                    <ol className="list-decimal list-inside space-y-1.5 text-slate-300">
                      <li>নিচের সবুজ বাটনে ক্লিক করে <strong>huma-mobile-runner.html</strong> ডাউনলোড করুন।</li>
                      <li>আপনার মোবাইলের <strong>Files / Downloads</strong> ফোল্ডারে যান।</li>
                      <li>ফাইলটিতে ট্যাপ করে <strong>Chrome / ব্রাউজার</strong> দিয়ে ওপেন করুন।</li>
                      <li>মোবাইল স্ক্রিনের ভার্চুয়াল শর্টকাট দিয়ে কোড লিখুন এবং <strong>▶ চালান</strong> চাপুন!</li>
                    </ol>
                  </div>

                  <div className="pt-2 flex flex-wrap gap-2">
                    <a
                      href="/api/download/runner"
                      download="huma-mobile-runner.html"
                      className="inline-flex items-center space-x-2 px-4 py-2.5 rounded-lg bg-emerald-600 hover:bg-emerald-500 font-bold text-white text-xs shadow-lg shadow-emerald-950 transition cursor-pointer"
                    >
                      <Download className="w-4 h-4" />
                      <span>ডাউনলোড করুন huma-mobile-runner.html</span>
                    </a>

                    <a
                      href="/huma-mobile.html"
                      target="_blank"
                      rel="noreferrer"
                      className="inline-flex items-center space-x-1.5 px-3.5 py-2.5 rounded-lg bg-slate-800 hover:bg-slate-700 text-slate-200 font-semibold text-xs border border-slate-700 transition cursor-pointer"
                    >
                      <span>ব্রাউজারে সরাসরি টেস্ট করুন ↗</span>
                    </a>
                  </div>
                </div>
              )}

              {mobileTab === 'termux' && (
                <div className="space-y-4">
                  <div className="p-3 bg-cyan-950/30 border border-cyan-800/50 rounded-lg text-cyan-200 space-y-1">
                    <div className="font-bold text-sm text-cyan-400">
                      Android Termux দিয়ে ফোনে আসল C++20 কম্পাইলার চালান
                    </div>
                    <p className="text-[11px] text-cyan-300/90 leading-relaxed">
                      Android ফোনে লিনাক্স টার্মিনাল অ্যাপ Termux-এ HUMA X সম্পূর্ণ নেটিভভাবে চলে।
                    </p>
                  </div>

                  <div className="space-y-2">
                    <h4 className="font-bold text-white">Termux এ ১-লাইনের অটো-ইনস্টল কমান্ড:</h4>
                    <p className="text-slate-400 text-[11px]">Termux ওপেন করে এই কমান্ডটি পেস্ট করুন:</p>
                    <pre className="bg-[#0a0d13] p-3 rounded-lg border border-slate-800 text-cyan-300 font-mono text-[11px] whitespace-pre-wrap select-all">
{`pkg update && pkg install -y clang cmake make git
git clone https://github.com/huma-lang/huma-x.git
cd huma-x/huma_lang
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
cp build/huma $PREFIX/bin/
huma version`}
                    </pre>
                  </div>

                  <div className="space-y-2">
                    <h4 className="font-bold text-white">ইনস্টল হওয়ার পর ফোনে চালানোর কমান্ড:</h4>
                    <pre className="bg-[#0a0d13] p-2.5 rounded-lg border border-slate-800 text-emerald-400 font-mono text-[11px]">
{`huma run hello.huma
huma repl`}
                    </pre>
                  </div>

                  <div className="pt-2">
                    <a
                      href="/api/download/mobile"
                      download="huma-x-android-mobile.zip"
                      className="inline-flex items-center space-x-2 px-4 py-2.5 rounded-lg bg-cyan-600 hover:bg-cyan-500 font-bold text-white text-xs shadow-lg shadow-cyan-950 transition cursor-pointer"
                    >
                      <Download className="w-4 h-4" />
                      <span>ডাউনলোড করুন Android Mobile Package (.zip)</span>
                    </a>
                  </div>
                </div>
              )}

              {mobileTab === 'github' && (
                <div className="space-y-4">
                  <div className="p-3 bg-purple-950/30 border border-purple-800/50 rounded-lg text-purple-200 space-y-1">
                    <div className="font-bold text-sm text-purple-400">
                      GitHub Actions Multi-Platform CI/CD
                    </div>
                    <p className="text-[11px] text-purple-300/90 leading-relaxed">
                      গিটহাবে পুশ করলে স্বয়ংক্রিয়ভাবে Android ARM64, Linux, Windows এবং macOS বাইনারি বিল্ড হয়ে রিলিজ ফাইলে যুক্ত হবে।
                    </p>
                  </div>

                  <div className="space-y-2">
                    <h4 className="font-bold text-white">GitHub Actions বিল্ড আর্কিটেকচার:</h4>
                    <ul className="list-disc list-inside space-y-1 text-slate-300 text-[11px]">
                      <li><strong>Android ARM64:</strong> NDK r26b cross-compiled binary</li>
                      <li><strong>Linux x86-64:</strong> GCC 12 native ELF binary</li>
                      <li><strong>Windows x64:</strong> MSVC / GCC huma.exe</li>
                      <li><strong>macOS:</strong> Universal binary (arm64 & x86_64)</li>
                      <li><strong>Web / Mobile:</strong> huma-mobile-offline-runner.html</li>
                    </ul>
                  </div>

                  <div className="p-3 bg-slate-900 border border-slate-800 rounded-lg">
                    <div className="text-[11px] text-slate-400 mb-1">ওয়ার্কফ্লো ফাইলের অবস্থান:</div>
                    <code className="text-cyan-400 font-mono text-[11px]">
                      .github/workflows/build-and-release.yml
                    </code>
                  </div>

                  <div className="pt-2">
                    <a
                      href="/api/download-zip"
                      download="huma-x-complete-v1.0.0.zip"
                      className="inline-flex items-center space-x-2 px-4 py-2.5 rounded-lg bg-indigo-600 hover:bg-indigo-500 font-bold text-white text-xs shadow-lg shadow-indigo-950 transition cursor-pointer"
                    >
                      <Download className="w-4 h-4" />
                      <span>সম্পূর্ণ সোর্স কোড ডাউনলোড (.zip)</span>
                    </a>
                  </div>
                </div>
              )}
            </div>

            {/* Modal Footer */}
            <div className="px-5 py-3 border-t border-slate-800 bg-[#11161d] flex items-center justify-between">
              <span className="text-[11px] text-slate-500">HUMA X Ecosystem v1.0.0</span>
              <button
                onClick={() => setShowMobileModal(false)}
                className="px-4 py-1.5 bg-slate-800 hover:bg-slate-700 text-slate-200 rounded-md text-xs font-semibold transition cursor-pointer"
              >
                বন্ধ করুন
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
