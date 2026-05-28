let isFetching = false;
let fetchInterval;

const startBtn = document.getElementById('startBtn');
const stopBtn = document.getElementById('stopBtn');
const clearBtn = document.getElementById('clearBtn');
const apiUrlInput = document.getElementById('apiUrl');
const dataContainer = document.getElementById('dataContainer');
const logsList = document.getElementById('logsList');

// Load logs from localStorage on page load
loadLogsFromStorage();

startBtn.addEventListener('click', startFetching);
stopBtn.addEventListener('click', stopFetching);
clearBtn.addEventListener('click', clearLogs);

function startFetching() {
    if (isFetching) return;

    const apiUrl = apiUrlInput.value.trim();
    if (!apiUrl) {
        alert('Please enter a valid API URL');
        return;
    }

    isFetching = true;
    startBtn.disabled = true;
    stopBtn.disabled = false;

    // Fetch immediately first
    fetchData(apiUrl);

    // Then fetch every 5 seconds
    fetchInterval = setInterval(() => {
        fetchData(apiUrl);
    }, 5000);

    updateStatus('running', 'Fetching data...');
}

function stopFetching() {
    isFetching = false;
    clearInterval(fetchInterval);
    startBtn.disabled = false;
    stopBtn.disabled = true;
    updateStatus('stopped', 'Stopped');
}

function fetchData(apiUrl) {
    fetch(apiUrl)
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            displayData(data);
            logData(data);
            updateStatus('running', 'Connected ✓');
        })
        .catch(error => {
            console.error('Error fetching data:', error);
            updateStatus('error', `Error: ${error.message}`);
        });
}

function displayData(data) {
    dataContainer.innerHTML = '';

    // Handle different data formats
    if (Array.isArray(data)) {
        data.forEach((item, index) => {
            addDataItem(item, index);
        });
    } else if (typeof data === 'object' && data !== null) {
        // If data is a single object, display it
        Object.entries(data).forEach(([key, value], index) => {
            addDataItem({ name: key, value: value }, index);
        });
    }
}

function addDataItem(item, index) {
    const dataItem = document.createElement('div');
    dataItem.className = 'data-item';

    // Support different property names
    const name = item.name || item.label || item.device || Object.keys(item)[0] || `Item ${index + 1}`;
    const value = item.value || item.weight || item.data || item.reading || Object.values(item)[0] || 'N/A';
    const timestamp = new Date().toLocaleTimeString();

    dataItem.innerHTML = `
        <div class="label">${name}</div>
        <div class="value">${formatValue(value)}</div>
        <div class="timestamp">${timestamp}</div>
    `;

    dataContainer.appendChild(dataItem);
}

function formatValue(value) {
    if (typeof value === 'number') {
        return value.toFixed(2);
    }
    return String(value);
}

function logData(data) {
    const logs = JSON.parse(localStorage.getItem('sensorLogs')) || [];
    const timestamp = new Date().toLocaleString();

    const logEntry = {
        timestamp: timestamp,
        data: data
    };

    logs.push(logEntry);

    // Keep only last 1000 entries
    if (logs.length > 1000) {
        logs.shift();
    }

    localStorage.setItem('sensorLogs', JSON.stringify(logs));
    displayLogs();
}

function loadLogsFromStorage() {
    displayLogs();
}

function displayLogs() {
    const logs = JSON.parse(localStorage.getItem('sensorLogs')) || [];

    if (logs.length === 0) {
        logsList.innerHTML = '<p>No data logged yet</p>';
        return;
    }

    logsList.innerHTML = logs
        .reverse()
        .slice(0, 50) // Show last 50 entries
        .map(log => {
            const dataStr = typeof log.data === 'string' ? log.data : JSON.stringify(log.data);
            return `
                <div class="log-entry">
                    <div class="time">${log.timestamp}</div>
                    <div><strong>Data:</strong> ${dataStr}</div>
                </div>
            `;
        })
        .join('');
}

function clearLogs() {
    if (confirm('Are you sure you want to clear all logged data?')) {
        localStorage.removeItem('sensorLogs');
        displayLogs();
        alert('Logs cleared!');
    }
}

function updateStatus(type, message) {
    let statusEl = document.getElementById('status');
    if (!statusEl) {
        statusEl = document.createElement('div');
        statusEl.id = 'status';
        statusEl.className = 'status';
        document.body.appendChild(statusEl);
    }

    statusEl.textContent = message;
    statusEl.className = `status ${type}`;
}

// Initialize
stopBtn.disabled = true;
