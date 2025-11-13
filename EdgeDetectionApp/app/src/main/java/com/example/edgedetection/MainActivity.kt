package com.example.edgedetection

import android.Manifest
import android.content.pm.PackageManager
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.example.edgedetection.databinding.ActivityMainBinding
import java.nio.ByteBuffer

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var glSurfaceView: GLSurfaceView
    private lateinit var glRenderer: GLView
    private lateinit var cameraController: CameraController

    private val CAMERA_PERMISSION_REQUEST_CODE = 101
    private var nativeInitialized = false

    private external fun initNative()
    private external fun destroyNative()
    private external fun processFrameNative(
        width: Int,
        height: Int,
        yPlane: ByteBuffer,
        yStride: Int
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.fpsText.text = "FPS: ..."

        glSurfaceView = binding.glSurfaceView
        glRenderer = GLView()
        glSurfaceView.setEGLContextClientVersion(2)
        glSurfaceView.setRenderer(glRenderer)
        glSurfaceView.renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY

        if (checkCameraPermission()) {
            setupCamera()
            ensureNativeInit()
        } else {
            requestCameraPermission()
        }
    }

    private fun checkCameraPermission(): Boolean {
        return ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED
    }

    private fun requestCameraPermission() {
        ActivityCompat.requestPermissions(
            this,
            arrayOf(Manifest.permission.CAMERA),
            CAMERA_PERMISSION_REQUEST_CODE
        )
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == CAMERA_PERMISSION_REQUEST_CODE) {
            if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                setupCamera()
                ensureNativeInit()
            } else {
                Toast.makeText(this, "Camera permission is required", Toast.LENGTH_LONG).show()
                finish()
            }
        }
    }

    private fun setupCamera() {
        cameraController = CameraController(this, null) { width, height, buffer, stride ->
            processFrameNative(width, height, buffer, stride)
        }
    }

    override fun onResume() {
        super.onResume()
        glSurfaceView.onResume()
        if (::cameraController.isInitialized) {
            cameraController.start()
        }
    }

    override fun onPause() {
        if (::cameraController.isInitialized) {
            cameraController.stop()
        }
        glSurfaceView.onPause()
        super.onPause()
    }

    override fun onDestroy() {
        if (nativeInitialized) {
            destroyNative()
            nativeInitialized = false
        }
        super.onDestroy()
    }

    private fun ensureNativeInit() {
        if (!nativeInitialized) {
            initNative()
            nativeInitialized = true
        }
    }

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}