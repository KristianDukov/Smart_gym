// Styles
const globalStyles = {
  progressColor: '#71ec21', // Progress bar color
  progressEmptyColor: '#121212', // Progress bar (empty part) color
  innerColor: '#282828', // Circle's inner color (same as the page background)
  valueFont: '65px sans-serif', // Value's font size and style
  valueColor: '#fefefe', // Value's color
  labelFont: '50px sans-serif', // Title's font size and style
  labelColor: '#898989', // Title's color
}

// Initialize graphs to 0
drawGraph('repetitions', 10, 0, 'Repetitions')
drawGraph('power', 2000, 0, 'Power')
drawGraph('contraction', 5, 0, 'Contraction')
drawGraph('set', 3, 0, 'Set')
drawGraph('extension', 4000, 0, 'Extension')
drawGraph('rest', 9999999, 0, 'Rest')

// Initialize weight tooltip
const weightElement = document.getElementById('weightValue')
weightElement.innerHTML = 0
// Initialize rest info labels
const account = document.getElementById('account')
// const machine = document.getElementById('machine')

let lastSend = 0

// Connect to the socket. Same URL, port 81.
const Socket = new WebSocket(`ws://${window.location.hostname}:81`)
// const Socket = new WebSocket('ws://89.156.183.125:81/192.168.0.14')

// When a new websockets message is received, redraw the dial with the updated value
Socket.onmessage = (evt) => {
  const data = JSON.parse(evt.data)
  // Calls function to visualize the graphs
  updateGraphs(data)
}

// --- Button events ---/
const calibrationButton = document.getElementById('calibration')
const saveButton = document.getElementById('save')
const machineList = document.getElementById('machineSelect')
// Save button title to be able to tongue it
const calibrationButtonTitle = calibrationButton.innerText
// Button / Select events sending corresponding requests to the server
calibrationButton.addEventListener('click', () => calibrate())
saveButton.addEventListener('click', () => save())
machineList.addEventListener('change', () => machineSelect())

// Calibration method sends a calibration request and waits for a response
// when calibration is completed, to restore the UI to the starting point
function calibrate() {
  Socket.send(0)
  // Change button title to 'Calibrating..' while the action takes place
  calibrationButton.innerText = 'Calibrating...'
  calibrationButton.style.boxShadow = '1px 1px 60px #282828'
  Socket.onmessage = (res) => {
    if (res.data === 'Calibration completed!') {
      // Restore buttons value after action is completed
      calibrationButton.innerText = calibrationButtonTitle
      calibrationButton.style.boxShadow = '1px 1px 1px #282828'
    }
  }
}
// Save method sends a message 999 corresponding to a save request
function save() {
  Socket.send(999)
}
// machineSelect method sends a message '1-5' corresponding to the selected value
function machineSelect() {
  const value = parseInt(machineList.value, 10)
  Socket.send(value)
}

// Send degrees when the slider is slid
// NOTE: Logic below also prevents sending more than 1 message every 10ms. The ESP hates if you blast it with updates.
function sendDegrees(degrees) { // eslint-disable-line no-unused-vars
  const now = (new Date()).getTime()
	if (lastSend > now - 20) return
  // Update weight tooltip
  weightElement.innerHTML = degrees
	lastSend = now
	Socket.send(degrees)
}

// Function draws the graph and titles
// id : The type of graph, ex. repetitions, set etc..
// max : The max possible value
// value: The current value
// title: The graph's title
function drawGraph(id, max, value, title) {
  // First, we get a reference (id) to the div in the HTML which we will draw the dial in
  let canvas = document.getElementById(id)
  let ctx = canvas.getContext('2d')
  canvas.height = canvas.offsetHeight * 2
  canvas.width = canvas.offsetWidth * 2
  // Calculate the center of the div
  let centerX = canvas.width / 2
  let centerY = canvas.height / 2
  // Draw the dark (unfilled) arc
  ctx.beginPath()
  ctx.fillStyle = globalStyles.progressEmptyColor
  ctx.arc(centerX, centerY, centerY * 0.82, Math.PI * 1.5, 15, false)
  ctx.lineTo(centerX, centerY)
  ctx.fill()
  ctx.closePath()
  // Draw the colored arc representing the progress
  ctx.beginPath()
  ctx.fillStyle = globalStyles.progressColor
  ctx.arc(centerX, centerY, centerY * 0.82, Math.PI * 1.5, (Math.PI * 2 * (value / max)) + (Math.PI * 1.5), false)
  ctx.lineTo(centerX, centerY)
  ctx.fill()
  ctx.closePath()
  // Draw the background circle color
  ctx.beginPath()
  ctx.fillStyle = globalStyles.innerColor
  ctx.arc(centerX, centerY, centerY * 0.7, 0, Math.PI * 2, false)
  ctx.fill()
  ctx.closePath()
  // Add value label
  ctx.font = globalStyles.valueFont
  ctx.fillStyle = globalStyles.valueColor
  ctx.textBaseline = 'center'
  ctx.textAlign = 'center'
  ctx.fillText(value, centerX, centerY * 1.1)

  const titleId = `${id}Title`
  // Rewriting canvas to draw the title
  canvas = document.getElementById(titleId)
  ctx = canvas.getContext('2d')
  canvas.height = canvas.offsetHeight * 2
  canvas.width = canvas.offsetWidth * 2
  // Calculate the center of the div
  centerX = canvas.width / 2
  centerY = canvas.height / 2
  // Add Title label
  ctx.font = globalStyles.labelFont
  ctx.fillStyle = globalStyles.labelColor
  ctx.textBaseline = 'bottom'
  ctx.textAlign = 'center'
  ctx.fillText(title, centerX, centerY * 2)
}

// It's called each time with the structured data as parameters to update the graphs
function updateGraphs(data) {
  // Add here any new graphs, as well as in the html file and on the top of the current file (Initialize)
  if (data.extension !== undefined) {
    drawGraph('extension', 4000, data.extension, 'Extension')
  } else if (data.rest !== undefined) {
    drawGraph('rest', 9999999, data.rest, 'Rest')
  } else if (data.set !== undefined) {
    drawGraph('repetitions', 10, data.reps, 'Repetitions')
    drawGraph('power', 2000, data.power, 'Power')
    drawGraph('contraction', 180, data.contract, 'Contraction')
    drawGraph('set', 3, data.set, 'Set')
    // Update other info
    account.innerHTML = data.account
    // machine.innerHTML = data.machine
  }
}

console.log('v.1.0.6')
