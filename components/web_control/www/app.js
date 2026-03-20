// HTTP polling connection
let sendInterval = null;
let telemetryInterval = null;
let pingInterval = null;
let lastPingTime = 0;
let isConnected = false;

// Joystick state
let steeringValue = 0;
let throttleValue = 0;

// Speed Control
let maxSpeedPercent = 50;

// DOM elements
const statusElement = document.getElementById('status');
const statusText = statusElement.querySelector('.status-text');
const latencyElement = document.getElementById('latency');
const batteryElement = document.getElementById('battery');
const speedElement = document.getElementById('speed');
const stopButton = document.getElementById('stop-button');

// Mode logic elements
const modeButton = document.getElementById('mode-button');
const modeText = document.getElementById('mode-text');
let autonomousMode = false;

// Joystick elements
const steeringJoystick = document.getElementById('steering-joystick');
const steeringKnob = document.getElementById('steering-knob');
const throttleJoystick = document.getElementById('throttle-joystick');
const throttleKnob = document.getElementById('throttle-knob');

// Speed slider elements
const maxSpeedInput = document.getElementById('max-speed');
const maxSpeedDisplay = document.getElementById('max-speed-value');

// Initialize connection
function startConnection() {
    isConnected = true;
    updateStatus(true);
    startSendLoop();
    startTelemetryLoop();
    startPing();
}

// Update connection status
function updateStatus(connected) {
    isConnected = connected;
    if (connected) {
        statusElement.classList.add('connected');
        statusText.textContent = 'Connected';
    } else {
        statusElement.classList.remove('connected');
        statusText.textContent = 'Disconnected';
        latencyElement.textContent = '--ms';
    }
}

// Start ping/heartbeat
function startPing() {
    pingInterval = setInterval(async () => {
        try {
            lastPingTime = Date.now();
            const response = await fetch('/telemetry');
            if (response.ok) {
                const latency = Date.now() - lastPingTime;
                latencyElement.textContent = `${latency}ms`;
                if (!isConnected) {
                    startConnection();
                }
            }
        } catch (error) {
            console.error('Ping failed:', error);
            updateStatus(false);
        }
    }, 1000);
}

// Send motor commands at 50Hz (20ms)
function startSendLoop() {
    sendInterval = setInterval(() => {
        sendMotorCommand();
    }, 20); // 50Hz = 20ms
}

function stopSendLoop() {
    if (sendInterval) {
        clearInterval(sendInterval);
        sendInterval = null;
    }
}

// Fetch telemetry data
function startTelemetryLoop() {
    telemetryInterval = setInterval(async () => {
        try {
            const response = await fetch('/telemetry');
            if (response.ok) {
                const data = await response.json();
                batteryElement.textContent = `${data.battery}%`;
                speedElement.textContent = `${data.speed.toFixed(1)} km/h`;
            }
        } catch (error) {
            console.error('Telemetry fetch failed:', error);
        }
    }, 100); // Update every 100ms
}

// Send motor command via HTTP POST
async function sendMotorCommand() {
    if (!isConnected || autonomousMode) return;

    try {
        const msg = new Uint8Array(4);

        // Apply speed limit to throttle
        const limitedThrottle = Math.round((throttleValue * maxSpeedPercent) / 100);

        msg[0] = 0x01; // Motor control message
        msg[1] = limitedThrottle; // Throttle (-100 to +100) with limit
        msg[2] = steeringValue; // Steering (-100 to +100)
        msg[3] = (msg[1] + msg[2]) & 0xFF; // Simple checksum

        await fetch('/command', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/octet-stream'
            },
            body: msg
        });
    } catch (error) {
        console.error('Command send failed:', error);
        updateStatus(false);
    }
}

// Emergency stop
async function emergencyStop() {
    steeringValue = 0;
    throttleValue = 0;
    updateJoystickPosition(steeringKnob, 0, 0);
    updateJoystickPosition(throttleKnob, 0, 0);

    try {
        const msg = new Uint8Array([0x03]); // Emergency stop
        await fetch('/command', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/octet-stream'
            },
            body: msg
        });
    } catch (error) {
        console.error('Emergency stop failed:', error);
    }
}

