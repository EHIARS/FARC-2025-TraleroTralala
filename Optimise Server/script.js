const params = [
  "speedScale", "maxSpeedScale", "rampUpStep", "rampDownStep",
  "rampDownFastStep", "rampInterval", "sinr", "cosr"
];

let currentIP = null;
let isConnected = false;
let retryCount = 0;
const maxRetries = 5;
let configUpdateInterval;

// DOM utility function
function $(id) { return document.getElementById(id); }

// Status display function
function updateStatus(message, isError = false) {
  const statusElem = $("status");
  if (statusElem) {
    statusElem.innerText = message;
    statusElem.style.color = isError ? "red" : "green";
  }
}

// Connection functions
async function testConnection(host) {
  try {
    const response = await fetch(`http://${host}/config`, {
      method: 'GET',
      mode: 'no-cors',  // Important for local network requests
      cache: 'no-store',
      headers: {
        'Content-Type': 'application/json'
      }
    });
    
    // Even if the response isn't OK, we got a response
    return true;
  } catch (e) {
    console.error(`Connection test to ${host} failed:`, e);
    return false;
  }
}

async function loadConfig() {
  if (!currentIP) return;
  
  try {
    const response = await fetch(`http://${currentIP}/config`, {
      method: 'GET',
      mode: 'no-cors',
      cache: 'no-store'
    });
    
    // Handle non-JSON responses
    const text = await response.text();
    let data;
    try {
      data = JSON.parse(text);
    } catch (e) {
      throw new Error("Invalid JSON response");
    }
    
    params.forEach(p => {
      const input = document.getElementById(p) || document.getElementById(`${p}-value`);
      if (input) input.value = data[p];
    });
    
    retryCount = 0;
  } catch (e) {
    console.error("Config load failed:", e);
    retryCount++;
    
    if (retryCount >= maxRetries) {
      currentIP = null;
      isConnected = false;
      updateStatus("❌ Connection lost - retrying...", true);
      setTimeout(initConnection, 2000);
    }
  }
}

async function discoverESP32() {
  const hosts = [
    "192.168.171.17",    // Static IP
    "192.168.4.1",       // AP fallback
    "esp32.local"        // mDNS
  ];

  for (const host of hosts) {
    const isConnected = await testConnection(host);
    if (isConnected) {
      console.log(`Connected to ESP32 at ${host}`);
      return host;
    }
  }
  
  throw new Error("Could not connect to any known ESP32 address");
}

// Configuration functions


async function send() {
  if (!currentIP) {
    updateStatus("❌ Not connected to ESP32", true);
    return;
  }

  const payload = {};
  params.forEach(p => {
    const input = $(p) || $(`${p}-value`);
    if (input) payload[p] = parseFloat(input.value);
  });

  try {
    const response = await fetch(`http://${currentIP}/config`, {
      method: 'POST',
      mode: 'cors',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(payload)
    });
    
    if (!response.ok) throw new Error("Bad response");
    
    updateStatus("✅ Sent successfully");
  } catch (e) {
    console.error("Send failed:", e);
    updateStatus("❌ Failed to send config", true);
    currentIP = null;
    isConnected = false;
  }
}

// UI interaction functions
function highlightChange(element) {
  element.style.backgroundColor = '#e6f7ff';
  setTimeout(() => {
    element.style.backgroundColor = '';
  }, 500);
}

function setupInputSync() {
  document.querySelectorAll('.param-row').forEach(row => {
    const slider = row.querySelector('input[type="range"]');
    const number = row.querySelector('input[type="number"]');
    
    if (slider && number) {
      // Sync slider to number input
      slider.addEventListener('input', () => {
        number.value = slider.value;
        highlightChange(number);
      });
      
      // Sync number input to slider with validation
      number.addEventListener('input', () => {
        const min = parseFloat(slider.min);
        const max = parseFloat(slider.max);
        let value = parseFloat(number.value);
        
        // Validate range
        if (isNaN(value)) value = min;
        if (value < min) value = min;
        if (value > max) value = max;
        
        number.value = value;
        slider.value = value;
        highlightChange(number);
      });
    }
  });

  // Initialize all values
  document.querySelectorAll('.param-value').forEach(input => {
    input.addEventListener('change', function() {
      highlightChange(this);
    });
  });
}

// Initialization
async function initConnection() {
  try {
    updateStatus("⌛ Searching for ESP32...");
    currentIP = await discoverESP32();
    updateStatus(`✅ Connected to ${currentIP}`);
    isConnected = true;
    retryCount = 0;
    await loadConfig();
    
    // Start periodic config updates
    configUpdateInterval = setInterval(loadConfig, 1000);
  } catch (e) {
    console.error("Connection failed:", e);
    updateStatus("❌ Connection failed - retrying...", true);
    setTimeout(initConnection, 1000);
  }
}

// Main initialization
window.onload = () => {
  setupInputSync();
  document.getElementById('sendButton')?.addEventListener('click', send);
  initConnection();
};