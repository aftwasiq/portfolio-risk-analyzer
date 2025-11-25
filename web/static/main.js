let lastSummary = null;

const resultsDiv = document.getElementById('results');
const connectBtn = document.getElementById('connectBtn');
const uploadBtn = document.getElementById('uploadBtn');
const form = document.getElementById('uploadForm');
const fileInput = document.getElementById('fileInput');

const navBtns = document.querySelectorAll('.nav-btn');
const pages = {
    information: document.getElementById('informationPage'),
    risk: document.getElementById('riskPage'),
    documentation: document.getElementById('documentationPage')
};

Object.values(pages).forEach(p => p.classList.remove('active'));
pages.risk.classList.add('active');

navBtns.forEach(btn => btn.classList.remove('active'));
navBtns.forEach(btn => {
    if (btn.dataset.page === 'risk') btn.classList.add('active');

    btn.addEventListener('click', () => {
        navBtns.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');

        Object.values(pages).forEach(p => p.classList.remove('active'));
        pages[btn.dataset.page].classList.add('active');
    });
});

connectBtn.addEventListener('click', () => {
    alert("Portfolio connection coming soon!");
});

uploadBtn.addEventListener('click', () => {
    form.classList.toggle('show');
});

form.addEventListener('submit', async (e) => {
    e.preventDefault();
    const file = fileInput.files[0];
    if (!file) return;

    const formData = new FormData();
    formData.append('file', file);

    resultsDiv.classList.add('hidden');
    resultsDiv.innerHTML = "<p>Processing...</p>";
    resultsDiv.classList.remove('fade-in');

    try {
        const res = await fetch('/upload_csv', { method: 'POST', body: formData });
        if (!res.ok) {
            resultsDiv.innerHTML = `<p>Error: ${res.statusText}</p>`;
            return;
        }
        const data = await res.json();
        lastSummary = buildSummary(data);

        const front = document.getElementById('frontOptions');
        front.classList.add('fade-out');

        setTimeout(() => {
            front.style.display = 'none';
            renderResults(data);
            resultsDiv.classList.remove('hidden');
            resultsDiv.classList.add('fade-in');
        }, 1000);

    } catch (err) {
        console.error(err);
        resultsDiv.innerHTML = `<p>Unexpected error occurred</p>`;
    }
});

function buildSummary(data) {
    return {
        total_return: data.portfolio.TotalReturn / 100.0,
        volatility: data.portfolio.PortfolioVolatility / 100.0,
        stocks: data.stocks.map(s => ({
            ticker: s.Ticker,
            weight: s.Weight,
            mean_return: s.MeanReturn,
            volatility: s.Volatility
        }))
    };
}

async function requestAI() {
    if (!lastSummary) return;

    const aiDiv = document.getElementById('aiResults');
    aiDiv.innerHTML = "<p>Generating AI analysis...</p>";
    aiDiv.classList.remove('fade-in');
    aiDiv.classList.remove('hidden');

    try {
        const res = await fetch('/interpret', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(lastSummary)
        });

        if (!res.ok) {
            aiDiv.innerHTML = `<p>Error: ${res.statusText}</p>`;
            return;
        }

        const data = await res.json();
        aiDiv.innerHTML = `<h3>AI Interpretation</h3><p>${data.analysis}</p>`;
        aiDiv.classList.add('fade-in');

    } catch (err) {
        console.error(err);
        aiDiv.innerHTML = `<p>Unexpected error occurred while generating AI analysis</p>`;
    }
}

