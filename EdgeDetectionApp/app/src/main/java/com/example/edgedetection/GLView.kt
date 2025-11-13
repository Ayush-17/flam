package com.example.edgedetection

import android.opengl.GLSurfaceView
import android.util.Log
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class GLView : GLSurfaceView.Renderer {

    private val TAG = "GLView"

    var fpsListener: ((Float) -> Unit)? = null

    private external fun onGlSurfaceCreated()
    private external fun onGlSurfaceChanged(width: Int, height: Int)
    private external fun onGlDrawFrame(): Float

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        Log.d(TAG, "onSurfaceCreated: delegating to native")
        onGlSurfaceCreated()
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        Log.d(TAG, "onSurfaceChanged: ${'$'}width x ${'$'}height, delegating to native")
        onGlSurfaceChanged(width, height)
    }

    override fun onDrawFrame(gl: GL10?) {
        val fps = onGlDrawFrame()
        fpsListener?.invoke(fps)
    }
}