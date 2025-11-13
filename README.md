# Real-Time Edge Detection Viewer

This project is a high-performance Android application that demonstrates a complete native processing pipeline. It captures the camera feed, processes it in real-time using OpenCV in C++, and renders the resulting edge-detected video feed using OpenGL ES.

This submission fulfills the R&D Intern Assessment, demonstrating proficiency in Android (Kotlin), JNI, C++, OpenCV, OpenGL ES, and TypeScript.

## 📸 Features

* **Real-Time Camera Feed**: Uses the Camera2 API with an `ImageReader` to get efficient, raw `YUV_420_888` frames.
* **Native C++ Processing**: Frames are passed via JNI to a C++ layer with zero memory copies on the Java side.
* **OpenCV Canny Edge Detection**: `cv::Canny` is applied to the grayscale (Y-plane) of the camera feed in a background thread.
* **OpenGL ES Rendering**: The processed Canny frame is uploaded to a `GL_TEXTURE_2D` and rendered to a `GLSurfaceView` in real-time.
* **Thread-Safe Pipeline**: A `std::mutex` is used in C++ to safely pass the processed `cv::Mat` from the camera processing thread to the OpenGL rendering thread.
* **Live FPS Counter**: The GL renderer calculates the real-time FPS and securely passes it back to the Android UI thread for display.
* **Minimal Web Viewer**: A separate, minimal web page built with TypeScript demonstrates the ability to display a sample processed frame and mock-up stats.

---

## 🖼️ Screenshots & Demo

*(Add your own screenshot or GIF here for the final submission)*

`[Image or GIF of the app running with Canny edge detection]`

---

## ⚙️ Setup & Build Instructions

### 1. Android (NDK, OpenCV)

**Prerequisites:**
* Android Studio
* Android NDK (Install via Android Studio: `SDK Manager` > `SDK Tools` > `NDK (Side by side)`)

**Steps:**

1.  **Clone the repository:**
    ```bash
    git clone <your-repo-url>
    ```

2.  **Download OpenCV Android SDK:**
    * Go to the [OpenCV Releases Page](https://opencv.org/releases/).
    * Download the **"Android"** pack (e.g., `opencv-4.x.x-android-sdk.zip`).
    * Unzip the file.

3.  **Copy OpenCV SDK:**
    * Find the `sdk` folder inside the unzipped directory (e.g., `OpenCV-android-sdk/sdk`).
    * Copy this entire `sdk` folder **into** the project's `app/src/main/cpp/` directory.
    * The final path must be: `EdgeDetectionApp/app/src/main/cpp/sdk/`

4.  **Build the App:**
    * Open the project in Android Studio.
    * Sync Gradle (Click the elephant icon or `File > Sync Project with Gradle Files`).
    * Run the app on a physical Android device.

### 2. Web Viewer

1.  Navigate to the `/web` directory in your terminal:
    ```bash
    cd web
    ```
2.  Install dependencies:
    ```bash
    npm install
    ```
3.  Build the TypeScript:
    ```bash
    npm run build
    ```
4.  Open `index.html` in your browser to see the static viewer.

---

## 🏗️ Architecture & Frame Flow

The project is modular, with a clean separation of concerns.

1.  **Android (Kotlin)**:
    * `MainActivity.kt`: Manages permissions and holds the `GLSurfaceView`. It updates the UI with FPS stats received from the `GLView`.
    * `CameraController.kt`: A dedicated class to manage the complex Camera2 API. It sets up an `ImageReader` to capture `YUV_40_888` frames.
    * `GLView.kt`: A `GLSurfaceView.Renderer` that acts as a thin wrapper, forwarding all GL events (`onSurfaceCreated`, `onDrawFrame`, etc.) to the C++ layer via JNI.

2.  **JNI (C++)**:
    * `native-lib.cpp`: The JNI "glue" layer. It exposes functions like `Java_com_example_edgedetection_MainActivity_processFrameNative` and `Java_com_example_edgedetection_GLView_onGlDrawFrame`. It holds a singleton instance of our `ImageProcessor`.
    * `ImageProcessor.cpp`: The main C++ class. It:
        * Receives raw frame data from the `ImageReader` (via JNI).
        * Wraps the Y-plane in a `cv::Mat` (no copy).
        * Runs `cv::Canny` to get the edge map.
        * Stores the result in a `std::mutex`-protected `cv::Mat`.
        * On the GL thread, it uploads this `cv::Mat` to a `GL_TEXTURE_D` as `GL_LUMINANCE`.
        * Calculates the render FPS.
    * `SimpleTextureRenderer.cpp`: A reusable helper class that encapsulates all OpenGL drawing logic: shader compilation, VBO/IBO creation, and drawing a textured quad.

3.  **Web (TypeScript)**:
    * `index.html`: Contains the DOM elements (`<img>`, `<span>`).
    * `src/index.ts`: A simple `WebViewer` class that loads a static Base64 image and uses `setInterval` to simulate live stat updates.