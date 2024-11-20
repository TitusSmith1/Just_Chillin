/** Handle root or redirect to captive portal */
void handleRoot() {
  if (captivePortal()) { // If captive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent(
    "   <!DOCTYPE html>"
    "    <html lang='en'>"
    "    <head>"
    "        <meta charset='UTF-8'>"
    "        <meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "       <title>JustChillin'Freezer</title>"
    "        <style>"
    "            body {"
    "                font-family: Arial, sans-serif;"
    "                margin: 0;"
    "                padding: 20px;"
    "                background-color: #0f0f0f;"
    "                color: #333;"
    "            }"
    "            h1 {"
		"    padding: 20px;"
		"    size: 60px;"
		"    margin: 0;"
    "                color: #0194B0;"
    "            }"
		"h2 {"
    "                font-family: Arial, sans-serif;"
    "                margin: 0;"
    "                padding: 30px;"
		"    size: 20px;"
    "                background-color: #3f3f3f;"
    "                color: #0194B0;"
		"    border-radius: 10px;"
    "                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);"
    "            }"
		"h3 {"
    "                font-family: Arial, sans-serif;"
    "                margin: 10;"
    "                padding: 15px;"
		"    size: 20px;"
    "                background-color: white;"
    "                color: #000000;"
		"    border-radius: 10px;"
    "                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);"
    "            }"
		"h4 {"
    "                font-family: Arial, sans-serif;"
    "                margin: 10;"
    "                padding: 15px;"
		"    size: 15px;"
    "                background-color: white;"
    "                color: #333;"
		"    border-radius: 10px;"
    "                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);"
    "            }"
    "green_text {" 
    "                color: #00ff00;"
    "            }"
    "red_text {" 
    "                color: #ff0000;"
    "            }"
    "            .container {"
    "                max-width: 600px;"
    "                margin: auto;"
    "                padding: 20px;"
    "                background: white;"
    "                border-radius: 10px;"
    "                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);"
    "            }"
    "            img {"
    "                max-width: 100%;"
    "                height: auto;"
    "                border-radius: 8px;"
    "            }"
    "            a {"
    "                color: #0194B0;"
    "                text-decoration: none;"
    "            }"
    "            a:hover {"
    "                text-decoration: underline;"
    "            }"
    "            @media (max-width: 600px) {"
    "                body {"
    "                    padding: 10px;"
    "                }"
    "            }"
    "        </style>"
    "    </head>"
    "    <body>"
    "        <div class='container'>"
    "            <h1>Just Chillin' Freezer</h1>"
    "            <img src='https://img.goodfon.com/wallpaper/nbig/4/57/fjords-rocky-mountains-snow-sea-bay-water-night-northern-lig.webp' alt='Background_Image'>"
  );
  server.sendContent("<h2>Current Freezer Temperature: " + String(temp) + String(isCelcius?" °C":" °F")+"</h2>");
  if(is_active){
    server.sendContent("<h2>Freezer Status <green_text> ON </green_text> </h2>");
  }
  else{
    server.sendContent("<h2>Freezer Status <red_text> OFF </red_text> </h2>");
  }
  server.sendContent("<p><h3>You can update the setpoint temperature <a href='/set_temp'>here</a>.</h3></p><p><h3>You can also <a href='/update_units'>toggle units</a>.</h3></p><p><h3>You can view the temperature graph <a href='/get_graph'>here</a>.</h3></p><p><h4> Wifi Connection:</p>");
  if (server.client().localIP() == apIP) {
    server.sendContent(String("<p>You are connected through the soft AP: <strong>") + softAP_ssid + "</strong></p>");
  } else {
    server.sendContent(String("<p>You are connected through the wifi network: <strong>") + ssid + "</strong></p>");
  }
  server.sendContent("<p>Wifi connection configuration <a href='/wifi'>here</a>.</h4></p></body></html></div></body></html>");
  server.client().stop(); // Stop is needed because we sent no content length
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname)+".local")) {
    Serial.print("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

/** Wifi config page handler */
void handleWifi() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent(
    "<html><head></head><body>"
    "<h1>Wifi config</h1>"
  );
  if (server.client().localIP() == apIP) {
    server.sendContent(String("<p>You are connected through the soft AP: ") + softAP_ssid + "</p>");
  } else {
    server.sendContent(String("<p>You are connected through the wifi network: ") + ssid + "</p>");
  }
  server.sendContent(
    "\r\n<br />"
    "<table><tr><th align='left'>SoftAP config</th></tr>"
  );
  server.sendContent(String() + "<tr><td>SSID " + String(softAP_ssid) + "</td></tr>");
  server.sendContent(String() + "<tr><td>IP " + toStringIp(WiFi.softAPIP()) + "</td></tr>");
  server.sendContent(
    "</table>"
    "\r\n<br />"
    "<table><tr><th align='left'>WLAN config</th></tr>"
  );
  server.sendContent(String() + "<tr><td>SSID " + String(ssid) + "</td></tr>");
  server.sendContent(String() + "<tr><td>IP " + toStringIp(WiFi.localIP()) + "</td></tr>");
  server.sendContent(
    "</table>"
    "\r\n<br />"
    "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>"
  );
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      server.sendContent(String() + "\r\n<tr><td>SSID " + WiFi.SSID(i) + String((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":" *") + " (" + WiFi.RSSI(i) + ")</td></tr>");
    }
  } else {
    server.sendContent(String() + "<tr><td>No WLAN found</td></tr>");
  }
  server.sendContent(
    "</table>"
    "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
    "<input type='text' placeholder='network' name='n'/>"
    "<br /><input type='password' placeholder='password' name='p'/>"
    "<br /><input type='submit' value='Connect/Disconnect'/></form>"
    "<p>You may want to <a href='/'>return to the home page</a>.</p>"
    "</body></html>"
  );
  server.client().stop(); // Stop is needed because we sent no content length
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 302, "text/plain", "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void handleSetTemp() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent(R"(
    <!DOCTYPE html>
    <html lang='en'>
    <head>
        <meta charset='UTF-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1.0'>
        <title>Just Chillin' Freezer</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                margin: 0;
                padding: 20px;
                background-color: #0f0f0f;
                color: #333;
            }
            h1 {
                padding: 20px;
                margin: 0;
                color: #0194B0;
            }
            h4 {
                margin: 10;
                padding: 15px;
                size: 15px;
                background-color: white;
                color: #333;
                border-radius: 10px;
                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            }
            .container {
                max-width: 600px;
                margin: auto;
                padding: 20px;
                background: white;
                border-radius: 10px;
                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            }
            input[type="text"], input[type="submit"] {
                padding: 10px;
                margin-top: 10px;
                border-radius: 5px;
                border: 1px solid #ccc;
                width: calc(100% - 22px);
            }
            input[type="submit"] {
                background-color: #0194B0;
                color: white;
                cursor: pointer;
            }
            input[type="submit"]:hover {
                background-color: #017f9b;
            }
        </style>
    </head>
    <body>
        <div class='container'>
            <h1>Just Chillin' Freezer</h1>
            <form method="POST" action="save_temp">
                <h4>Send Data:</h4>
                <input type="text" placeholder="Temperature" name="t" required pattern="^\d+(\.\d+)?$" title="Enter a valid decimal number" />
                <br />
                <input type="submit" value="Send" />
            </form>
        </div>
    </body>
    </html>
    )");
    /*
    "<html><head></head><body>"
    "<h1>Display</h1>"
  );
    server.sendContent(
    "</table>"
    "\r\n<br /><form method='POST' action='save_temp'><h4>Send Data:</h4>"
    "<input type='number' placeholder='temperature' name='t'/>"
    "<br /><input type='submit' value='Send'/></form>"
    "</body></html>"
  );*/
  server.client().stop(); // Stop is needed because we sent no content length
}

void handleSaveTemp() {
  Serial.println("Data Incoming");
  newtemp=server.arg("t").toFloat();
  Serial.println(newtemp);


  // Create a small HTML response with a JavaScript alert
  String response = "<html><head><script>"
                    "alert('Temp updated: " + String(newtemp) + "');"
                    "window.location.href = 'set_temp';"
                    "</script></head><body></body></html>";

  // Send the response
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "text/html", response); // Send a 200 response with the HTML
  server.client().stop(); // Stop is needed because we sent no content length
  updateOLED();
}

