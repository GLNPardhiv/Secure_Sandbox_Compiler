import React, { useState, useEffect, useRef } from 'react';
import CodeMirror from '@uiw/react-codemirror';
import { cpp } from '@codemirror/lang-cpp';
import { vscodeDark } from '@uiw/codemirror-theme-vscode';
import { Play, FileCode2, Terminal, Shield, Activity, Trash2, Plus, Edit2, MoreVertical, X, Check, ExternalLink, Zap, Lock } from 'lucide-react';

export default function App() {
  const [files, setFiles] = useState({});
  const [currentFile, setCurrentFile] = useState('main.cpp');
  const [code, setCode] = useState('');
  const [output, setOutput] = useState('Sandbox initialized. Awaiting payload...');
  const [isRunning, setIsRunning] = useState(false);
  const [isSaved, setIsSaved] = useState(true);
  const [executionTime, setExecutionTime] = useState(0);
  
  // Resizable panel widths
  const [sidebarWidth, setSidebarWidth] = useState(300);
  const [rightPanelWidth, setRightPanelWidth] = useState(450);
  const [telemetryHeight, setTelemetryHeight] = useState(50);
  
  // Drag state
  const [isDragging, setIsDragging] = useState(null);
  const containerRef = useRef(null);
  const autoSaveTimeoutRef = useRef(null);

  // File management UI
  const [renameFile, setRenameFile] = useState(null);
  const [newFileName, setNewFileName] = useState('');
  const [contextMenu, setContextMenu] = useState(null);
  const [showNewFileDialog, setShowNewFileDialog] = useState(false);
  const [newFileInput, setNewFileInput] = useState('');

  // Telemetry States
  const [tiers, setTiers] = useState([
    { id: 1, name: 'Heuristics', status: 'idle', desc: 'Fast-Pass Analysis', icon: '⚡' },
    { id: 2, name: 'AI Scanner', status: 'idle', desc: 'Gemini Semantic Check', icon: '🧠' },
    { id: 3, name: 'Kernel', status: 'idle', desc: 'Seccomp-BPF Sandbox', icon: '🔒' },
    { id: 4, name: 'AI Tutor', status: 'idle', desc: 'Post-Mortem Diagnostics', icon: '🤖' }
  ]);

  // Initialize files from localStorage
  useEffect(() => {
    const savedFiles = JSON.parse(localStorage.getItem('sandbox_files')) || {};
    if (Object.keys(savedFiles).length === 0) {
      savedFiles['main.cpp'] = '#include <iostream>\n\nint main() {\n    std::cout << "System ready.";\n    return 0;\n}';
    }
    setFiles(savedFiles);
    const firstFile = Object.keys(savedFiles)[0];
    setCurrentFile(firstFile);
    setCode(savedFiles[firstFile]);
    setIsSaved(true);
  }, []);

  // Auto-save on idle
  useEffect(() => {
    if (autoSaveTimeoutRef.current) {
      clearTimeout(autoSaveTimeoutRef.current);
    }

    if (!isSaved) {
      autoSaveTimeoutRef.current = setTimeout(() => {
        const newFiles = { ...files, [currentFile]: code };
        setFiles(newFiles);
        localStorage.setItem('sandbox_files', JSON.stringify(newFiles));
        setIsSaved(true);
      }, 1500);
    }

    return () => {
      if (autoSaveTimeoutRef.current) {
        clearTimeout(autoSaveTimeoutRef.current);
      }
    };
  }, [code, isSaved, currentFile, files]);

  // Handle mouse move for dragging
  useEffect(() => {
    const handleMouseMove = (e) => {
      if (!isDragging) return;

      if (isDragging === 'sidebar') {
        const newWidth = e.clientX;
        if (newWidth > 200 && newWidth < 600) {
          setSidebarWidth(newWidth);
        }
      } else if (isDragging === 'rightPanel') {
        const newWidth = window.innerWidth - e.clientX;
        if (newWidth > 300 && newWidth < 800) {
          setRightPanelWidth(newWidth);
        }
      } else if (isDragging === 'telemetry') {
        const container = containerRef.current;
        if (container) {
          const rect = container.getBoundingClientRect();
          const newHeight = ((e.clientY - rect.top) / rect.height) * 100;
          if (newHeight > 20 && newHeight < 80) {
            setTelemetryHeight(newHeight);
          }
        }
      }
    };

    const handleMouseUp = () => {
      setIsDragging(null);
    };

    if (isDragging) {
      document.addEventListener('mousemove', handleMouseMove);
      document.addEventListener('mouseup', handleMouseUp);
      return () => {
        document.removeEventListener('mousemove', handleMouseMove);
        document.removeEventListener('mouseup', handleMouseUp);
      };
    }
  }, [isDragging]);

  // File operations
  const loadFile = (fileName) => {
    setCode(files[fileName]);
    setCurrentFile(fileName);
    setIsSaved(true);
    setContextMenu(null);
    setRenameFile(null);
  };

  const createNewFile = () => {
    let fileName = newFileInput.trim();
    if (!fileName) return;
    if (!fileName.endsWith('.cpp')) fileName += '.cpp';
    if (files[fileName]) {
      alert('File already exists!');
      return;
    }

    const newFiles = { ...files, [fileName]: '#include <iostream>\n\nint main() {\n    return 0;\n}' };
    setFiles(newFiles);
    localStorage.setItem('sandbox_files', JSON.stringify(newFiles));
    setCurrentFile(fileName);
    setCode(newFiles[fileName]);
    setIsSaved(true);
    setShowNewFileDialog(false);
    setNewFileInput('');
  };

  const handleRename = (fileName) => {
    setRenameFile(fileName);
    setNewFileName(fileName);
  };

  const cancelRename = () => {
    setRenameFile(null);
    setNewFileName('');
  };

  const confirmRename = (oldName) => {
    let finalName = newFileName.trim();
    
    if (!finalName || finalName === oldName) {
      cancelRename();
      return;
    }
    
    if (!finalName.endsWith('.cpp')) {
      finalName += '.cpp';
    }
    
    if (finalName === oldName) {
      cancelRename();
      return;
    }
    
    if (files[finalName]) {
      alert('File name already exists!');
      return;
    }

    const newFiles = { ...files };
    newFiles[finalName] = newFiles[oldName];
    delete newFiles[oldName];
    setFiles(newFiles);
    localStorage.setItem('sandbox_files', JSON.stringify(newFiles));

    if (currentFile === oldName) {
      setCurrentFile(finalName);
    }
    
    cancelRename();
    setContextMenu(null);
  };

  const deleteFile = (fileName) => {
    if (!confirm(`Delete ${fileName}?`)) return;

    const newFiles = { ...files };
    delete newFiles[fileName];
    setFiles(newFiles);
    localStorage.setItem('sandbox_files', JSON.stringify(newFiles));

    if (currentFile === fileName) {
      const remainingFiles = Object.keys(newFiles);
      if (remainingFiles.length > 0) {
        setCurrentFile(remainingFiles[0]);
        setCode(newFiles[remainingFiles[0]]);
        setIsSaved(true);
      }
    }
    setContextMenu(null);
  };

  // Update telemetry
  const updateTelemetry = (outText) => {
    const lower = outText.toLowerCase();
    
    // Always reset to idle first
    let newTiers = [
      { id: 1, name: 'Heuristics', status: 'idle', desc: 'Fast-Pass Analysis' },
      { id: 2, name: 'AI Scanner', status: 'idle', desc: 'Gemini Semantic Check' },
      { id: 3, name: 'Kernel', status: 'idle', desc: 'Seccomp-BPF Sandbox' },
      { id: 4, name: 'AI Tutor', status: 'idle', desc: 'Post-Mortem Diagnostics' }
    ];

    // --- TIER 1: HEURISTICS ---
    if (lower.includes("fast-fail") || lower.includes("blocked by local")) {
      newTiers[0].status = 'danger'; // Hard blocked
      setTiers(newTiers);
      return; // Stop processing further tiers
    } else if (lower.includes("local heuristic") || lower.includes("invoking ai")) {
      newTiers[0].status = 'success'; // Passed
    }

    // --- TIER 2: AI SCANNER ---
    if (lower.includes("blocked by ai")) {
      newTiers[1].status = 'danger'; // AI Blocked it
      setTiers(newTiers);
      return; // Stop processing further tiers
    } else if (lower.includes("invoking ai")) {
      newTiers[1].status = 'success'; // AI Scanned and Passed it
    } else if (lower.includes("skipping ai")) {
      newTiers[1].status = 'idle'; // Completely bypassed to save tokens!
    }

    // --- TIER 3: KERNEL SANDBOX ---
    if (lower.includes("signal 31") || lower.includes("signal 11") || lower.includes("signal 9") || lower.includes("terminated by signal")) {
      newTiers[2].status = 'danger'; // Sandbox killed it
    } else if (lower.includes("execution status") || lower.includes("sandbox interpretation")) {
      newTiers[2].status = 'success'; // Ran successfully
    }

    // --- TIER 4: AI TUTOR ---
    // The Tutor ONLY runs if Tier 3 crashed, or if compilation failed
    if (lower.includes("ai tutor") || lower.includes("crash analysis") || lower.includes("compiler help")) {
      newTiers[3].status = 'warning'; // Lights up Yellow to show active help
    }

    setTiers(newTiers);
  };

  // Run code
  const runCode = async () => {
    setIsRunning(true);
    setExecutionTime(0);
    setTiers(tiers.map(t => ({ ...t, status: 'idle' })));
    setOutput('[*] Initializing Secure Pipeline...\n[*] Uploading payload to host...');

    const startTime = performance.now();

    try {
      const res = await fetch('/run', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ code, mode: '--run' })
      });
      const data = await res.json();
      setOutput(data.output);
      updateTelemetry(data.output);
    } catch (err) {
      setOutput('[!] Fatal Error: Connection to Kernel lost.\n[!] Details: ' + err.message);
    }
    
    const endTime = performance.now();
    setExecutionTime(endTime - startTime);
    setIsRunning(false);
  };

  const getStatusColor = (status) => {
    switch(status) {
      case 'success': return 'from-emerald-900/40 to-emerald-800/20 border-emerald-600/50 text-emerald-300';
      case 'danger': return 'from-rose-900/40 to-rose-800/20 border-rose-600/50 text-rose-300';
      case 'warning': return 'from-amber-900/40 to-amber-800/20 border-amber-600/50 text-amber-300';
      default: return 'from-slate-900/40 to-slate-800/20 border-slate-700/50 text-slate-400';
    }
  };

  return (
    <div className="h-screen flex flex-col bg-gradient-to-br from-[#0a0e27] via-[#0d1117] to-[#0a0e27] text-slate-300 font-sans selection:bg-indigo-500/40 overflow-hidden">
      
      {/* HEADER */}
      <header className="h-16 border-b border-indigo-600/20 flex items-center justify-between px-6 bg-gradient-to-r from-slate-900/80 via-indigo-900/20 to-slate-900/80 backdrop-blur-xl shadow-2xl">
        <div className="flex items-center gap-4">
          <div className="p-2.5 rounded-xl bg-gradient-to-br from-indigo-600/30 to-purple-600/20 border border-indigo-500/40 shadow-lg shadow-indigo-500/10">
            <Shield className="text-indigo-300" size={26} />
          </div>
          <div>
            <h1 className="font-black text-xl tracking-tight text-transparent bg-clip-text bg-gradient-to-r from-indigo-300 via-cyan-300 to-purple-300">
              DefenseOS Sandbox
            </h1>
            <p className="text-xs text-slate-500 font-medium">Advanced C++ Execution & Analysis</p>
          </div>
        </div>
        <div className="flex gap-3">
          <button 
            onClick={() => setShowNewFileDialog(true)}
            className="flex items-center gap-2 px-4 py-2.5 rounded-lg bg-gradient-to-r from-slate-700/50 to-slate-800/50 hover:from-slate-600/50 hover:to-slate-700/50 border border-slate-600/40 text-slate-300 hover:text-slate-200 transition font-medium text-sm shadow-lg hover:shadow-xl backdrop-blur-sm"
          >
            <Plus size={16} /> New File
          </button>
          <div className="flex items-center gap-2 px-4 py-2.5 rounded-lg bg-gradient-to-r from-slate-700/50 to-slate-800/50 border border-slate-600/40 text-slate-300 font-medium text-sm">
            <span className={`w-2 h-2 rounded-full transition-all ${isSaved ? 'bg-emerald-400' : 'bg-amber-400 animate-pulse'}`}></span>
            {isSaved ? 'All Saved' : 'Saving...'}
          </div>
          <button 
            onClick={runCode} 
            disabled={isRunning}
            className={`flex items-center gap-2 px-6 py-2.5 rounded-lg text-sm font-bold text-white transition border shadow-2xl backdrop-blur-sm ${
              isRunning 
                ? 'bg-gradient-to-r from-indigo-900/60 to-purple-900/60 border-indigo-700/30 cursor-not-allowed opacity-70' 
                : 'bg-gradient-to-r from-indigo-600 to-purple-600 hover:from-indigo-500 hover:to-purple-500 border-indigo-500/50 shadow-indigo-500/30 hover:shadow-indigo-400/40'
            }`}
          >
            <Play size={16} /> {isRunning ? 'Executing...' : 'Execute'}
          </button>
        </div>
      </header>

      {/* MAIN LAYOUT */}
      <div ref={containerRef} className="flex flex-1 overflow-hidden relative">
        
        {/* SIDEBAR - FILES */}
        <div 
          style={{ width: `${sidebarWidth}px` }} 
          className="border-r border-indigo-600/20 bg-slate-900/40 backdrop-blur-xl flex flex-col overflow-hidden shadow-xl"
        >
          <div className="p-4 border-b border-indigo-600/20 bg-gradient-to-r from-slate-900/50 to-indigo-900/20">
            <h3 className="text-xs font-black text-indigo-300 uppercase tracking-widest">📁 Project Files</h3>
            <p className="text-xs text-slate-600 mt-1">{Object.keys(files).length} file{Object.keys(files).length !== 1 ? 's' : ''}</p>
          </div>
          
          <div className="flex-1 overflow-y-auto p-3 space-y-2">
            {Object.keys(files).length === 0 ? (
              <div className="text-center py-12 text-slate-600 text-xs">
                <FileCode2 size={32} className="mx-auto mb-3 opacity-30" />
                <p>No files yet</p>
              </div>
            ) : (
              Object.keys(files).map(fileName => (
                <div key={fileName}>
                  {renameFile === fileName ? (
                    <div className="flex gap-2 px-3 py-2 bg-slate-800/60 rounded-lg border border-indigo-500/30">
                      <input
                        type="text"
                        value={newFileName}
                        onChange={(e) => setNewFileName(e.target.value)}
                        className="flex-1 px-2 py-1 bg-slate-900/80 text-white rounded text-xs border border-slate-700/50 focus:outline-none focus:border-indigo-500/60 focus:ring-1 focus:ring-indigo-500/30 placeholder-slate-600"
                        autoFocus
                        onKeyDown={(e) => {
                          if (e.key === 'Enter') confirmRename(fileName);
                          if (e.key === 'Escape') cancelRename();
                        }}
                      />
                      <button 
                        onClick={() => confirmRename(fileName)}
                        className="text-emerald-400 hover:text-emerald-300 transition p-1 hover:bg-slate-700/50 rounded"
                        title="Confirm"
                      >
                        <Check size={14} />
                      </button>
                      <button 
                        onClick={cancelRename}
                        className="text-rose-400 hover:text-rose-300 transition p-1 hover:bg-slate-700/50 rounded"
                        title="Cancel"
                      >
                        <X size={14} />
                      </button>
                    </div>
                  ) : (
                    <div 
                      className={`flex items-center justify-between px-3 py-2.5 rounded-lg cursor-pointer text-xs transition group border backdrop-blur-sm ${
                        currentFile === fileName 
                          ? 'bg-gradient-to-r from-indigo-600/40 to-purple-600/30 text-indigo-200 border-indigo-500/50 shadow-lg shadow-indigo-500/10' 
                          : 'hover:bg-slate-800/40 text-slate-400 hover:text-slate-200 border-transparent hover:border-slate-700/30'
                      }`}
                      onClick={() => loadFile(fileName)}
                      onContextMenu={(e) => {
                        e.preventDefault();
                        setContextMenu({ fileName, x: e.clientX, y: e.clientY });
                      }}
                    >
                      <div className="flex items-center gap-2 flex-1 truncate">
                        <FileCode2 size={14} className="flex-shrink-0 text-indigo-400" />
                        <span className="truncate font-medium">{fileName}</span>
                      </div>
                      <button
                        onClick={(e) => {
                          e.stopPropagation();
                          setContextMenu({ fileName, x: e.clientX, y: e.clientY });
                        }}
                        className="opacity-0 group-hover:opacity-100 transition p-1 hover:bg-slate-700/50 rounded"
                      >
                        <MoreVertical size={12} />
                      </button>
                    </div>
                  )}
                </div>
              ))
            )}
          </div>
        </div>

        {/* SIDEBAR DIVIDER */}
        <div
          onMouseDown={() => setIsDragging('sidebar')}
          className="w-1.5 bg-gradient-to-b from-indigo-600/20 via-purple-600/20 to-indigo-600/20 hover:from-indigo-500/60 hover:via-purple-500/60 hover:to-indigo-500/60 cursor-col-resize transition-colors group relative"
        >
          <div className="absolute inset-0 w-3 -left-1 hover:bg-indigo-500/10 transition-colors" />
        </div>

        {/* EDITOR SECTION */}
        <div className="flex-1 flex flex-col min-w-0 border-r border-indigo-600/20 relative">
          <div className="h-12 bg-gradient-to-r from-slate-900/50 to-indigo-900/20 border-b border-indigo-600/20 flex items-center px-4 gap-3 backdrop-blur-sm shadow-lg">
            <div className="flex items-center gap-2 flex-1">
              <FileCode2 size={16} className="text-indigo-400" />
              <span className="text-sm text-slate-300 font-mono font-semibold">{currentFile}</span>
              <span className="text-xs text-slate-600">•</span>
              <span className="text-xs text-slate-500">{code.split('\n').length} lines</span>
            </div>
            {!isSaved && (
              <div className="flex items-center gap-1.5 px-2 py-1 bg-amber-600/20 border border-amber-500/40 rounded-md">
                <span className="w-1.5 h-1.5 bg-amber-400 rounded-full animate-pulse"></span>
                <span className="text-xs text-amber-300 font-medium">Saving...</span>
              </div>
            )}
          </div>
          <div className="flex-1 overflow-hidden bg-[#1e1e1e] editor-container relative">
            <CodeMirror
              value={code}
              height="100%"
              extensions={[cpp()]}
              theme={vscodeDark}
              onChange={(value) => {
                setCode(value);
                setIsSaved(false);
              }}
              className="text-sm h-full"
            />
          </div>
        </div>

        {/* RIGHT DIVIDER */}
        <div
          onMouseDown={() => setIsDragging('rightPanel')}
          className="w-1.5 bg-gradient-to-b from-indigo-600/20 via-cyan-600/20 to-indigo-600/20 hover:from-indigo-500/60 hover:via-cyan-500/60 hover:to-indigo-500/60 cursor-col-resize transition-colors group relative"
        >
          <div className="absolute inset-0 w-3 -right-1 hover:bg-indigo-500/10 transition-colors" />
        </div>

        {/* RIGHT PANEL */}
        <div 
          style={{ width: `${rightPanelWidth}px` }} 
          className="flex flex-col bg-slate-900/40 backdrop-blur-xl border-l border-indigo-600/20 overflow-hidden shadow-2xl"
        >
          
          {/* TELEMETRY */}
          <div 
            style={{ height: `${telemetryHeight}%` }}
            className="border-b border-indigo-600/20 bg-gradient-to-b from-slate-900/50 to-transparent p-4 flex flex-col overflow-hidden"
          >
            <div className="flex items-center gap-2 mb-4 text-xs font-black text-cyan-300 uppercase tracking-widest flex-shrink-0">
              <Activity size={16} /> Pipeline Status
            </div>
            <div className="grid grid-cols-2 gap-3 flex-1 overflow-y-auto">
              {tiers.map(tier => (
                <div 
                  key={tier.id} 
                  className={`flex flex-col p-4 rounded-xl border transition-all duration-500 backdrop-blur-sm bg-gradient-to-br ${getStatusColor(tier.status)} hover:shadow-lg hover:shadow-current/20 group`}
                >
                  <div className="flex items-start justify-between mb-3">
                    <div className="flex items-center gap-2">
                      <span className="text-2xl">{tier.icon}</span>
                      <div>
                        <h4 className="font-bold text-sm leading-tight">Tier {tier.id}</h4>
                        <p className="text-xs font-semibold leading-tight">{tier.name}</p>
                      </div>
                    </div>
                  </div>
                  <p className="text-xs opacity-90 leading-snug mb-3 line-clamp-2">{tier.desc}</p>
                  <div className="h-1.5 bg-black/40 rounded-full overflow-hidden">
                    <div 
                      className={`h-full transition-all duration-500 rounded-full ${
                        tier.status === 'success' ? 'bg-gradient-to-r from-emerald-500 to-emerald-400 w-full' :
                        tier.status === 'danger' ? 'bg-gradient-to-r from-rose-500 to-rose-400 w-full' :
                        tier.status === 'warning' ? 'bg-gradient-to-r from-amber-500 to-amber-400 w-full' :
                        'bg-gradient-to-r from-slate-600 to-slate-500 w-0'
                      }`}
                    />
                  </div>
                </div>
              ))}
            </div>
          </div>

          {/* DIVIDER */}
          <div
            onMouseDown={() => setIsDragging('telemetry')}
            className="h-1.5 bg-gradient-to-r from-indigo-600/20 via-cyan-600/20 to-indigo-600/20 hover:from-indigo-500/60 hover:via-cyan-500/60 hover:to-indigo-500/60 cursor-row-resize transition-colors group relative"
          >
            <div className="absolute inset-0 h-3 -top-1 hover:bg-indigo-500/10 transition-colors" />
          </div>

          {/* TERMINAL */}
          <div className="flex-1 flex flex-col overflow-hidden">
            <div className="h-12 bg-gradient-to-r from-slate-900/50 to-cyan-900/20 border-b border-indigo-600/20 flex items-center px-4 gap-2 flex-shrink-0 backdrop-blur-sm justify-between">
              <div className="flex items-center gap-2">
                <Terminal size={16} className="text-cyan-400" />
                <span className="text-xs font-black text-cyan-300 uppercase tracking-widest">Host Terminal</span>
              </div>
              {executionTime > 0 && (
                <span className="text-xs text-slate-500 font-mono">{(executionTime).toFixed(2)}ms</span>
              )}
            </div>
            <pre className="flex-1 p-4 text-[12px] font-mono text-emerald-300 overflow-auto whitespace-pre-wrap break-words bg-gradient-to-br from-slate-950/80 to-slate-900/50 selection:bg-emerald-500/40 scrollbar-thin scrollbar-thumb-slate-700 scrollbar-track-slate-900/50">
              {output}
            </pre>
          </div>
        </div>

        {/* CONTEXT MENU */}
        {contextMenu && (
          <div
            className="fixed bg-slate-800/95 border border-slate-700/50 rounded-lg shadow-2xl z-50 backdrop-blur-xl"
            style={{ top: `${contextMenu.y}px`, left: `${contextMenu.x}px` }}
            onMouseLeave={() => setContextMenu(null)}
          >
            <button
              onClick={() => handleRename(contextMenu.fileName)}
              className="w-full text-left px-4 py-2.5 text-sm text-slate-300 hover:bg-indigo-600/30 hover:text-indigo-300 transition flex items-center gap-2 font-medium rounded-t-lg"
            >
              <Edit2 size={16} /> Rename
            </button>
            <button
              onClick={() => deleteFile(contextMenu.fileName)}
              className="w-full text-left px-4 py-2.5 text-sm text-slate-300 hover:bg-rose-600/30 hover:text-rose-300 transition flex items-center gap-2 font-medium border-t border-slate-700/30 rounded-b-lg"
            >
              <Trash2 size={16} /> Delete
            </button>
          </div>
        )}

        {/* NEW FILE DIALOG */}
        {showNewFileDialog && (
          <div className="fixed inset-0 bg-black/60 flex items-center justify-center z-50 backdrop-blur-sm">
            <div className="bg-gradient-to-br from-slate-800 to-slate-900 border border-indigo-600/30 rounded-xl shadow-2xl p-6 w-96 backdrop-blur-xl">
              <h3 className="text-lg font-bold text-indigo-300 mb-4">Create New File</h3>
              <input
                type="text"
                value={newFileInput}
                onChange={(e) => setNewFileInput(e.target.value)}
                placeholder="filename.cpp"
                className="w-full px-4 py-2.5 bg-slate-900/80 text-white rounded-lg border border-slate-700/50 focus:outline-none focus:border-indigo-500/60 focus:ring-1 focus:ring-indigo-500/30 placeholder-slate-600 font-mono text-sm mb-4"
                onKeyDown={(e) => {
                  if (e.key === 'Enter') createNewFile();
                  if (e.key === 'Escape') setShowNewFileDialog(false);
                }}
                autoFocus
              />
              <div className="flex gap-3">
                <button
                  onClick={createNewFile}
                  className="flex-1 px-4 py-2.5 bg-gradient-to-r from-indigo-600 to-purple-600 hover:from-indigo-500 hover:to-purple-500 text-white rounded-lg font-bold text-sm transition shadow-lg"
                >
                  Create
                </button>
                <button
                  onClick={() => setShowNewFileDialog(false)}
                  className="flex-1 px-4 py-2.5 bg-slate-700/50 hover:bg-slate-600/50 text-slate-300 rounded-lg font-bold text-sm transition border border-slate-600/50"
                >
                  Cancel
                </button>
              </div>
            </div>
          </div>
        )}
      </div>

      {/* FOOTER */}
      <footer className="h-10 border-t border-indigo-600/20 bg-gradient-to-r from-slate-900/60 via-slate-900/40 to-slate-900/60 backdrop-blur-xl flex items-center justify-between px-6 shadow-2xl">
        <div className="flex items-center gap-6">
          <div className="flex items-center gap-2 text-xs text-slate-500 font-medium">
            <Lock size={12} className="text-indigo-400" />
            Secure Sandbox
          </div>
          <div className="flex items-center gap-2 text-xs text-slate-500 font-medium">
            <Zap size={12} className="text-cyan-400" />
            {Object.keys(files).length} Files
          </div>
          <div className="text-xs text-slate-600">
            {currentFile} • {code.length} bytes
          </div>
        </div>
        
        <div className="flex items-center gap-4">
          <div className="text-xs text-slate-600">
            Execution: {executionTime > 0 ? `${executionTime.toFixed(2)}ms` : '—'}
          </div>
          <div className="w-px h-5 bg-slate-700/50"></div>
          <div className="flex items-center gap-2">
            <span className="text-xs text-slate-600">v1.0.0</span>
            <a href="#" className="text-slate-500 hover:text-indigo-400 transition">
              <ExternalLink size={14} />
            </a>
          </div>
        </div>
      </footer>
    </div>
  );
}