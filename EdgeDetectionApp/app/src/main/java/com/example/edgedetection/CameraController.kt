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
    private val textureView: TextureView,
    private val onFrameAvailable: (width: Int, height: Int, buffer: ByteBuffer, rowStride: Int) -> Unit
) {

    private val TAG = "CameraController"
    private val CAMERA_ID = "0" // Use rear camera

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
            val width = image.width
            val height = image.height
            val rowStride = plane.rowStride
            onFrameAvailable(width, height, buffer, rowStride)
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
    }

    fun stop() {
        closeCamera()
        stopBackgroundThread()
        imageReader?.close()
        imageReader = null
    }

    private fun openCamera() {
        if (ContextCompat.checkSelfPermission(context, Manifest.permission.CAMERA)
            != PackageManager.PERMISSION_GRANTED
        ) {
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
        val texture = textureView.surfaceTexture ?: return
        if (!::captureSize.isInitialized) {
            captureSize = Size(textureView.width.coerceAtLeast(1), textureView.height.coerceAtLeast(1))
        }
        texture.setDefaultBufferSize(captureSize.width, captureSize.height)
        val previewSurface = Surface(texture)
        val processingSurface = imageReader?.surface ?: return

        try {
            val previewRequestBuilder = cameraDevice?.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW)
            previewRequestBuilder?.addTarget(previewSurface)
            previewRequestBuilder?.addTarget(processingSurface)

            cameraDevice?.createCaptureSession(
                listOf(previewSurface, processingSurface),
                object : CameraCaptureSession.StateCallback() {
                    override fun onConfigured(session: CameraCaptureSession) {
                        Log.d(TAG, "Capture session configured")
                        captureSession = session
                        try {
                            previewRequestBuilder?.let {
                                it.set(
                                    CaptureRequest.CONTROL_AF_MODE,
                                    CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE
                                )
                                session.setRepeatingRequest(it.build(), null, backgroundHandler)
                            }
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