void handleUnits() {
  Serial.println("Data Incoming");
  isCelcius=!isCelcius;
  Serial.println(isCelcius);

  // Create a small HTML response with a JavaScript alert
  String response = "<html><head><script>"
                    "alert('Temp updated: " + String(isCelcius ? "Celcius": "Farenheit" )+ "');"
                    "window.location.href = '/';"
                    "</script></head><body></body></html>";

  // Send the response
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "text/html", response); // Send a 200 response with the HTML
  server.client().stop(); // Stop is needed because we sent no content length
  updateOLED();
}
/** Handle Graph page */

void handleGraph() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent(R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Temperature Graph</title>
    <style>body{font-family:Arial,sans-serif;margin:0;padding:20px;background-color:#0f0f0f;color:#333;}h1{color:#0194B0;margin-bottom:20px;}.container{max-width:800px;margin:auto;padding:20px;background:white;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}canvas{width:100%;height:400px;}</style>
    </head>
    <body>
    <div class="container"><h1>Temperature Over Time</h1><canvas id="temperatureChart" width="800" height="400"></canvas></div>
    <script>
    const canvas = document.getElementById('temperatureChart');
    const ctx = canvas.getContext('2d');
    let data = { labels: [], temperatures: [] };

    function drawGraph() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      
      // Draw axes
      ctx.strokeStyle = '#333';
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(50, 350);
      ctx.lineTo(750, 350); // X-axis
      ctx.moveTo(50, 50);
      ctx.lineTo(50, 350); // Y-axis
      ctx.stroke();
      
      // Draw labels
      ctx.fillStyle = '#333';
      ctx.font = '12px Arial';
      data.labels.forEach((label, index) => {
        const x = 50 + (index / (data.labels.length - 1)) * 700;
        ctx.fillText(label, x - 10, 370); // X-axis labels
      });
      for (let i = 0; i <= 10; i++) {
        const y = 350 - (i / 10) * 300;
        const temp = Math.round((Math.min(...data.temperatures) + (i / 10) * (Math.max(...data.temperatures) - Math.min(...data.temperatures))) * 10) / 10;
        ctx.fillText(temp + '°C', 10, y + 3); // Y-axis labels
      }
      
      // Plot data
      ctx.strokeStyle = '#0194B0';
      ctx.lineWidth = 2;
      ctx.beginPath();
      data.temperatures.forEach((temp, index) => {
        const x = 50 + (index / (data.labels.length - 1)) * 700;
        const y = 350 - ((temp - Math.min(...data.temperatures)) / (Math.max(...data.temperatures) - Math.min(...data.temperatures))) * 300;
        if (index === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      });
      ctx.stroke();
    }

    function fetchTemperatureData() {
      fetch('/get_temperature_data')
        .then(response => response.json())
        .then(newData => {
          data = newData;
          drawGraph();
        })
        .catch(error => console.error('Error fetching data:', error));
    }

    setInterval(fetchTemperatureData, 5000);
    fetchTemperatureData();
    </script>
    </body>
    </html>
  )");
  server.client().stop(); // Stop is needed because we sent no content length
}



