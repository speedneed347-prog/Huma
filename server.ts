import express, { Request, Response } from 'express';
import { exec, spawn } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const port = 3000;

app.use(express.json({ limit: '10mb' }));
app.use(express.static(path.join(__dirname, 'public')));

// Path to huma executable
const HUMA_BIN = fs.existsSync('/usr/local/bin/huma')
  ? '/usr/local/bin/huma'
  : path.join(__dirname, 'huma_lang/build/huma');

// ==================== Cloud Native Compiler API ====================

// GET /api/version
app.get('/api/version', (_req: Request, res: Response) => {
  res.json({
    name: 'HUMA X',
    full_name: 'Human-Oriented Universal Machine Language',
    version: '1.0.0',
    build_date: '2026-11-01',
    compiler_standard: 'C++20',
    host_architecture: 'Linux x86-64',
    status: 'production-ready',
  });
});

// GET /api/targets
app.get('/api/targets', (_req: Request, res: Response) => {
  res.json({
    default_target: 'vm',
    supported_targets: [
      {
        id: 'vm',
        name: 'Bytecode Virtual Machine',
        description: 'Instant stack-based bytecode interpreter with call frames and debug info.',
        supported: true,
      },
      {
        id: 'native',
        name: 'Native Machine Code (ELF x86-64)',
        description: 'Direct compilation to optimized standalone binary using GCC 12 C++20 backend.',
        supported: true,
      },
      {
        id: 'wasm',
        name: 'WebAssembly / JavaScript Bundle',
        description: 'Client-side runtime module with browser and standard library interoperability.',
        supported: true,
      },
    ],
  });
});

// GET /api/health
app.get('/api/health', (_req: Request, res: Response) => {
  const binaryExists = fs.existsSync(HUMA_BIN);
  res.json({
    status: binaryExists ? 'healthy' : 'degraded',
    huma_binary_path: HUMA_BIN,
    available_compilers: {
      gpp: true,
      cmake: true,
      huma_engine: binaryExists,
    },
    timestamp: new Date().toISOString(),
  });
});

// POST /api/check
app.post('/api/check', (req: Request, res: Response) => {
  const { source } = req.body;
  if (typeof source !== 'string') {
    return res.status(400).json({ error: 'Source code string is required' });
  }

  const tempFile = `/tmp/huma_check_${Date.now()}_${Math.random().toString(36).substring(7)}.huma`;
  fs.writeFileSync(tempFile, source);

  exec(`${HUMA_BIN} check "${tempFile}" --json`, (error, stdout, stderr) => {
    fs.unlink(tempFile, () => {});
    try {
      const parsed = JSON.parse(stdout);
      return res.json(parsed);
    } catch {
      return res.json({
        success: !error,
        output: stdout,
        error: stderr,
        diagnostics: [],
      });
    }
  });
});

// POST /api/run
app.post('/api/run', (req: Request, res: Response) => {
  const { source, target = 'vm', timeout_ms = 5000 } = req.body;
  if (typeof source !== 'string') {
    return res.status(400).json({ error: 'Source code string is required' });
  }

  const tempFile = `/tmp/huma_run_${Date.now()}_${Math.random().toString(36).substring(7)}.huma`;
  fs.writeFileSync(tempFile, source);

  const startTime = Date.now();
  const cmd = `${HUMA_BIN} run "${tempFile}" --target ${target} --json`;

  exec(cmd, { timeout: Math.min(timeout_ms, 10000) }, (error, stdout, stderr) => {
    fs.unlink(tempFile, () => {});
    const executionTimeMs = Date.now() - startTime;

    try {
      const parsed = JSON.parse(stdout);
      return res.json({
        status: parsed.status,
        stdout: parsed.stdout,
        stderr: parsed.stderr || stderr,
        target,
        execution_time_ms: executionTimeMs,
        exit_code: error ? (error.code ?? 1) : 0,
      });
    } catch {
      return res.json({
        status: error ? 'error' : 'success',
        stdout,
        stderr,
        target,
        execution_time_ms: executionTimeMs,
        exit_code: error ? (error.code ?? 1) : 0,
      });
    }
  });
});