function renderResults(data) {
    let html = '';

    html += '<h3>Historical Stock Prices</h3>';
    html += '<div id="chartsContainer"></div>';

    html += '<h3>Portfolio Summary</h3>';
    html += '<table>';
    html += '<tr><th>Total Return</th><th>Portfolio Volatility</th></tr>';
    html += `<tr>
        <td class="${data.portfolio.TotalReturn >= 0 ? 'positive' : 'negative'}" style="font-weight:bold;">
            ${data.portfolio.TotalReturn.toFixed(4)}%
        </td>
        <td class="${data.portfolio.PortfolioVolatility <= 0 ? 'positive' : 'negative'}" style="font-weight:bold;">
            ${data.portfolio.PortfolioVolatility.toFixed(4)}%
        </td>
    </tr>`;
    html += '</table>';

    html += '<table>';
    html += '<tr>'
         + '<th data-tooltip="The stock ticker symbol, used to identify the stock.">Ticker</th>'
         + '<th data-tooltip="Number of shares held in the portfolio.">Shares</th>'
         + '<th data-tooltip="Proportion of this stock in the portfolio by value.">Weight</th>'
         + '<th data-tooltip="Average return of the stock over a period.">Mean Return</th>'
         + '<th data-tooltip="Volatility: Standard deviation of stock returns, measures risk.">Volatility</th>'
         + '<th data-tooltip="Value at Risk, estimated potential loss over a period at a given confidence level.">VaR</th>'
         + '</tr>';

    data.stocks.forEach(s => {
        html += `<tr>
            <td>${s.Ticker}</td>
            <td>${s.Shares}</td>
            <td>${(s.Weight*100).toFixed(2)}%</td>
            <td class="${s.MeanReturn >= 0 ? 'positive' : 'negative'}" style="font-weight:bold;">
                ${s.MeanReturn.toFixed(4)}%
            </td>
            <td class="${s.Volatility >= 0 ? 'positive' : 'negative'}" style="font-weight:bold;">
                ${s.Volatility.toFixed(4)}
            </td>
            <td class="${s.VaR >= 0 ? 'negative' : 'positive'}" style="font-weight:bold;">
                ${s.VaR.toFixed(4)}%
            </td>
        </tr>`;
    });
    html += '</table>';

    html += `<button id="aiBtn" class="big-btn smooth-btn">Interpret My Results!</button>`;
    html += `<div id="aiResults" class="hidden"></div>`;

    resultsDiv.innerHTML = html;

    loadHistoricalCharts();

    document.getElementById('aiBtn').addEventListener('click', () => {
        requestAI();
    });
}

async function loadHistoricalCharts() {
    const chartsDiv = document.getElementById('chartsContainer');
    chartsDiv.innerHTML = "<p>Loading historical price chart...</p>";

    try {
        const res = await fetch('/historical_data');
        const csvText = await res.text();

        const rows = csvText.trim().split('\n');
        rows.shift(); // remove header

        const dataByTicker = {};

        rows.forEach(line => {
            const [ticker, dateStr, closeStr] = line.split(',');
            if (!ticker || !dateStr || !closeStr) return;

            const close = parseFloat(closeStr);
            if (isNaN(close)) return;

            const dateLabel = dateStr.split(" ")[0];
            if (!dataByTicker[ticker]) dataByTicker[ticker] = [];
            dataByTicker[ticker].push({ date: dateLabel, close });
        });

        chartsDiv.innerHTML = "";
        const canvas = document.createElement("canvas");
        canvas.id = "combinedChart";
        canvas.style.height = "600px";
        canvas.style.width = "100%";
        chartsDiv.appendChild(canvas);

        const labels = [];
        const datasets = [];
        const colors = [
            '#ff6384','#36a2eb','#ffcd56','#4bc0c0','#9966ff','#ff9f40','#c9cbcf'
        ];

        Object.keys(dataByTicker).forEach((ticker, index) => {
            const sorted = dataByTicker[ticker].sort((a, b) => new Date(a.date) - new Date(b.date));
            if (labels.length === 0) sorted.forEach(d => labels.push(d.date));

            const color = colors[index % colors.length];
            datasets.push({
                label: ticker,
                data: sorted.map(d => d.close),
                borderColor: color,
                backgroundColor: color,
                borderWidth: 1.25,
                tension: 0.35,
                pointRadius: 2,
                pointHoverRadius: 4,
                pointBackgroundColor: color,  // points match line color
                pointBorderWidth: 0,
                showLine: true
            });
        });

        new Chart(canvas.getContext("2d"), {
            type: 'line',
            data: { labels, datasets },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                layout: { padding: 20 },
                scales: {
                    y: { title: { display: true, text: "Close ($)" } },
                    x: { title: { display: true, text: "Date" }, ticks: { maxRotation: 45, minRotation: 0 } }
                },
                plugins: { legend: { display: true, position: 'top' } }
            }
        });

    } catch (err) {
        console.error(err);
        chartsDiv.innerHTML = "<p>Error loading chart data.</p>";
    }
}