// Joystick handling with Multi-touch support
function setupJoystick(joystickElement, knobElement, isVertical, callback) {
    let currentTouchId = null;
    let startX = 0;
    let startY = 0;

    const maxDistance = 40; // Maximum distance from center

    function handleStart(e) {
        if (currentTouchId !== null) return; // Already dragging

        const touch = e.changedTouches[0];
        currentTouchId = touch.identifier;

        const rect = joystickElement.getBoundingClientRect();
        startX = rect.left + rect.width / 2;
        startY = rect.top + rect.height / 2;

        handleMove(e);
    }

    function handleMove(e) {
        e.preventDefault();

        // Find our touch
        let touch = null;
        for (let i = 0; i < e.changedTouches.length; i++) {
            if (e.changedTouches[i].identifier === currentTouchId) {
                touch = e.changedTouches[i];
                break;
            }
        }

        if (!touch) return;

        let deltaX = touch.clientX - startX;
        let deltaY = touch.clientY - startY;

        // Limit to max distance
        const distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);
        if (distance > maxDistance) {
            const angle = Math.atan2(deltaY, deltaX);
            deltaX = Math.cos(angle) * maxDistance;
            deltaY = Math.sin(angle) * maxDistance;
        }

        updateJoystickPosition(knobElement, deltaX, deltaY);

        // Calculate value (-100 to +100)
        let value;
        if (isVertical) {
            value = Math.round((-deltaY / maxDistance) * 100);
        } else {
            value = Math.round((deltaX / maxDistance) * 100);
        }

        // Clamp value
        value = Math.max(-100, Math.min(100, value));
        callback(value);
    }

    function handleEnd(e) {
        // Find our touch
        let touchFound = false;
        for (let i = 0; i < e.changedTouches.length; i++) {
            if (e.changedTouches[i].identifier === currentTouchId) {
                touchFound = true;
                break;
            }
        }

        if (!touchFound) return;

        currentTouchId = null;

        // Return to center
        updateJoystickPosition(knobElement, 0, 0);
        callback(0);
    }

    // Touch events
    joystickElement.addEventListener('touchstart', handleStart, { passive: false });
    joystickElement.addEventListener('touchmove', handleMove, { passive: false });
    joystickElement.addEventListener('touchend', handleEnd);
    joystickElement.addEventListener('touchcancel', handleEnd);

    // Mouse events (for desktop testing - single pointer only)
    let isDraggingMouse = false;
    joystickElement.addEventListener('mousedown', (e) => {
        isDraggingMouse = true;
        const rect = joystickElement.getBoundingClientRect();
        startX = rect.left + rect.width / 2;
        startY = rect.top + rect.height / 2;

        // Update initially
        let deltaX = e.clientX - startX;
        let deltaY = e.clientY - startY;
        // ... (reuse logic or keep simple) - keeping simple for readability
    });

    document.addEventListener('mousemove', (e) => {
        if (!isDraggingMouse) return;
        let deltaX = e.clientX - startX;
        let deltaY = e.clientY - startY;

        const distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);
        if (distance > maxDistance) {
            const angle = Math.atan2(deltaY, deltaX);
            deltaX = Math.cos(angle) * maxDistance;
            deltaY = Math.sin(angle) * maxDistance;
        }
        updateJoystickPosition(knobElement, deltaX, deltaY);

        let value;
        if (isVertical) {
            value = Math.round((-deltaY / maxDistance) * 100);
        } else {
            value = Math.round((deltaX / maxDistance) * 100);
        }
        value = Math.max(-100, Math.min(100, value));
        callback(value);
    });

    document.addEventListener('mouseup', () => {
        if (!isDraggingMouse) return;
        isDraggingMouse = false;
        updateJoystickPosition(knobElement, 0, 0);
        callback(0);
    });
}

function updateJoystickPosition(knob, x, y) {
    knob.style.transform = `translate(calc(-50% + ${x}px), calc(-50% + ${y}px))`;
}

// Initialize joysticks
setupJoystick(steeringJoystick, steeringKnob, false, (value) => {
    steeringValue = value;
});

setupJoystick(throttleJoystick, throttleKnob, true, (value) => {
    throttleValue = value;
});

// Speed Slider Logic
maxSpeedInput.addEventListener('input', (e) => {
    maxSpeedPercent = parseInt(e.target.value);
    maxSpeedDisplay.textContent = `${maxSpeedPercent}%`;
});

maxSpeedInput.addEventListener('change', async (e) => {
    maxSpeedPercent = parseInt(e.target.value);
    
    if (!isConnected) return;
    try {
        const msg = new Uint8Array([0x05, maxSpeedPercent]);
        await fetch('/command', {
            method: 'POST',
            headers: { 'Content-Type': 'application/octet-stream' },
            body: msg
        });
    } catch (error) {
        console.error('Speed update failed:', error);
    }
});

// Stop button
stopButton.addEventListener('click', emergencyStop);
stopButton.addEventListener('touchstart', (e) => {
    e.preventDefault();
    emergencyStop();
});

// Prevent scrolling and zooming
document.addEventListener('touchmove', (e) => {
    if (e.target.closest('.joystick') || e.target.closest('.stop-button') || e.target.closest('.speed-slider')) {
        e.preventDefault();
    }
}, { passive: false });

// Mode Switch Logic
modeButton.addEventListener('click', async () => {
    autonomousMode = !autonomousMode;
    if (autonomousMode) {
        modeButton.classList.remove('manual');
        modeButton.classList.add('auto');
        modeText.textContent = 'Auto Mode';
        steeringJoystick.style.opacity = '0.4';
        throttleJoystick.style.opacity = '0.4';
        steeringJoystick.style.pointerEvents = 'none';
        throttleJoystick.style.pointerEvents = 'none';
    } else {
        modeButton.classList.remove('auto');
        modeButton.classList.add('manual');
        modeText.textContent = 'Manual Mode';
        steeringJoystick.style.opacity = '1';
        throttleJoystick.style.opacity = '1';
        steeringJoystick.style.pointerEvents = 'auto';
        throttleJoystick.style.pointerEvents = 'auto';
    }

    if (!isConnected) return;
    try {
        const msg = new Uint8Array([0x04, autonomousMode ? 1 : 0]);
        await fetch('/command', {
            method: 'POST',
            headers: { 'Content-Type': 'application/octet-stream' },
            body: msg
        });
    } catch (error) {
        console.error('Mode switch failed:', error);
    }
});

// Initialize connection on load
window.addEventListener('load', () => {
    startConnection();

    // Set initial display value
    maxSpeedDisplay.textContent = `${maxSpeedInput.value}%`;
    maxSpeedPercent = parseInt(maxSpeedInput.value);
});

// Cleanup on unload
window.addEventListener('beforeunload', () => {
    stopSendLoop();
    if (telemetryInterval) clearInterval(telemetryInterval);
    if (pingInterval) clearInterval(pingInterval);
});