/** Handle temperature data retrieval */

void handleTemperatureData() {
  // Simulated temperature data for this example
  String jsonResponse = "{ \"labels\": [\"10:00\", \"10:05\", \"10:10\", \"10:15\"], \"temperatures\": [2.5, 2.7, 2.9, 3.0] }";
  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "application/json", jsonResponse);
  server.client().stop();
}

/*
void handleGraph(){
  server.sendContent(R"(
    <!DOCTYPE html>
    <html lang='en'>
    <head>
        <meta charset='UTF-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1.0'>
        <title>Temperature Graph</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                margin: 0;
                padding: 20px;
                background-color: #0f0f0f;
                color: #333;
            }
            h1 {
                padding: 20px;
                margin: 0;
                color: #0194B0;
            }
            .container {
                max-width: 600px;
                margin: auto;
                padding: 20px;
                background: white;
                border-radius: 10px;
                box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            }
            canvas {
                max-width: 100%;
                border-radius: 8px;
            }
        </style>
        <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    </head>
    <body>
        <div class='container'>
            <h1>Temperature Over Time</h1>
            <canvas id="temperatureChart"></canvas>
            <script>
                // Fetch temperature data from the server
                fetch('/get_temperature_data')
                    .then(response => response.json())
                    .then(data => {
                        const labels = ) + labels + (;
                        const temperatureChart = new Chart(
                            document.getElementById('temperatureChart'),
                            {
                                type: 'line',
                                data: {
                                    labels: labels,
                                    datasets: [{
                                        label: 'Temperature (°C)',
                                        data: data,
                                        borderColor: '#0194B0',
                                        backgroundColor: 'rgba(1, 148, 176, 0.2)',
                                        fill: true,
                                    }]
                                },
                                options: {
                                    responsive: true,
                                    scales: {
                                        y: {
                                            beginAtZero: true
                                        }
                                    }
                                }
                            }
                        );
                    });
            </script>
        </div>
    </body>
    </html>
    )");
  server.client().stop(); // Stop is needed because we sent no content length
}
*/
void handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 404, "text/plain", message );
}

