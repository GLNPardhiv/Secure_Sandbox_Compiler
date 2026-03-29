const express = require('express');
const { exec } = require('child_process');
const fs = require('fs').promises;
const path = require('path');
const crypto = require('crypto');

const app = express();
const PORT = 5000;

app.use(express.json());

// Ensure the test_files directory exists
const testDir = path.join(__dirname, 'test_files');
fs.mkdir(testDir, { recursive: true }).catch(console.error);

// Serve the frontend UI
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'templates', 'index.html'));
});

// Handle code execution
app.post('/run', async (req, res) => {
    const { code, mode } = req.body; 

    if (!code || !code.trim()) {
        return res.json({ output: "Error: No code provided." });
    }

    const filename = `web_${crypto.randomUUID().replace(/-/g, '')}.cpp`;
    const filepath = path.join(testDir, filename);
    
    // Default to --run if no mode is selected
    const cliFlag = mode || '--run'; 

    try {
        await fs.writeFile(filepath, code);

        exec(`./sandboxcc ${filepath} ${cliFlag}`, { timeout: 35000 }, async (error, stdout, stderr) => {
            let output = stdout || stderr || "";
            
            if (error && error.killed) {
                output += "\n[!] Error: Execution timed out (exceeded 35 seconds).";
            } else if (error && !output) {
                output += `\n[!] Process Error: ${error.message}`;
            }

            try { await fs.unlink(filepath); } catch (e) {}

            console.log(`\n--- Execution Request ---`);
            console.log(output.trim());
            
            res.json({ output: output.trim() });
        });

    } catch (err) {
        res.json({ output: `[!] Server Error: ${err.message}` });
    }
});

app.listen(PORT, () => {
    console.log(`🚀 Advanced Dashboard running on http://127.0.0.1:${PORT}`);
});