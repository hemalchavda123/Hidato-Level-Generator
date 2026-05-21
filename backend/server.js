const express = require('express');
const cors = require('cors');
const { spawn } = require('child_process');
const path = require('path');

const app = express();
const port = 3000;

app.use(cors());
app.use(express.json());

// Serve static files from the frontend directory
app.use(express.static(path.join(__dirname, '../frontend')));

app.post('/api/generate', (req, res) => {
    const { cells, difficulty } = req.body;
    
    if (!cells || difficulty === undefined) {
        return res.status(400).json({ error: 'Missing cells or difficulty' });
    }

    const enginePath = path.join(__dirname, '../engine/hidato_engine.exe');
    
    const engine = spawn(enginePath, [cells.toString(), difficulty.toString(), '--json']);
    
    let outputData = '';
    let errorData = '';

    engine.stdout.on('data', (data) => {
        outputData += data.toString();
    });

    engine.stderr.on('data', (data) => {
        errorData += data.toString();
    });

    engine.on('close', (code) => {
        if (code !== 0) {
            console.error('Engine error:', errorData);
            return res.status(500).json({ error: 'Engine failed to generate puzzle', details: errorData });
        }
        
        try {
            // Find the JSON part in case there's extra output (though we shouldn't have any with --json)
            const jsonStart = outputData.indexOf('{');
            const jsonEnd = outputData.lastIndexOf('}');
            if (jsonStart !== -1 && jsonEnd !== -1) {
                const jsonStr = outputData.substring(jsonStart, jsonEnd + 1);
                const puzzleData = JSON.parse(jsonStr);
                res.json(puzzleData);
            } else {
                throw new Error("No JSON found in output");
            }
        } catch (err) {
            console.error('Failed to parse engine output:', err, '\nOutput:', outputData);
            res.status(500).json({ error: 'Failed to parse engine output' });
        }
    });
});

app.listen(port, () => {
    console.log(`Backend server running at http://localhost:${port}`);
});
