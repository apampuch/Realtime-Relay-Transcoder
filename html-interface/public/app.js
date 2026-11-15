import { ws, random_id, send_to_server } from "./websockets.js";

/* =====================================================
 * DOM references
 * ===================================================== */

const filePath = document.getElementById("file-path");

const codecSelect = document.getElementById("codec");
const presetSelect = document.getElementById("preset");
const bitrateInput = document.getElementById("bitrate");

const playButton = document.getElementById("play-button");
const playIcon = document.getElementById("play-icon");
const backwardButton = document.getElementById("backward-button");
const forwardButton = document.getElementById("forward-button");
const seekBar = document.getElementById("seek-bar");
const timeDisplay = document.getElementById("time-display");

const serverURL = document.getElementById("server-url");
const connectButton = document.getElementById("connect-button");

const logOutput = document.getElementById("log-output");
const clearLogButton = document.getElementById("clear-log-button");


/* =====================================================
 * Logging
 * ===================================================== */

function log(message, type = "normal") {
    const entry = document.createElement("div");
    entry.className = "log-entry";

    if (type === "error" || type === "success") {
        entry.classList.add(type);
    }

    const timestamp = new Date().toLocaleTimeString();

    const time = document.createElement("span");
    time.className = "log-time";
    time.textContent = `[${timestamp}]`;

    entry.appendChild(time);
    entry.appendChild(document.createTextNode(message));

    logOutput.appendChild(entry);

    // Scroll to the latest message
    logOutput.scrollTop = logOutput.scrollHeight;
}

clearLogButton.addEventListener("click", () => {
    logOutput.replaceChildren();
});


/* =====================================================
 * File selection
 * ===================================================== */

// access with
// const filePath = document.getElementById("file-path").value.trim();


/* =====================================================
 * Encoding options
 * ===================================================== */

// Add additional codecs and their presets here.

const codecPresets = {
    x264: [
        "ultrafast",
        "superfast",
        "veryfast",
        "faster",
        "fast",
        "medium",
        "slow",
        "slower",
        "veryslow"
    ]

    // Example:
    // av1: ["0", "1", "2", "3", "4", "5"]
};


// Populate codec dropdown.

for (const codec of Object.keys(codecPresets)) {
    const option = document.createElement("option");

    option.value = codec;
    option.textContent = codec;

    codecSelect.appendChild(option);
}


// Update presets when codec changes.

codecSelect.addEventListener("change", () => {
    const codec = codecSelect.value;
    const presets = codecPresets[codec] || [];

    presetSelect.replaceChildren();

    for (const preset of presets) {
        const option = document.createElement("option");

        option.value = preset;
        option.textContent = preset;

        presetSelect.appendChild(option);
    }

    // Default to "slow" when available, otherwise first preset.
    const defaultPreset = presets.includes("slow")
        ? "slow"
        : presets[0];

    if (defaultPreset) {
        presetSelect.value = defaultPreset;
    }
});

// Initialize codec and preset dropdowns.
codecSelect.dispatchEvent(new Event("change"));


/* =====================================================
 * Playback controls
 *
 * Implement these yourself.
 * ===================================================== */

// TODO: Play / Pause callback

// TODO: Seek backward 5 seconds callback

// TODO: Seek forward 5 seconds callback

// TODO: Seek bar callback

// TODO: Update time display when playback position changes.


/* =====================================================
 * Server connection
 * ===================================================== */

connectButton.addEventListener("click", () => {
    console.log("Clicked");

    const urlString = serverURL.value.trim();

    if (!urlString) {
        log("Please enter a server URL.", "error");
        return;
    }

    // Validate URL syntax and protocol.
    let url;

    try {
        url = new URL(urlString);
    } catch {
        log("Invalid server URL.", "error");
        return;
    }

    if (url.protocol !== "http:" && url.protocol !== "https:") {
        log("Server URL must use http:// or https://.", "error");
        return;
    }

    if (!url.hostname) {
        log("Server URL must contain a hostname.", "error");
        return;
    }

    if (!url.port) {
        // use default port
        url.port = "8889";
    }

    log(`Connecting to ${url.protocol}//${url.host}:${url.port}${url.pathname}${url.search}${url.hash}...`);

    // TODO:
    // Establish connection to mediamtx server.
    
});

ws.addEventListener("open", async () => {
    console.log("Connected to websocket.");
});

/* =====================================================
 * Initial state
 * ===================================================== */

log("RR Transcoder initialized.", "success");
