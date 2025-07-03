const esp32IP = "192.168.171.17"; // ← change this to your ESP32's IP

const params = [
  "speedScale", "maxSpeedScale", "rampUpStep", "rampDownStep",
  "rampDownFastStep", "rampInterval", "sinr", "cosr"
];

function $(id) {
  return document.getElementById(id);
}

async function loadConfig() {
  try {
    const res = await fetch(esp32IP + "/config");
    const data = await res.json();
    params.forEach(p => { if ($(p)) $(p).value = data[p]; });
    updateChart(data.m1, data.m2, data.m3);
  } catch (e) {
    const statusElem = $("status");
    if (statusElem) statusElem.innerText = "❌ Failed to load config";
  }
}

async function send() {
  const payload = {};
  params.forEach(p => {
    payload[p] = parseFloat($(p).value);
  });

  try {
    const res = await fetch(esp32IP + "/config", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });
    const json = await res.json();
    const statusElem = $("status");
    if (statusElem) statusElem.innerText = "✅ Sent successfully";
  } catch (e) {
    const statusElem = $("status");
    if (statusElem) statusElem.innerText = "❌ Failed to send config";
  }
}

// === Chart.js setup ===
const ctx = document.getElementById("chart").getContext("2d");
const chartData = {
  labels: [],
  datasets: [
    { label: "m1", data: [], borderColor: "red", fill: false },
    { label: "m2", data: [], borderColor: "blue", fill: false },
    { label: "m3", data: [], borderColor: "green", fill: false }
  ]
};
const chart = new Chart(ctx, {
  type: "line",
  data: chartData,
  options: {
    animation: false,
    scales: {
      y: { suggestedMin: -255, suggestedMax: 255 }
    }
  }
});

function updateChart(m1, m2, m3) {
  const now = new Date().toLocaleTimeString();
  chartData.labels.push(now);
  chartData.datasets[0].data.push(m1);
  chartData.datasets[1].data.push(m2);
  chartData.datasets[2].data.push(m3);

  if (chartData.labels.length > 30) {
    chartData.labels.shift();
    chartData.datasets.forEach(ds => ds.data.shift());
  }

  chart.update();
}

setInterval(loadConfig, 1000); // Refresh every 2 seconds
window.onload = loadConfig;
