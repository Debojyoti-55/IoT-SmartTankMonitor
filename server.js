const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const cors = require('cors');

const app = express();
const server = http.createServer(app);
const io = new Server(server, { cors: { origin: "*" } });

// Global object to store the most recent data from the ESP32
let latestData = {
    id: "No Data",
    w_lvl: 0,
    booking: 0,
    lastUpdated: null
};

app.use(cors());
app.use(express.json()); 

// Endpoint for ESP32 to POST data
app.post('/api/data', (req, res) => {
    if (!req.body || Object.keys(req.body).length === 0) {
        return res.status(400).send({ error: "No data provided" });
    }

    console.log("Data received from ESP32:", req.body);
    
    // Update our global storage object
    latestData = {
        ...req.body,
        lastUpdated: new Date().toLocaleString()
    };
    
    // Broadcast to the frontend dashboard
    io.emit('updateDashboard', req.body); 
    
    res.status(200).send({ status: "Success", received: req.body });
});

// New GET Endpoint for pure JSON data
app.get('/get/data/v2', (req, res) => {
    // res.json automatically sets Content-Type to application/json
    res.json(latestData);
});

// Serve the dashboard
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index.html');
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => console.log(`Server running on port ${PORT}`));