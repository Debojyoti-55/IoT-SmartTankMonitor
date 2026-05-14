const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const cors = require('cors');

const app = express();
const server = http.createServer(app);
const io = new Server(server, { cors: { origin: "*" } });

app.use(cors());
app.use(express.json()); // Allows the server to understand JSON

// // Endpoint for ESP32 to POST data
// app.post('/api/data', (req, res) => {
//     // Expecting JSON: { "id": "ESP32_01", "w_lvl": 75 }
//     console.log("Data received:", req.body);
    
//     // Broadcast the entire body to the frontend
//     io.emit('updateDashboard', req.body); 
    
//     res.status(200).send({ status: "Data Received" });
// });

app.post('/api/data', (req, res) => {
    // Check if body exists to prevent crash
    if (!req.body) {
        return res.status(400).send({ error: "No data provided" });
    }

    console.log("Data received:", req.body);
    
    // Use optional chaining (req.body?.id) or a simple check
    const deviceId = req.body.id || "Unknown Device";
    
    io.emit('updateDashboard', req.body); 
    res.status(200).send({ status: "Success" });
});

// Serve a simple dashboard
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index.html');
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => console.log(`Server running on port ${PORT}`));