// @ts-nocheck
const px = (value) => `${value}px`;
const toNumber = (value, fallback) => {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
};
const toBoolean = (value, fallback) => {
  return typeof value === "boolean" ? value : fallback;
};
const toString = (value, fallback) => {
  return typeof value === "string" ? value : fallback;
};

const manifests = window.ovlRuntime?.getManifests?.() ?? window.__OVL_MANIFESTS__ ?? [];

window.ovlHost = {
    manifests,
};

/*
{
  "name": "test_proj",
  "entry_uri": "file:///absolute/path/to/component/web/index.html",
  "window": {
    "width": 600,
    "height": 400,
    "x": 40,
    "y": 40,
    "always_on_display": true,
    "movable": true,
    "move_element": ""
  }
}
*/

const pointerX = (event) => Number.isFinite(event.screenX) ? event.screenX : event.clientX;
const pointerY = (event) => Number.isFinite(event.screenY) ? event.screenY : event.clientY;
let inputRegionFrame = 0;
let pendingFullWindowInputRegion = false;

const notifyInputRegionChanged = (fullWindow = false) => {
  pendingFullWindowInputRegion = pendingFullWindowInputRegion || fullWindow;
  if (inputRegionFrame !== 0) return;

  inputRegionFrame = requestAnimationFrame(() => {
    const shouldUseFullWindow = pendingFullWindowInputRegion;
    inputRegionFrame = 0;
    pendingFullWindowInputRegion = false;
    document.dispatchEvent(new CustomEvent("ovl:input-region-changed", {
      detail: { fullWindow: shouldUseFullWindow },
    }));
  });
};

const interactiveSelector = [
  ".ovl-interactive",
  "input",
  "textarea",
  "select",
  "button",
  "a[href]",
  "[contenteditable='true']",
  "[role='button']",
  "[role='checkbox']",
  "[role='link']",
  "[role='radio']",
  "[role='slider']",
  "[role='textbox']",
].join(",");

const canStartDrag = (event, handle) => {
  const target = event.target;
  if (target?.closest?.(interactiveSelector) !== null) {
    return false;
  }

  return handle.nodeType === 9 || target === handle || handle.contains?.(target);
};

const makeDraggable = (element, handle) => {
  let dragStart = null;
  let captureTarget = null;
  let animationFrame = 0;
  let currentX = 0;
  let currentY = 0;

  const ownerDocument = handle.nodeType === 9
    ? handle
    : handle.ownerDocument ?? document;
  const ownerWindow = ownerDocument.defaultView ?? window;

  const applyDrag = () => {
    animationFrame = 0;
    if (dragStart === null) {
      return;
    }

    element.style.transform = `translate(${currentX - dragStart.pointerX}px, ${currentY - dragStart.pointerY}px)`;
  };

  const move = (event) => {
    if (dragStart === null) {
      return;
    }

    currentX = pointerX(event);
    currentY = pointerY(event);
    if (animationFrame === 0) {
      animationFrame = ownerWindow.requestAnimationFrame(applyDrag);
    }
  };

  const stopDragging = (event) => {
    if (dragStart === null) {
      return;
    }

    currentX = event.type === "pointercancel" ? currentX : pointerX(event);
    currentY = event.type === "pointercancel" ? currentY : pointerY(event);
    const finalLeft = dragStart.left + currentX - dragStart.pointerX;
    const finalTop = dragStart.top + currentY - dragStart.pointerY;

    dragStart = null;
    ownerWindow.removeEventListener("pointermove", move);
    ownerWindow.removeEventListener("pointerup", stopDragging);
    ownerWindow.removeEventListener("pointercancel", stopDragging);

    if (animationFrame !== 0) {
      ownerWindow.cancelAnimationFrame(animationFrame);
      animationFrame = 0;
    }

    element.style.left = px(finalLeft);
    element.style.top = px(finalTop);
    element.style.transform = "";
    element.style.willChange = "";
    notifyInputRegionChanged();

    if (
      captureTarget !== null
      && captureTarget.hasPointerCapture?.(event.pointerId)
    ) {
      captureTarget.releasePointerCapture(event.pointerId);
    }
    captureTarget = null;
  };

  handle.addEventListener("pointerdown", (event) => {
    if (event.button !== 0 || !canStartDrag(event, handle)) {
      return;
    }

    dragStart = {
      pointerX: pointerX(event),
      pointerY: pointerY(event),
      left: element.offsetLeft,
      top: element.offsetTop,
    };
    currentX = dragStart.pointerX;
    currentY = dragStart.pointerY;

    element.style.willChange = "transform";
    notifyInputRegionChanged(true);
    captureTarget = event.target?.setPointerCapture ? event.target : handle;
    captureTarget.setPointerCapture?.(event.pointerId);
    ownerWindow.addEventListener("pointermove", move);
    ownerWindow.addEventListener("pointerup", stopDragging);
    ownerWindow.addEventListener("pointercancel", stopDragging);
    event.preventDefault();
  });
};

