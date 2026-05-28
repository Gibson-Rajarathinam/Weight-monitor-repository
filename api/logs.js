let dataStore = [];

export default function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'POST') {
    // Receive data from ESP32
    const { name, value } = req.body;
    const entry = {
      name,
      value,
      timestamp: new Date().toISOString()
    };

    dataStore.push(entry);

    // Keep only last 100 entries
    if (dataStore.length > 100) {
      dataStore.shift();
    }

    res.status(200).json({ success: true, entry });
  } else if (req.method === 'GET') {
    // Send stored data to frontend
    res.status(200).json(dataStore);
  }
}
