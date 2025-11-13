"use strict";
console.log("Web viewer script loaded.");
/**
 * A sample Base64 encoded image.
 * (This is just a tiny 10x10 black PNG for this example)
 */
const DUMMY_FRAME_BASE64 = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAoAAAAKCAQAAAAnOwcgAAAAEUlEQVR42mNkYAAAAAYAAjAAv/wH/wAAAABJRU5ErkJggg==";
/**
 * Manages the web viewer page, updating the frame and stats.
 */
class WebViewer {
    constructor() {
        this.mockWidth = 640;
        this.mockHeight = 480;
        this.imgElement = document.getElementById('frame');
        this.resolutionElement = document.getElementById('resolution');
        this.fpsElement = document.getElementById('fps');
        if (this.imgElement && this.resolutionElement && this.fpsElement) {
            console.log("WebViewer initialized and found DOM elements.");
        }
        else {
            console.error("Failed to find required DOM elements.");
        }
    }
    /**
     * Loads the static sample frame and starts the stats simulation.
     */
    run() {
        if (!this.imgElement)
            return;
        // 1. Load the static sample frame
        this.imgElement.src = DUMMY_FRAME_BASE64;
        this.imgElement.onload = () => {
            this.updateStats();
        };
        // 2. Simulate live FPS updates
        setInterval(() => {
            this.updateStats();
        }, 1000); // Update stats every second
    }
    /**
     * Updates the DOM with mock stats.
     */
    updateStats() {
        // Generate a mock FPS value (e.g., 14.5 +/- 1.0)
        const mockFps = 14.5 + (Math.random() * 2) - 1;
        if (this.resolutionElement) {
            this.resolutionElement.textContent = `${this.mockWidth} x ${this.mockHeight}`;
        }
        if (this.fpsElement) {
            this.fpsElement.textContent = mockFps.toFixed(1);
        }
    }
}
// Run the app once the window loads
window.onload = () => {
    const viewer = new WebViewer();
    viewer.run();
};
