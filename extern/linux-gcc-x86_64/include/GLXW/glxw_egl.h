
#ifndef glxw_egl_h
#define glxw_egl_h

struct glxw_egl;

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>


#ifndef __gl_h_
#define __gl_h_
#endif

#ifdef __cplusplus
extern "C" {
#endif

int glxwInitEGL(void);
int glxwInitEGLCtx(struct glxw_egl *ctx);

struct glxw_egl {
PFNEGLCREATESYNC64KHRPROC _eglCreateSync64KHR;
PFNEGLCREATEIMAGEKHRPROC _eglCreateImageKHR;
PFNEGLDESTROYIMAGEKHRPROC _eglDestroyImageKHR;
PFNEGLLOCKSURFACEKHRPROC _eglLockSurfaceKHR;
PFNEGLUNLOCKSURFACEKHRPROC _eglUnlockSurfaceKHR;
PFNEGLQUERYSURFACE64KHRPROC _eglQuerySurface64KHR;
PFNEGLCREATESYNCKHRPROC _eglCreateSyncKHR;
PFNEGLDESTROYSYNCKHRPROC _eglDestroySyncKHR;
PFNEGLCLIENTWAITSYNCKHRPROC _eglClientWaitSyncKHR;
PFNEGLSIGNALSYNCKHRPROC _eglSignalSyncKHR;
PFNEGLGETSYNCATTRIBKHRPROC _eglGetSyncAttribKHR;
PFNEGLCREATESTREAMKHRPROC _eglCreateStreamKHR;
PFNEGLDESTROYSTREAMKHRPROC _eglDestroyStreamKHR;
PFNEGLSTREAMATTRIBKHRPROC _eglStreamAttribKHR;
PFNEGLQUERYSTREAMKHRPROC _eglQueryStreamKHR;
PFNEGLQUERYSTREAMU64KHRPROC _eglQueryStreamu64KHR;
PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC _eglStreamConsumerGLTextureExternalKHR;
PFNEGLSTREAMCONSUMERACQUIREKHRPROC _eglStreamConsumerAcquireKHR;
PFNEGLSTREAMCONSUMERRELEASEKHRPROC _eglStreamConsumerReleaseKHR;
PFNEGLGETSTREAMFILEDESCRIPTORKHRPROC _eglGetStreamFileDescriptorKHR;
PFNEGLCREATESTREAMFROMFILEDESCRIPTORKHRPROC _eglCreateStreamFromFileDescriptorKHR;
PFNEGLQUERYSTREAMTIMEKHRPROC _eglQueryStreamTimeKHR;
PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC _eglCreateStreamProducerSurfaceKHR;
PFNEGLWAITSYNCKHRPROC _eglWaitSyncKHR;
PFNEGLSETBLOBCACHEFUNCSANDROIDPROC _eglSetBlobCacheFuncsANDROID;
PFNEGLDUPNATIVEFENCEFDANDROIDPROC _eglDupNativeFenceFDANDROID;
PFNEGLQUERYSURFACEPOINTERANGLEPROC _eglQuerySurfacePointerANGLE;
PFNEGLGETPLATFORMDISPLAYEXTPROC _eglGetPlatformDisplayEXT;
PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC _eglCreatePlatformWindowSurfaceEXT;
PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC _eglCreatePlatformPixmapSurfaceEXT;
PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC _eglSwapBuffersWithDamageEXT;
PFNEGLCREATEPIXMAPSURFACEHIPROC _eglCreatePixmapSurfaceHI;
PFNEGLCREATEDRMIMAGEMESAPROC _eglCreateDRMImageMESA;
PFNEGLEXPORTDRMIMAGEMESAPROC _eglExportDRMImageMESA;
PFNEGLQUERYNATIVEDISPLAYNVPROC _eglQueryNativeDisplayNV;
PFNEGLQUERYNATIVEWINDOWNVPROC _eglQueryNativeWindowNV;
PFNEGLQUERYNATIVEPIXMAPNVPROC _eglQueryNativePixmapNV;
PFNEGLPOSTSUBBUFFERNVPROC _eglPostSubBufferNV;
PFNEGLCREATESTREAMSYNCNVPROC _eglCreateStreamSyncNV;
PFNEGLCREATEFENCESYNCNVPROC _eglCreateFenceSyncNV;
PFNEGLDESTROYSYNCNVPROC _eglDestroySyncNV;
PFNEGLFENCENVPROC _eglFenceNV;
PFNEGLCLIENTWAITSYNCNVPROC _eglClientWaitSyncNV;
PFNEGLSIGNALSYNCNVPROC _eglSignalSyncNV;
PFNEGLGETSYNCATTRIBNVPROC _eglGetSyncAttribNV;
PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC _eglGetSystemTimeFrequencyNV;
PFNEGLGETSYSTEMTIMENVPROC _eglGetSystemTimeNV;
};

extern struct glxw_egl *glxw_egl;

#define eglCreateSync64KHR (glxw_egl->_eglCreateSync64KHR)
#define eglCreateImageKHR (glxw_egl->_eglCreateImageKHR)
#define eglDestroyImageKHR (glxw_egl->_eglDestroyImageKHR)
#define eglLockSurfaceKHR (glxw_egl->_eglLockSurfaceKHR)
#define eglUnlockSurfaceKHR (glxw_egl->_eglUnlockSurfaceKHR)
#define eglQuerySurface64KHR (glxw_egl->_eglQuerySurface64KHR)
#define eglCreateSyncKHR (glxw_egl->_eglCreateSyncKHR)
#define eglDestroySyncKHR (glxw_egl->_eglDestroySyncKHR)
#define eglClientWaitSyncKHR (glxw_egl->_eglClientWaitSyncKHR)
#define eglSignalSyncKHR (glxw_egl->_eglSignalSyncKHR)
#define eglGetSyncAttribKHR (glxw_egl->_eglGetSyncAttribKHR)
#define eglCreateStreamKHR (glxw_egl->_eglCreateStreamKHR)
#define eglDestroyStreamKHR (glxw_egl->_eglDestroyStreamKHR)
#define eglStreamAttribKHR (glxw_egl->_eglStreamAttribKHR)
#define eglQueryStreamKHR (glxw_egl->_eglQueryStreamKHR)
#define eglQueryStreamu64KHR (glxw_egl->_eglQueryStreamu64KHR)
#define eglStreamConsumerGLTextureExternalKHR (glxw_egl->_eglStreamConsumerGLTextureExternalKHR)
#define eglStreamConsumerAcquireKHR (glxw_egl->_eglStreamConsumerAcquireKHR)
#define eglStreamConsumerReleaseKHR (glxw_egl->_eglStreamConsumerReleaseKHR)
#define eglGetStreamFileDescriptorKHR (glxw_egl->_eglGetStreamFileDescriptorKHR)
#define eglCreateStreamFromFileDescriptorKHR (glxw_egl->_eglCreateStreamFromFileDescriptorKHR)
#define eglQueryStreamTimeKHR (glxw_egl->_eglQueryStreamTimeKHR)
#define eglCreateStreamProducerSurfaceKHR (glxw_egl->_eglCreateStreamProducerSurfaceKHR)
#define eglWaitSyncKHR (glxw_egl->_eglWaitSyncKHR)
#define eglSetBlobCacheFuncsANDROID (glxw_egl->_eglSetBlobCacheFuncsANDROID)
#define eglDupNativeFenceFDANDROID (glxw_egl->_eglDupNativeFenceFDANDROID)
#define eglQuerySurfacePointerANGLE (glxw_egl->_eglQuerySurfacePointerANGLE)
#define eglGetPlatformDisplayEXT (glxw_egl->_eglGetPlatformDisplayEXT)
#define eglCreatePlatformWindowSurfaceEXT (glxw_egl->_eglCreatePlatformWindowSurfaceEXT)
#define eglCreatePlatformPixmapSurfaceEXT (glxw_egl->_eglCreatePlatformPixmapSurfaceEXT)
#define eglSwapBuffersWithDamageEXT (glxw_egl->_eglSwapBuffersWithDamageEXT)
#define eglCreatePixmapSurfaceHI (glxw_egl->_eglCreatePixmapSurfaceHI)
#define eglCreateDRMImageMESA (glxw_egl->_eglCreateDRMImageMESA)
#define eglExportDRMImageMESA (glxw_egl->_eglExportDRMImageMESA)
#define eglQueryNativeDisplayNV (glxw_egl->_eglQueryNativeDisplayNV)
#define eglQueryNativeWindowNV (glxw_egl->_eglQueryNativeWindowNV)
#define eglQueryNativePixmapNV (glxw_egl->_eglQueryNativePixmapNV)
#define eglPostSubBufferNV (glxw_egl->_eglPostSubBufferNV)
#define eglCreateStreamSyncNV (glxw_egl->_eglCreateStreamSyncNV)
#define eglCreateFenceSyncNV (glxw_egl->_eglCreateFenceSyncNV)
#define eglDestroySyncNV (glxw_egl->_eglDestroySyncNV)
#define eglFenceNV (glxw_egl->_eglFenceNV)
#define eglClientWaitSyncNV (glxw_egl->_eglClientWaitSyncNV)
#define eglSignalSyncNV (glxw_egl->_eglSignalSyncNV)
#define eglGetSyncAttribNV (glxw_egl->_eglGetSyncAttribNV)
#define eglGetSystemTimeFrequencyNV (glxw_egl->_eglGetSystemTimeFrequencyNV)
#define eglGetSystemTimeNV (glxw_egl->_eglGetSystemTimeNV)

#ifdef __cplusplus
}
#endif

#endif
