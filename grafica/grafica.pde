
float[] data = {40, 90, 30, 70, 20}; // Initial data points

void setup() {
  size(400, 200);
  background(255);
  drawPlot(data);
}

void draw() {
  // Simulate receiving a new data point every frame
  float newDataPoint = random(0, height);
  addDataPoint(newDataPoint);

  // Redraw the plot with the updated data
  background(255);
  drawPlot(data);
}

void addDataPoint(float newDataPoint) {
  // Add the new data point to the array
  float[] newData = new float[data.length + 1];
  System.arraycopy(data, 0, newData, 0, data.length);
  newData[data.length] = newDataPoint;
  data = newData;  

  // Limit the data array size to prevent it from growing indefinitely
  if (data.length > width / 10) {
    data = subset(data, 1);
  }
}

void drawPlot(float[] data) {
  float xSpacing = width / (data.length - 1);

  // Plot the points
  for (int i = 0; i < data.length; i++) {
    float x = i * xSpacing;
    float y = height - data[i];
    ellipse(x, y, 8, 8);
  }

  // Connect the points with lines
  for (int i = 0; i < data.length - 1; i++) {
    float x1 = i * xSpacing;
    float y1 = height - data[i];
    float x2 = (i + 1) * xSpacing;
    float y2 = height - data[i + 1];
    line(x1, y1, x2, y2);
  }
}