// POST /api/compile
app.post('/api/compile', (req: Request, res: Response) => {
  const { source, target = 'native' } = req.body;
  if (typeof source !== 'string') {
    return res.status(400).json({ error: 'Source code string is required' });
  }

  const tempSrc = `/tmp/huma_comp_${Date.now()}_${Math.random().toString(36).substring(7)}.huma`;
  const tempOut = `/tmp/huma_out_${Date.now()}_${Math.random().toString(36).substring(7)}` +
    (target === 'wasm' ? '.js' : target === 'bytecode' ? '.humac' : '.bin');

  fs.writeFileSync(tempSrc, source);

  const startTime = Date.now();
  const cmd = `${HUMA_BIN} compile "${tempSrc}" --target ${target} -o "${tempOut}"`;

  exec(cmd, { timeout: 15000 }, (error, stdout, stderr) => {
    fs.unlink(tempSrc, () => {});
    const compilationTimeMs = Date.now() - startTime;

    if (error) {
      return res.status(422).json({
        success: false,
        error: stderr || stdout,
        target,
        compilation_time_ms: compilationTimeMs,
      });
    }

    let codeOutput = '';
    if (fs.existsSync(tempOut) && (target === 'wasm' || target === 'bytecode')) {
      codeOutput = fs.readFileSync(tempOut, 'utf-8');
      fs.unlink(tempOut, () => {});
    }

    return res.json({
      success: true,
      target,
      compilation_time_ms: compilationTimeMs,
      message: `Successfully compiled HUMA source to target '${target}'`,
      details: stdout,
      compiled_code: codeOutput || undefined,
    });
  });
});

// POST /api/format
app.post('/api/format', (req: Request, res: Response) => {
  const { source } = req.body;
  if (typeof source !== 'string') {
    return res.status(400).json({ error: 'Source code is required' });
  }

  const tempFile = `/tmp/huma_fmt_${Date.now()}.huma`;
  fs.writeFileSync(tempFile, source);

  exec(`${HUMA_BIN} format "${tempFile}"`, (error, stdout, stderr) => {
    fs.unlink(tempFile, () => {});
    if (error) {
      return res.json({ formatted: source, error: stderr });
    }
    return res.json({ formatted: stdout });
  });
});

// POST /api/ir
app.post('/api/ir', (req: Request, res: Response) => {
  const { source } = req.body;
  const tempFile = `/tmp/huma_ir_${Date.now()}.huma`;
  fs.writeFileSync(tempFile, source || '');

  exec(`${HUMA_BIN} dump-ir "${tempFile}"`, (error, stdout, stderr) => {
    fs.unlink(tempFile, () => {});
    return res.json({ ir: stdout, error: stderr });
  });
});

// POST /api/bytecode
app.post('/api/bytecode', (req: Request, res: Response) => {
  const { source } = req.body;
  const tempFile = `/tmp/huma_bc_${Date.now()}.huma`;
  fs.writeFileSync(tempFile, source || '');

  exec(`${HUMA_BIN} dump-bytecode "${tempFile}"`, (error, stdout, stderr) => {
    fs.unlink(tempFile, () => {});
    return res.json({ bytecode: stdout, error: stderr });
  });
});

// POST /api/ast
app.post('/api/ast', (req: Request, res: Response) => {
  const { source } = req.body;
  const tempFile = `/tmp/huma_ast_${Date.now()}.huma`;
  fs.writeFileSync(tempFile, source || '');

  exec(`${HUMA_BIN} dump-ast "${tempFile}"`, (error, stdout, stderr) => {
    fs.unlink(tempFile, () => {});
    return res.json({ ast: stdout, error: stderr });
  });
});

// GET /api/examples
app.get('/api/examples', (_req: Request, res: Response) => {
  const examplesDir = path.join(__dirname, 'huma_lang/examples');
  if (!fs.existsSync(examplesDir)) {
    return res.json({ examples: [] });
  }

  const files = fs.readdirSync(examplesDir).filter(f => f.endsWith('.huma'));
  const examples = files.map(file => {
    const content = fs.readFileSync(path.join(examplesDir, file), 'utf-8');
    const title = file.replace('.huma', '').replace(/_/g, ' ');
    return {
      id: file,
      name: title.charAt(0).toUpperCase() + title.slice(1),
      code: content,
    };
  });

  res.json({ examples });
});

// GET /api/download-zip
app.get('/api/download-zip', (_req: Request, res: Response) => {
  const zipPath = path.join(__dirname, 'public/huma-x-v1.0.0.zip');
  if (fs.existsSync(zipPath)) {
    res.download(zipPath, 'huma-x-v1.0.0-source.zip');
  } else {
    res.status(404).json({ error: 'Zip archive not found' });
  }
});

// Setup Vite middleware or static serving
async function setupServer() {
  if (process.env.NODE_ENV !== 'production') {
    const { createServer } = await import('vite');
    const vite = await createServer({
      server: { middlewareMode: true },
      appType: 'spa',
    });
    app.use(vite.middlewares);
  } else {
    app.use(express.static(path.join(__dirname, 'dist')));
    app.get('*', (_req: Request, res: Response) => {
      res.sendFile(path.join(__dirname, 'dist/index.html'));
    });
  }

  app.listen(port, '0.0.0.0', () => {
    console.log(`HUMA X Server & Cloud Native Compiler running at http://0.0.0.0:${port}`);
  });
}

setupServer();
