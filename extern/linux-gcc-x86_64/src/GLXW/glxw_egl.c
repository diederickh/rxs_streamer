#include <GLXW/glxw_egl.h>

#include <EGL/egl.h>

static void* open_libgl() { return (void*)1; }
static void close_libgl(void *libgl) { (void)libgl; }
static void *get_proc(void *libgl, const char *proc)
{
    (void)libgl;
    return eglGetProcAddress(proc);
}

static void load_procs(void *libgl, struct glxw_egl *ctx);
struct glxw_egl *glxw_egl = 0;

int glxwInitEGLCtx(struct glxw_egl *ctx)
{
    void *libgl;
    if((libgl = open_libgl()))
    {
        load_procs(libgl, ctx);
        close_libgl(libgl);
        return 0;
    }
    return -1;
}

int glxwInitEGL(void)
{
    static struct glxw_egl ctx;
    if(glxw_egl || glxwInitEGLCtx(&ctx) == 0)
    {
        glxw_egl = &ctx;
        return 0;
    }

    return -1;
}

static void load_procs(void *libgl, struct glxw_egl *ctx)
{
ctx->_eglCreateSync64KHR = (PFNEGLCREATESYNC64KHRPROC)get_proc(libgl, "eglCreateSync64KHR");
ctx->_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)get_proc(libgl, "eglCreateImageKHR");
ctx->_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)get_proc(libgl, "eglDestroyImageKHR");
ctx->_eglLockSurfaceKHR = (PFNEGLLOCKSURFACEKHRPROC)get_proc(libgl, "eglLockSurfaceKHR");
ctx->_eglUnlockSurfaceKHR = (PFNEGLUNLOCKSURFACEKHRPROC)get_proc(libgl, "eglUnlockSurfaceKHR");
ctx->_eglQuerySurface64KHR = (PFNEGLQUERYSURFACE64KHRPROC)get_proc(libgl, "eglQuerySurface64KHR");
ctx->_eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC)get_proc(libgl, "eglCreateSyncKHR");
ctx->_eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC)get_proc(libgl, "eglDestroySyncKHR");
ctx->_eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC)get_proc(libgl, "eglClientWaitSyncKHR");
ctx->_eglSignalSyncKHR = (PFNEGLSIGNALSYNCKHRPROC)get_proc(libgl, "eglSignalSyncKHR");
ctx->_eglGetSyncAttribKHR = (PFNEGLGETSYNCATTRIBKHRPROC)get_proc(libgl, "eglGetSyncAttribKHR");
ctx->_eglCreateStreamKHR = (PFNEGLCREATESTREAMKHRPROC)get_proc(libgl, "eglCreateStreamKHR");
ctx->_eglDestroyStreamKHR = (PFNEGLDESTROYSTREAMKHRPROC)get_proc(libgl, "eglDestroyStreamKHR");
ctx->_eglStreamAttribKHR = (PFNEGLSTREAMATTRIBKHRPROC)get_proc(libgl, "eglStreamAttribKHR");
ctx->_eglQueryStreamKHR = (PFNEGLQUERYSTREAMKHRPROC)get_proc(libgl, "eglQueryStreamKHR");
ctx->_eglQueryStreamu64KHR = (PFNEGLQUERYSTREAMU64KHRPROC)get_proc(libgl, "eglQueryStreamu64KHR");
ctx->_eglStreamConsumerGLTextureExternalKHR = (PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC)get_proc(libgl, "eglStreamConsumerGLTextureExternalKHR");
ctx->_eglStreamConsumerAcquireKHR = (PFNEGLSTREAMCONSUMERACQUIREKHRPROC)get_proc(libgl, "eglStreamConsumerAcquireKHR");
ctx->_eglStreamConsumerReleaseKHR = (PFNEGLSTREAMCONSUMERRELEASEKHRPROC)get_proc(libgl, "eglStreamConsumerReleaseKHR");
ctx->_eglGetStreamFileDescriptorKHR = (PFNEGLGETSTREAMFILEDESCRIPTORKHRPROC)get_proc(libgl, "eglGetStreamFileDescriptorKHR");
ctx->_eglCreateStreamFromFileDescriptorKHR = (PFNEGLCREATESTREAMFROMFILEDESCRIPTORKHRPROC)get_proc(libgl, "eglCreateStreamFromFileDescriptorKHR");
ctx->_eglQueryStreamTimeKHR = (PFNEGLQUERYSTREAMTIMEKHRPROC)get_proc(libgl, "eglQueryStreamTimeKHR");
ctx->_eglCreateStreamProducerSurfaceKHR = (PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC)get_proc(libgl, "eglCreateStreamProducerSurfaceKHR");
ctx->_eglWaitSyncKHR = (PFNEGLWAITSYNCKHRPROC)get_proc(libgl, "eglWaitSyncKHR");
ctx->_eglSetBlobCacheFuncsANDROID = (PFNEGLSETBLOBCACHEFUNCSANDROIDPROC)get_proc(libgl, "eglSetBlobCacheFuncsANDROID");
ctx->_eglDupNativeFenceFDANDROID = (PFNEGLDUPNATIVEFENCEFDANDROIDPROC)get_proc(libgl, "eglDupNativeFenceFDANDROID");
ctx->_eglQuerySurfacePointerANGLE = (PFNEGLQUERYSURFACEPOINTERANGLEPROC)get_proc(libgl, "eglQuerySurfacePointerANGLE");
ctx->_eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)get_proc(libgl, "eglGetPlatformDisplayEXT");
ctx->_eglCreatePlatformWindowSurfaceEXT = (PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC)get_proc(libgl, "eglCreatePlatformWindowSurfaceEXT");
ctx->_eglCreatePlatformPixmapSurfaceEXT = (PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC)get_proc(libgl, "eglCreatePlatformPixmapSurfaceEXT");
ctx->_eglSwapBuffersWithDamageEXT = (PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC)get_proc(libgl, "eglSwapBuffersWithDamageEXT");
ctx->_eglCreatePixmapSurfaceHI = (PFNEGLCREATEPIXMAPSURFACEHIPROC)get_proc(libgl, "eglCreatePixmapSurfaceHI");
ctx->_eglCreateDRMImageMESA = (PFNEGLCREATEDRMIMAGEMESAPROC)get_proc(libgl, "eglCreateDRMImageMESA");
ctx->_eglExportDRMImageMESA = (PFNEGLEXPORTDRMIMAGEMESAPROC)get_proc(libgl, "eglExportDRMImageMESA");
ctx->_eglQueryNativeDisplayNV = (PFNEGLQUERYNATIVEDISPLAYNVPROC)get_proc(libgl, "eglQueryNativeDisplayNV");
ctx->_eglQueryNativeWindowNV = (PFNEGLQUERYNATIVEWINDOWNVPROC)get_proc(libgl, "eglQueryNativeWindowNV");
ctx->_eglQueryNativePixmapNV = (PFNEGLQUERYNATIVEPIXMAPNVPROC)get_proc(libgl, "eglQueryNativePixmapNV");
ctx->_eglPostSubBufferNV = (PFNEGLPOSTSUBBUFFERNVPROC)get_proc(libgl, "eglPostSubBufferNV");
ctx->_eglCreateStreamSyncNV = (PFNEGLCREATESTREAMSYNCNVPROC)get_proc(libgl, "eglCreateStreamSyncNV");
ctx->_eglCreateFenceSyncNV = (PFNEGLCREATEFENCESYNCNVPROC)get_proc(libgl, "eglCreateFenceSyncNV");
ctx->_eglDestroySyncNV = (PFNEGLDESTROYSYNCNVPROC)get_proc(libgl, "eglDestroySyncNV");
ctx->_eglFenceNV = (PFNEGLFENCENVPROC)get_proc(libgl, "eglFenceNV");
ctx->_eglClientWaitSyncNV = (PFNEGLCLIENTWAITSYNCNVPROC)get_proc(libgl, "eglClientWaitSyncNV");
ctx->_eglSignalSyncNV = (PFNEGLSIGNALSYNCNVPROC)get_proc(libgl, "eglSignalSyncNV");
ctx->_eglGetSyncAttribNV = (PFNEGLGETSYNCATTRIBNVPROC)get_proc(libgl, "eglGetSyncAttribNV");
ctx->_eglGetSystemTimeFrequencyNV = (PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC)get_proc(libgl, "eglGetSystemTimeFrequencyNV");
ctx->_eglGetSystemTimeNV = (PFNEGLGETSYSTEMTIMENVPROC)get_proc(libgl, "eglGetSystemTimeNV");
}
