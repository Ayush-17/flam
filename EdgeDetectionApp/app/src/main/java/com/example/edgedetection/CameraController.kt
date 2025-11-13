package com.example.edgedetection

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.graphics.ImageFormat
import android.graphics.SurfaceTexture
import android.hardware.camera2.*
import android.media.ImageReader
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import android.util.Size
import android.view.Surface
import android.view.TextureView
import androidx.core.content.ContextCompat
import java.nio.ByteBuffer

class CameraController(
    private val context: Context,
    private val textureView: TextureView?,
    private val onFrameAvailable: (width: Int, height: Int, buffer: ByteBuffer, rowStride: Int) -> Unit
) {

    private val TAG = "CameraController"
    private val CAMERA_ID = "0"

    private var cameraDevice: CameraDevice? = null
    private var captureSession: CameraCaptureSession? = null
    private var imageReader: ImageReader? = null
    private lateinit var captureSize: Size

    private lateinit var backgroundThread: HandlerThread
    private lateinit var backgroundHandler: Handler

    private val cameraManager: CameraManager by lazy {
        context.getSystemService(Context.CAMERA_SERVICE) as CameraManager
    }

    private val onImageAvailableListener = ImageReader.OnImageAvailableListener { reader ->
        val image = reader.acquireLatestImage() ?: return@OnImageAvailableListener
        val plane = image.planes.firstOrNull()
        if (plane != null) {
            val buffer = plane.buffer
            onFrameAvailable(image.width, image.height, buffer, plane.rowStride)
        }
        image.close()
    }

    private val cameraStateCallback = object : CameraDevice.StateCallback() {
        override fun onOpened(camera: CameraDevice) {
            Log.d(TAG, "Camera device opened")
            cameraDevice = camera
            createPreviewSession()
        }

        override fun onDisconnected(camera: CameraDevice) {
            Log.w(TAG, "Camera device disconnected")
            camera.close()
            cameraDevice = null
        }

        override fun onError(camera: CameraDevice, error: Int) {
            Log.e(TAG, "Camera device error: $error")
            camera.close()
            cameraDevice = null
        }
    }

    fun start() {
        startBackgroundThread()
        if (textureView != null) {
            if (textureView.isAvailable) {
                openCamera()
            } else {
                textureView.surfaceTextureListener = object : TextureView.SurfaceTextureListener {
                    override fun onSurfaceTextureAvailable(surface: SurfaceTexture, width: Int, height: Int) {
                        Log.d(TAG, "TextureView available")
                        openCamera()
                    }

                    override fun onSurfaceTextureSizeChanged(surface: SurfaceTexture, width: Int, height: Int) {}

                    override fun onSurfaceTextureDestroyed(surface: SurfaceTexture): Boolean = true

                    override fun onSurfaceTextureUpdated(surface: SurfaceTexture) {}
                }
            }
        } else {
            openCamera()
        }
    }

    fun stop() {
        closeCamera()
        stopBackgroundThread()
        imageReader?.close()
        imageReader = null
    }

    private fun openCamera() {
        if (ContextCompat.checkSelfPermission(context, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            Log.e(TAG, "Camera permission not granted")
            return
        }

        try {
            Log.d(TAG, "Opening camera...")
            val characteristics = cameraManager.getCameraCharacteristics(CAMERA_ID)
            val streamConfigMap = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)
            val yuvSizes = streamConfigMap?.getOutputSizes(ImageFormat.YUV_420_888)
            captureSize = yuvSizes?.firstOrNull { it.width == 640 && it.height == 480 }
                ?: yuvSizes?.firstOrNull()
                ?: Size(1280, 720)
            Log.d(TAG, "Selected capture size: $captureSize")

            imageReader = ImageReader.newInstance(
                captureSize.width,
                captureSize.height,
                ImageFormat.YUV_420_888,
                2
            ).apply {
                setOnImageAvailableListener(onImageAvailableListener, backgroundHandler)
            }

            cameraManager.openCamera(CAMERA_ID, cameraStateCallback, backgroundHandler)
        } catch (e: CameraAccessException) {
            Log.e(TAG, "Failed to open camera", e)
        }
    }

    private fun closeCamera() {
        captureSession?.close()
        captureSession = null
        cameraDevice?.close()
        cameraDevice = null
        Log.d(TAG, "Camera closed")
    }

    private fun createPreviewSession() {
        val camera = cameraDevice ?: return
        val processingSurface = imageReader?.surface ?: return

        val surfaces = mutableListOf(processingSurface)
        val texture = textureView?.surfaceTexture
        if (texture != null) {
            if (!::captureSize.isInitialized) {
                captureSize = Size(textureView.width.coerceAtLeast(1), textureView.height.coerceAtLeast(1))
            }
            texture.setDefaultBufferSize(captureSize.width, captureSize.height)
            surfaces.add(Surface(texture))
        }

        try {
            val requestBuilder = camera.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW)
            surfaces.forEach { requestBuilder.addTarget(it) }

            camera.createCaptureSession(
                surfaces,
                object : CameraCaptureSession.StateCallback() {
                    override fun onConfigured(session: CameraCaptureSession) {
                        Log.d(TAG, "Capture session configured")
                        captureSession = session
                        try {
                            requestBuilder.set(
                                CaptureRequest.CONTROL_AF_MODE,
                                CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE
                            )
                            session.setRepeatingRequest(requestBuilder.build(), null, backgroundHandler)
                        } catch (e: CameraAccessException) {
                            Log.e(TAG, "Failed to set repeating request", e)
                        }
                    }

                    override fun onConfigureFailed(session: CameraCaptureSession) {
                        Log.e(TAG, "Capture session configuration failed")
                    }
                },
                backgroundHandler
            )
        } catch (e: CameraAccessException) {
            Log.e(TAG, "Failed to create capture session", e)
        }
    }

    private fun startBackgroundThread() {
        backgroundThread = HandlerThread("CameraBackground").also { it.start() }
        backgroundHandler = Handler(backgroundThread.looper)
    }

    private fun stopBackgroundThread() {
        backgroundThread.quitSafely()
        try {
            backgroundThread.join()
        } catch (e: InterruptedException) {
            Log.e(TAG, "Failed to stop background thread", e)
        }
    }
}