const setupIframeDragging = (iframe, wrapper, moveElement) => {
  const contentDocument = iframe.contentDocument;
  if (contentDocument === null) {
    console.warn("OVL iframe dragging unavailable: iframe document is not accessible.", {
      name: iframe.title,
    });
    return;
  }

  const className = moveElement.trim().replace(/^\./, "");
  const handles = className === ""
    ? [contentDocument]
    : Array.from(contentDocument.getElementsByClassName(className));

  if (handles.length === 0) {
    console.warn("OVL iframe drag handle not found.", {
      name: iframe.title,
      moveElement,
    });
    return;
  }

  if (className === "") {
    contentDocument.documentElement.style.cursor = "move";
    if (contentDocument.body !== null) {
      contentDocument.body.style.cursor = "move";
    }
  }

  for (const handle of handles) {
    if (handle.style !== undefined) {
      handle.style.cursor = "move";
      handle.style.touchAction = "none";
    }
    makeDraggable(wrapper, handle);
  }
};

const fragment = document.createDocumentFragment();

for (const manifest of window.ovlHost.manifests) {
  const requestedWindow = manifest.window ?? {};
  const x = toNumber(requestedWindow.x, 0);
  const y = toNumber(requestedWindow.y, 0);
  const width = toNumber(requestedWindow.width, 600);
  const height = toNumber(requestedWindow.height, 400);
  const movable = toBoolean(requestedWindow.movable, true);
  const moveElement = toString(requestedWindow.move_element, "");

  const wrapper = document.createElement("div");
  wrapper.classList.add("overlay-elem");
  wrapper.style.position = "absolute";
  wrapper.style.left = px(x);
  wrapper.style.top = px(y);
  wrapper.style.width = px(width);
  wrapper.style.height = px(height);
  wrapper.style.touchAction = "none";

  const iframe = document.createElement("iframe");
  iframe.title = manifest.name;
  iframe.classList.add("overlay");
  iframe.width = String(width);
  iframe.height = String(height);
  iframe.style.width = "100%";
  iframe.style.height = "100%";
  iframe.style.display = "block";
  iframe.style.boxSizing = "border-box";
  iframe.style.border = "0";
  iframe.style.userSelect = "none";
  iframe.src = manifest.entry_uri;

  iframe.addEventListener("load", () => {
    if (movable) {
      setupIframeDragging(iframe, wrapper, moveElement);
    }

    // console.log("OVL iframe loaded", {
    //   name: manifest.name,
    //   requested: { x, y, width, height },
    //   rendered: {
    //     x: Math.round(rect.left),
    //     y: Math.round(rect.top),
    //     width: Math.round(rect.width),
    //     height: Math.round(rect.height),
    //   },
    // });
  });

  wrapper.appendChild(iframe);
  fragment.appendChild(wrapper);
}

document.body.appendChild(fragment);
notifyInputRegionChanged();
