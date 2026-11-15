const express = require('express');
const path = require('path');

const app = express();
const PORT = 80;

// Serve HTML, CSS, JS, images, etc.
app.use(express.static(path.join(__dirname, 'public')));

// Health check for Docker
app.get('/health', (req, res) => {
    res.sendStatus(200);
});

app.listen(PORT, '0.0.0.0', () => {
    console.log(`Web server listening on port ${PORT}`);
});