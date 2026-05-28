# ESP8266 Data Logger

A simple web application to fetch data from an ESP8266 microcontroller and display it on a webpage.

## Features

- ✅ Fetch data from ESP8266 via REST API
- ✅ Display data in real-time cards
- ✅ Automatic data logging with localStorage
- ✅ View logged data history
- ✅ Clean, responsive UI
- ✅ Works offline (logs persist locally)

## Files

- `index.html` - Main HTML structure
- `style.css` - Styling and layout
- `script.js` - Fetching and data handling logic
- `vercel.json` - Vercel deployment config

## Local Testing

1. Open `index.html` in a browser
2. Enter your ESP8266 API URL (e.g., `http://192.168.1.100/api/data`)
3. Click "Start Fetching" to begin

## Vercel Deployment

### Step 1: Prepare Your Repository

```bash
# Initialize git (if not already done)
git init

# Add files
git add .
git commit -m "Initial commit: ESP8266 data logger"
```

### Step 2: Push to GitHub

1. Create a new repository on GitHub
2. Push your code:
```bash
git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO.git
git branch -M main
git push -u origin main
```

### Step 3: Deploy to Vercel

**Option A: Using Vercel CLI**
```bash
npm install -g vercel
vercel
# Follow the prompts and select your project
```

**Option B: Using Vercel Dashboard**
1. Go to https://vercel.com
2. Sign in with GitHub
3. Click "New Project"
4. Import your GitHub repository
5. Click "Deploy"

### Step 4: Configure CORS (if needed)

If you get CORS errors, your ESP8266 needs to send proper headers:

```cpp
// Add to your ESP8266 code
void setCorsHeaders(AsyncWebServerRequest *request) {
    request->addHeader("Access-Control-Allow-Origin", "*");
    request->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
    request->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

// Use in your API endpoint
server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request){
    setCorsHeaders(request);
    String json = "{\"name\": \"Sensor1\", \"value\": 25.5}";
    request->send(200, "application/json", json);
});
```

## ESP8266 API Format

Your ESP8266 should return JSON in one of these formats:

**Single object:**
```json
{
  "name": "Temperature",
  "value": 25.5
}
```

**Or array of objects:**
```json
[
  {"name": "Temp", "value": 25.5},
  {"name": "Humidity", "value": 60}
]
```

**Or simple object:**
```json
{
  "temperature": 25.5,
  "humidity": 60
}
```

## Data Logging

- Data is automatically logged to browser's localStorage
- Last 1000 entries are kept
- Clear logs with the "Clear Logs" button
- Logs persist even after closing the browser

## Troubleshooting

**"Error: Failed to fetch"**
- Check your API URL is correct
- Ensure ESP8266 is online and API is accessible
- Check CORS headers if API is on different domain

**No data appears**
- Check browser console (F12) for errors
- Verify API URL format
- Test API directly in browser address bar

**CORS errors**
- Add CORS headers to your ESP8266 API response

## License

MIT
