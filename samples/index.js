const box = document.getElementById("box");
const dragHandle = document.getElementById("dragHandle");
const timeValue = document.getElementById("time");
const dateValue = document.getElementById("date");

let dragging = false;
let dragPointerId = null;
let offsetX = 0;
let offsetY = 0;
let boxX = 0;
let boxY = 0;
let inputRegionFrameId = 0;

const timeFormatter = new Intl.DateTimeFormat(undefined, {
  hour: "2-digit",
  minute: "2-digit",
  second: "2-digit",
  hour12: false,
});

const dateFormatter = new Intl.DateTimeFormat(undefined, {
  weekday: "short",
  month: "short",
  day: "numeric",
  year: "numeric",
});

function clamp(value, min, max) {
  return Math.min(Math.max(value, min), max);
}

function clampBoxPosition(x, y) {
  const maxX = Math.max(0, window.innerWidth - box.offsetWidth);
  const maxY = Math.max(0, window.innerHeight - box.offsetHeight);

  return {
    x: clamp(x, 0, maxX),
    y: clamp(y, 0, maxY),
  };
}

function postInputRegion() {
  const regionHandler = window.webkit?.messageHandlers?.region;
  if (!regionHandler) {
    return;
  }

  const rect = box.getBoundingClientRect();
  regionHandler.postMessage({
    x: Math.round(rect.left),
    y: Math.round(rect.top),
    width: Math.round(rect.width),
    height: Math.round(rect.height),
  });
}

function scheduleInputRegionSync() {
  if (inputRegionFrameId) {
    return;
  }

  inputRegionFrameId = window.requestAnimationFrame(() => {
    inputRegionFrameId = 0;
    postInputRegion();
  });
}

function applyBoxPosition() {
  box.style.setProperty("--box-x", `${boxX}px`);
  box.style.setProperty("--box-y", `${boxY}px`);
  scheduleInputRegionSync();
}

function positionClockInitially() {
  const centered = clampBoxPosition(
    (window.innerWidth - box.offsetWidth) / 2,
    28
  );

  boxX = centered.x;
  boxY = centered.y;
  applyBoxPosition();
}

function moveClock(clientX, clientY) {
  const nextPosition = clampBoxPosition(clientX - offsetX, clientY - offsetY);
  boxX = nextPosition.x;
  boxY = nextPosition.y;
  applyBoxPosition();
}

function renderClock() {
  const now = new Date();
  timeValue.textContent = timeFormatter.format(now);
  dateValue.textContent = dateFormatter.format(now);
}

function scheduleClockTick() {
  renderClock();
  const delay = 1000 - (Date.now() % 1000) + 24;
  window.setTimeout(scheduleClockTick, delay);
}

function stopDragging() {
  if (!dragging) {
    return;
  }

  dragging = false;
  box.classList.remove("dragging");

  if (dragPointerId !== null && dragHandle.hasPointerCapture(dragPointerId)) {
    dragHandle.releasePointerCapture(dragPointerId);
  }

  dragPointerId = null;
  scheduleInputRegionSync();
}

dragHandle.addEventListener("pointerdown", (event) => {
  dragging = true;
  dragPointerId = event.pointerId;
  box.classList.add("dragging");

  const rect = box.getBoundingClientRect();
  offsetX = event.clientX - rect.left;
  offsetY = event.clientY - rect.top;

  dragHandle.setPointerCapture(event.pointerId);
  moveClock(event.clientX, event.clientY);
});

dragHandle.addEventListener("pointermove", (event) => {
  if (!dragging) {
    return;
  }

  moveClock(event.clientX, event.clientY);
});

dragHandle.addEventListener("pointerup", stopDragging);
dragHandle.addEventListener("pointercancel", stopDragging);

window.addEventListener("resize", () => {
  const clamped = clampBoxPosition(boxX, boxY);
  boxX = clamped.x;
  boxY = clamped.y;
  applyBoxPosition();
});

renderClock();
scheduleClockTick();
positionClockInitially();
