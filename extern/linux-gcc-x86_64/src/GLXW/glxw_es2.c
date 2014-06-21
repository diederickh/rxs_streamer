#include <GLXW/glxw_es2.h>

#include <EGL/egl.h>

static void* open_libgl() { return (void*)1; }
static void close_libgl(void *libgl) { (void)libgl; }
static void *get_proc(void *libgl, const char *proc)
{
    (void)libgl;
    return eglGetProcAddress(proc);
}

static void load_procs(void *libgl, struct glxw_es2 *ctx);
struct glxw_es2 *glxw_es2 = 0;

int glxwInitCtx(struct glxw_es2 *ctx)
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

int glxwInit(void)
{
    static struct glxw_es2 ctx;
    if(glxw_es2 || glxwInitCtx(&ctx) == 0)
    {
        glxw_es2 = &ctx;
        return 0;
    }

    return -1;
}

static void load_procs(void *libgl, struct glxw_es2 *ctx)
{
ctx->_glDebugMessageControlKHR = (PFNGLDEBUGMESSAGECONTROLKHRPROC)get_proc(libgl, "glDebugMessageControlKHR");
ctx->_glDebugMessageInsertKHR = (PFNGLDEBUGMESSAGEINSERTKHRPROC)get_proc(libgl, "glDebugMessageInsertKHR");
ctx->_glDebugMessageCallbackKHR = (PFNGLDEBUGMESSAGECALLBACKKHRPROC)get_proc(libgl, "glDebugMessageCallbackKHR");
ctx->_glGetDebugMessageLogKHR = (PFNGLGETDEBUGMESSAGELOGKHRPROC)get_proc(libgl, "glGetDebugMessageLogKHR");
ctx->_glPushDebugGroupKHR = (PFNGLPUSHDEBUGGROUPKHRPROC)get_proc(libgl, "glPushDebugGroupKHR");
ctx->_glPopDebugGroupKHR = (PFNGLPOPDEBUGGROUPKHRPROC)get_proc(libgl, "glPopDebugGroupKHR");
ctx->_glObjectLabelKHR = (PFNGLOBJECTLABELKHRPROC)get_proc(libgl, "glObjectLabelKHR");
ctx->_glGetObjectLabelKHR = (PFNGLGETOBJECTLABELKHRPROC)get_proc(libgl, "glGetObjectLabelKHR");
ctx->_glObjectPtrLabelKHR = (PFNGLOBJECTPTRLABELKHRPROC)get_proc(libgl, "glObjectPtrLabelKHR");
ctx->_glGetObjectPtrLabelKHR = (PFNGLGETOBJECTPTRLABELKHRPROC)get_proc(libgl, "glGetObjectPtrLabelKHR");
ctx->_glGetPointervKHR = (PFNGLGETPOINTERVKHRPROC)get_proc(libgl, "glGetPointervKHR");
ctx->_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)get_proc(libgl, "glEGLImageTargetTexture2DOES");
ctx->_glEGLImageTargetRenderbufferStorageOES = (PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC)get_proc(libgl, "glEGLImageTargetRenderbufferStorageOES");
ctx->_glGetProgramBinaryOES = (PFNGLGETPROGRAMBINARYOESPROC)get_proc(libgl, "glGetProgramBinaryOES");
ctx->_glProgramBinaryOES = (PFNGLPROGRAMBINARYOESPROC)get_proc(libgl, "glProgramBinaryOES");
ctx->_glMapBufferOES = (PFNGLMAPBUFFEROESPROC)get_proc(libgl, "glMapBufferOES");
ctx->_glUnmapBufferOES = (PFNGLUNMAPBUFFEROESPROC)get_proc(libgl, "glUnmapBufferOES");
ctx->_glGetBufferPointervOES = (PFNGLGETBUFFERPOINTERVOESPROC)get_proc(libgl, "glGetBufferPointervOES");
ctx->_glTexImage3DOES = (PFNGLTEXIMAGE3DOESPROC)get_proc(libgl, "glTexImage3DOES");
ctx->_glTexSubImage3DOES = (PFNGLTEXSUBIMAGE3DOESPROC)get_proc(libgl, "glTexSubImage3DOES");
ctx->_glCopyTexSubImage3DOES = (PFNGLCOPYTEXSUBIMAGE3DOESPROC)get_proc(libgl, "glCopyTexSubImage3DOES");
ctx->_glCompressedTexImage3DOES = (PFNGLCOMPRESSEDTEXIMAGE3DOESPROC)get_proc(libgl, "glCompressedTexImage3DOES");
ctx->_glCompressedTexSubImage3DOES = (PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC)get_proc(libgl, "glCompressedTexSubImage3DOES");
ctx->_glFramebufferTexture3DOES = (PFNGLFRAMEBUFFERTEXTURE3DOESPROC)get_proc(libgl, "glFramebufferTexture3DOES");
ctx->_glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC)get_proc(libgl, "glBindVertexArrayOES");
ctx->_glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC)get_proc(libgl, "glDeleteVertexArraysOES");
ctx->_glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)get_proc(libgl, "glGenVertexArraysOES");
ctx->_glIsVertexArrayOES = (PFNGLISVERTEXARRAYOESPROC)get_proc(libgl, "glIsVertexArrayOES");
ctx->_glGetPerfMonitorGroupsAMD = (PFNGLGETPERFMONITORGROUPSAMDPROC)get_proc(libgl, "glGetPerfMonitorGroupsAMD");
ctx->_glGetPerfMonitorCountersAMD = (PFNGLGETPERFMONITORCOUNTERSAMDPROC)get_proc(libgl, "glGetPerfMonitorCountersAMD");
ctx->_glGetPerfMonitorGroupStringAMD = (PFNGLGETPERFMONITORGROUPSTRINGAMDPROC)get_proc(libgl, "glGetPerfMonitorGroupStringAMD");
ctx->_glGetPerfMonitorCounterStringAMD = (PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC)get_proc(libgl, "glGetPerfMonitorCounterStringAMD");
ctx->_glGetPerfMonitorCounterInfoAMD = (PFNGLGETPERFMONITORCOUNTERINFOAMDPROC)get_proc(libgl, "glGetPerfMonitorCounterInfoAMD");
ctx->_glGenPerfMonitorsAMD = (PFNGLGENPERFMONITORSAMDPROC)get_proc(libgl, "glGenPerfMonitorsAMD");
ctx->_glDeletePerfMonitorsAMD = (PFNGLDELETEPERFMONITORSAMDPROC)get_proc(libgl, "glDeletePerfMonitorsAMD");
ctx->_glSelectPerfMonitorCountersAMD = (PFNGLSELECTPERFMONITORCOUNTERSAMDPROC)get_proc(libgl, "glSelectPerfMonitorCountersAMD");
ctx->_glBeginPerfMonitorAMD = (PFNGLBEGINPERFMONITORAMDPROC)get_proc(libgl, "glBeginPerfMonitorAMD");
ctx->_glEndPerfMonitorAMD = (PFNGLENDPERFMONITORAMDPROC)get_proc(libgl, "glEndPerfMonitorAMD");
ctx->_glGetPerfMonitorCounterDataAMD = (PFNGLGETPERFMONITORCOUNTERDATAAMDPROC)get_proc(libgl, "glGetPerfMonitorCounterDataAMD");
ctx->_glBlitFramebufferANGLE = (PFNGLBLITFRAMEBUFFERANGLEPROC)get_proc(libgl, "glBlitFramebufferANGLE");
ctx->_glRenderbufferStorageMultisampleANGLE = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC)get_proc(libgl, "glRenderbufferStorageMultisampleANGLE");
ctx->_glDrawArraysInstancedANGLE = (PFNGLDRAWARRAYSINSTANCEDANGLEPROC)get_proc(libgl, "glDrawArraysInstancedANGLE");
ctx->_glDrawElementsInstancedANGLE = (PFNGLDRAWELEMENTSINSTANCEDANGLEPROC)get_proc(libgl, "glDrawElementsInstancedANGLE");
ctx->_glVertexAttribDivisorANGLE = (PFNGLVERTEXATTRIBDIVISORANGLEPROC)get_proc(libgl, "glVertexAttribDivisorANGLE");
ctx->_glGetTranslatedShaderSourceANGLE = (PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC)get_proc(libgl, "glGetTranslatedShaderSourceANGLE");
ctx->_glCopyTextureLevelsAPPLE = (PFNGLCOPYTEXTURELEVELSAPPLEPROC)get_proc(libgl, "glCopyTextureLevelsAPPLE");
ctx->_glRenderbufferStorageMultisampleAPPLE = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC)get_proc(libgl, "glRenderbufferStorageMultisampleAPPLE");
ctx->_glResolveMultisampleFramebufferAPPLE = (PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC)get_proc(libgl, "glResolveMultisampleFramebufferAPPLE");
ctx->_glFenceSyncAPPLE = (PFNGLFENCESYNCAPPLEPROC)get_proc(libgl, "glFenceSyncAPPLE");
ctx->_glIsSyncAPPLE = (PFNGLISSYNCAPPLEPROC)get_proc(libgl, "glIsSyncAPPLE");
ctx->_glDeleteSyncAPPLE = (PFNGLDELETESYNCAPPLEPROC)get_proc(libgl, "glDeleteSyncAPPLE");
ctx->_glClientWaitSyncAPPLE = (PFNGLCLIENTWAITSYNCAPPLEPROC)get_proc(libgl, "glClientWaitSyncAPPLE");
ctx->_glWaitSyncAPPLE = (PFNGLWAITSYNCAPPLEPROC)get_proc(libgl, "glWaitSyncAPPLE");
ctx->_glGetInteger64vAPPLE = (PFNGLGETINTEGER64VAPPLEPROC)get_proc(libgl, "glGetInteger64vAPPLE");
ctx->_glGetSyncivAPPLE = (PFNGLGETSYNCIVAPPLEPROC)get_proc(libgl, "glGetSyncivAPPLE");
ctx->_glLabelObjectEXT = (PFNGLLABELOBJECTEXTPROC)get_proc(libgl, "glLabelObjectEXT");
ctx->_glGetObjectLabelEXT = (PFNGLGETOBJECTLABELEXTPROC)get_proc(libgl, "glGetObjectLabelEXT");
ctx->_glInsertEventMarkerEXT = (PFNGLINSERTEVENTMARKEREXTPROC)get_proc(libgl, "glInsertEventMarkerEXT");
ctx->_glPushGroupMarkerEXT = (PFNGLPUSHGROUPMARKEREXTPROC)get_proc(libgl, "glPushGroupMarkerEXT");
ctx->_glPopGroupMarkerEXT = (PFNGLPOPGROUPMARKEREXTPROC)get_proc(libgl, "glPopGroupMarkerEXT");
ctx->_glDiscardFramebufferEXT = (PFNGLDISCARDFRAMEBUFFEREXTPROC)get_proc(libgl, "glDiscardFramebufferEXT");
ctx->_glGenQueriesEXT = (PFNGLGENQUERIESEXTPROC)get_proc(libgl, "glGenQueriesEXT");
ctx->_glDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC)get_proc(libgl, "glDeleteQueriesEXT");
ctx->_glIsQueryEXT = (PFNGLISQUERYEXTPROC)get_proc(libgl, "glIsQueryEXT");
ctx->_glBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC)get_proc(libgl, "glBeginQueryEXT");
ctx->_glEndQueryEXT = (PFNGLENDQUERYEXTPROC)get_proc(libgl, "glEndQueryEXT");
ctx->_glQueryCounterEXT = (PFNGLQUERYCOUNTEREXTPROC)get_proc(libgl, "glQueryCounterEXT");
ctx->_glGetQueryivEXT = (PFNGLGETQUERYIVEXTPROC)get_proc(libgl, "glGetQueryivEXT");
ctx->_glGetQueryObjectivEXT = (PFNGLGETQUERYOBJECTIVEXTPROC)get_proc(libgl, "glGetQueryObjectivEXT");
ctx->_glGetQueryObjectuivEXT = (PFNGLGETQUERYOBJECTUIVEXTPROC)get_proc(libgl, "glGetQueryObjectuivEXT");
ctx->_glGetQueryObjecti64vEXT = (PFNGLGETQUERYOBJECTI64VEXTPROC)get_proc(libgl, "glGetQueryObjecti64vEXT");
ctx->_glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC)get_proc(libgl, "glGetQueryObjectui64vEXT");
ctx->_glDrawBuffersEXT = (PFNGLDRAWBUFFERSEXTPROC)get_proc(libgl, "glDrawBuffersEXT");
ctx->_glDrawArraysInstancedEXT = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)get_proc(libgl, "glDrawArraysInstancedEXT");
ctx->_glDrawElementsInstancedEXT = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)get_proc(libgl, "glDrawElementsInstancedEXT");
ctx->_glVertexAttribDivisorEXT = (PFNGLVERTEXATTRIBDIVISOREXTPROC)get_proc(libgl, "glVertexAttribDivisorEXT");
ctx->_glMapBufferRangeEXT = (PFNGLMAPBUFFERRANGEEXTPROC)get_proc(libgl, "glMapBufferRangeEXT");
ctx->_glFlushMappedBufferRangeEXT = (PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC)get_proc(libgl, "glFlushMappedBufferRangeEXT");
ctx->_glMultiDrawArraysEXT = (PFNGLMULTIDRAWARRAYSEXTPROC)get_proc(libgl, "glMultiDrawArraysEXT");
ctx->_glMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC)get_proc(libgl, "glMultiDrawElementsEXT");
ctx->_glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)get_proc(libgl, "glRenderbufferStorageMultisampleEXT");
ctx->_glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)get_proc(libgl, "glFramebufferTexture2DMultisampleEXT");
ctx->_glReadBufferIndexedEXT = (PFNGLREADBUFFERINDEXEDEXTPROC)get_proc(libgl, "glReadBufferIndexedEXT");
ctx->_glDrawBuffersIndexedEXT = (PFNGLDRAWBUFFERSINDEXEDEXTPROC)get_proc(libgl, "glDrawBuffersIndexedEXT");
ctx->_glGetIntegeri_vEXT = (PFNGLGETINTEGERI_VEXTPROC)get_proc(libgl, "glGetIntegeri_vEXT");
ctx->_glGetGraphicsResetStatusEXT = (PFNGLGETGRAPHICSRESETSTATUSEXTPROC)get_proc(libgl, "glGetGraphicsResetStatusEXT");
ctx->_glReadnPixelsEXT = (PFNGLREADNPIXELSEXTPROC)get_proc(libgl, "glReadnPixelsEXT");
ctx->_glGetnUniformfvEXT = (PFNGLGETNUNIFORMFVEXTPROC)get_proc(libgl, "glGetnUniformfvEXT");
ctx->_glGetnUniformivEXT = (PFNGLGETNUNIFORMIVEXTPROC)get_proc(libgl, "glGetnUniformivEXT");
ctx->_glActiveShaderProgramEXT = (PFNGLACTIVESHADERPROGRAMEXTPROC)get_proc(libgl, "glActiveShaderProgramEXT");
ctx->_glBindProgramPipelineEXT = (PFNGLBINDPROGRAMPIPELINEEXTPROC)get_proc(libgl, "glBindProgramPipelineEXT");
ctx->_glCreateShaderProgramvEXT = (PFNGLCREATESHADERPROGRAMVEXTPROC)get_proc(libgl, "glCreateShaderProgramvEXT");
ctx->_glDeleteProgramPipelinesEXT = (PFNGLDELETEPROGRAMPIPELINESEXTPROC)get_proc(libgl, "glDeleteProgramPipelinesEXT");
ctx->_glGenProgramPipelinesEXT = (PFNGLGENPROGRAMPIPELINESEXTPROC)get_proc(libgl, "glGenProgramPipelinesEXT");
ctx->_glGetProgramPipelineInfoLogEXT = (PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC)get_proc(libgl, "glGetProgramPipelineInfoLogEXT");
ctx->_glGetProgramPipelineivEXT = (PFNGLGETPROGRAMPIPELINEIVEXTPROC)get_proc(libgl, "glGetProgramPipelineivEXT");
ctx->_glIsProgramPipelineEXT = (PFNGLISPROGRAMPIPELINEEXTPROC)get_proc(libgl, "glIsProgramPipelineEXT");
ctx->_glProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC)get_proc(libgl, "glProgramParameteriEXT");
ctx->_glProgramUniform1fEXT = (PFNGLPROGRAMUNIFORM1FEXTPROC)get_proc(libgl, "glProgramUniform1fEXT");
ctx->_glProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC)get_proc(libgl, "glProgramUniform1fvEXT");
ctx->_glProgramUniform1iEXT = (PFNGLPROGRAMUNIFORM1IEXTPROC)get_proc(libgl, "glProgramUniform1iEXT");
ctx->_glProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC)get_proc(libgl, "glProgramUniform1ivEXT");
ctx->_glProgramUniform2fEXT = (PFNGLPROGRAMUNIFORM2FEXTPROC)get_proc(libgl, "glProgramUniform2fEXT");
ctx->_glProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC)get_proc(libgl, "glProgramUniform2fvEXT");
ctx->_glProgramUniform2iEXT = (PFNGLPROGRAMUNIFORM2IEXTPROC)get_proc(libgl, "glProgramUniform2iEXT");
ctx->_glProgramUniform2ivEXT = (PFNGLPROGRAMUNIFORM2IVEXTPROC)get_proc(libgl, "glProgramUniform2ivEXT");
ctx->_glProgramUniform3fEXT = (PFNGLPROGRAMUNIFORM3FEXTPROC)get_proc(libgl, "glProgramUniform3fEXT");
ctx->_glProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC)get_proc(libgl, "glProgramUniform3fvEXT");
ctx->_glProgramUniform3iEXT = (PFNGLPROGRAMUNIFORM3IEXTPROC)get_proc(libgl, "glProgramUniform3iEXT");
ctx->_glProgramUniform3ivEXT = (PFNGLPROGRAMUNIFORM3IVEXTPROC)get_proc(libgl, "glProgramUniform3ivEXT");
ctx->_glProgramUniform4fEXT = (PFNGLPROGRAMUNIFORM4FEXTPROC)get_proc(libgl, "glProgramUniform4fEXT");
ctx->_glProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC)get_proc(libgl, "glProgramUniform4fvEXT");
ctx->_glProgramUniform4iEXT = (PFNGLPROGRAMUNIFORM4IEXTPROC)get_proc(libgl, "glProgramUniform4iEXT");
ctx->_glProgramUniform4ivEXT = (PFNGLPROGRAMUNIFORM4IVEXTPROC)get_proc(libgl, "glProgramUniform4ivEXT");
ctx->_glProgramUniformMatrix2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC)get_proc(libgl, "glProgramUniformMatrix2fvEXT");
ctx->_glProgramUniformMatrix3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC)get_proc(libgl, "glProgramUniformMatrix3fvEXT");
ctx->_glProgramUniformMatrix4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC)get_proc(libgl, "glProgramUniformMatrix4fvEXT");
ctx->_glUseProgramStagesEXT = (PFNGLUSEPROGRAMSTAGESEXTPROC)get_proc(libgl, "glUseProgramStagesEXT");
ctx->_glValidateProgramPipelineEXT = (PFNGLVALIDATEPROGRAMPIPELINEEXTPROC)get_proc(libgl, "glValidateProgramPipelineEXT");
ctx->_glProgramUniform1uiEXT = (PFNGLPROGRAMUNIFORM1UIEXTPROC)get_proc(libgl, "glProgramUniform1uiEXT");
ctx->_glProgramUniform2uiEXT = (PFNGLPROGRAMUNIFORM2UIEXTPROC)get_proc(libgl, "glProgramUniform2uiEXT");
ctx->_glProgramUniform3uiEXT = (PFNGLPROGRAMUNIFORM3UIEXTPROC)get_proc(libgl, "glProgramUniform3uiEXT");
ctx->_glProgramUniform4uiEXT = (PFNGLPROGRAMUNIFORM4UIEXTPROC)get_proc(libgl, "glProgramUniform4uiEXT");
ctx->_glProgramUniform1uivEXT = (PFNGLPROGRAMUNIFORM1UIVEXTPROC)get_proc(libgl, "glProgramUniform1uivEXT");
ctx->_glProgramUniform2uivEXT = (PFNGLPROGRAMUNIFORM2UIVEXTPROC)get_proc(libgl, "glProgramUniform2uivEXT");
ctx->_glProgramUniform3uivEXT = (PFNGLPROGRAMUNIFORM3UIVEXTPROC)get_proc(libgl, "glProgramUniform3uivEXT");
ctx->_glProgramUniform4uivEXT = (PFNGLPROGRAMUNIFORM4UIVEXTPROC)get_proc(libgl, "glProgramUniform4uivEXT");
ctx->_glProgramUniformMatrix2x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC)get_proc(libgl, "glProgramUniformMatrix2x3fvEXT");
ctx->_glProgramUniformMatrix3x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC)get_proc(libgl, "glProgramUniformMatrix3x2fvEXT");
ctx->_glProgramUniformMatrix2x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC)get_proc(libgl, "glProgramUniformMatrix2x4fvEXT");
ctx->_glProgramUniformMatrix4x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC)get_proc(libgl, "glProgramUniformMatrix4x2fvEXT");
ctx->_glProgramUniformMatrix3x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC)get_proc(libgl, "glProgramUniformMatrix3x4fvEXT");
ctx->_glProgramUniformMatrix4x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC)get_proc(libgl, "glProgramUniformMatrix4x3fvEXT");
ctx->_glTexStorage1DEXT = (PFNGLTEXSTORAGE1DEXTPROC)get_proc(libgl, "glTexStorage1DEXT");
ctx->_glTexStorage2DEXT = (PFNGLTEXSTORAGE2DEXTPROC)get_proc(libgl, "glTexStorage2DEXT");
ctx->_glTexStorage3DEXT = (PFNGLTEXSTORAGE3DEXTPROC)get_proc(libgl, "glTexStorage3DEXT");
ctx->_glTextureStorage1DEXT = (PFNGLTEXTURESTORAGE1DEXTPROC)get_proc(libgl, "glTextureStorage1DEXT");
ctx->_glTextureStorage2DEXT = (PFNGLTEXTURESTORAGE2DEXTPROC)get_proc(libgl, "glTextureStorage2DEXT");
ctx->_glTextureStorage3DEXT = (PFNGLTEXTURESTORAGE3DEXTPROC)get_proc(libgl, "glTextureStorage3DEXT");
ctx->_glRenderbufferStorageMultisampleIMG = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC)get_proc(libgl, "glRenderbufferStorageMultisampleIMG");
ctx->_glFramebufferTexture2DMultisampleIMG = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC)get_proc(libgl, "glFramebufferTexture2DMultisampleIMG");
ctx->_glBeginPerfQueryINTEL = (PFNGLBEGINPERFQUERYINTELPROC)get_proc(libgl, "glBeginPerfQueryINTEL");
ctx->_glCreatePerfQueryINTEL = (PFNGLCREATEPERFQUERYINTELPROC)get_proc(libgl, "glCreatePerfQueryINTEL");
ctx->_glDeletePerfQueryINTEL = (PFNGLDELETEPERFQUERYINTELPROC)get_proc(libgl, "glDeletePerfQueryINTEL");
ctx->_glEndPerfQueryINTEL = (PFNGLENDPERFQUERYINTELPROC)get_proc(libgl, "glEndPerfQueryINTEL");
ctx->_glGetFirstPerfQueryIdINTEL = (PFNGLGETFIRSTPERFQUERYIDINTELPROC)get_proc(libgl, "glGetFirstPerfQueryIdINTEL");
ctx->_glGetNextPerfQueryIdINTEL = (PFNGLGETNEXTPERFQUERYIDINTELPROC)get_proc(libgl, "glGetNextPerfQueryIdINTEL");
ctx->_glGetPerfCounterInfoINTEL = (PFNGLGETPERFCOUNTERINFOINTELPROC)get_proc(libgl, "glGetPerfCounterInfoINTEL");
ctx->_glGetPerfQueryDataINTEL = (PFNGLGETPERFQUERYDATAINTELPROC)get_proc(libgl, "glGetPerfQueryDataINTEL");
ctx->_glGetPerfQueryIdByNameINTEL = (PFNGLGETPERFQUERYIDBYNAMEINTELPROC)get_proc(libgl, "glGetPerfQueryIdByNameINTEL");
ctx->_glGetPerfQueryInfoINTEL = (PFNGLGETPERFQUERYINFOINTELPROC)get_proc(libgl, "glGetPerfQueryInfoINTEL");
ctx->_glBlendParameteriNV = (PFNGLBLENDPARAMETERINVPROC)get_proc(libgl, "glBlendParameteriNV");
ctx->_glBlendBarrierNV = (PFNGLBLENDBARRIERNVPROC)get_proc(libgl, "glBlendBarrierNV");
ctx->_glCopyBufferSubDataNV = (PFNGLCOPYBUFFERSUBDATANVPROC)get_proc(libgl, "glCopyBufferSubDataNV");
ctx->_glCoverageMaskNV = (PFNGLCOVERAGEMASKNVPROC)get_proc(libgl, "glCoverageMaskNV");
ctx->_glCoverageOperationNV = (PFNGLCOVERAGEOPERATIONNVPROC)get_proc(libgl, "glCoverageOperationNV");
ctx->_glDrawBuffersNV = (PFNGLDRAWBUFFERSNVPROC)get_proc(libgl, "glDrawBuffersNV");
ctx->_glDrawArraysInstancedNV = (PFNGLDRAWARRAYSINSTANCEDNVPROC)get_proc(libgl, "glDrawArraysInstancedNV");
ctx->_glDrawElementsInstancedNV = (PFNGLDRAWELEMENTSINSTANCEDNVPROC)get_proc(libgl, "glDrawElementsInstancedNV");
ctx->_glDeleteFencesNV = (PFNGLDELETEFENCESNVPROC)get_proc(libgl, "glDeleteFencesNV");
ctx->_glGenFencesNV = (PFNGLGENFENCESNVPROC)get_proc(libgl, "glGenFencesNV");
ctx->_glIsFenceNV = (PFNGLISFENCENVPROC)get_proc(libgl, "glIsFenceNV");
ctx->_glTestFenceNV = (PFNGLTESTFENCENVPROC)get_proc(libgl, "glTestFenceNV");
ctx->_glGetFenceivNV = (PFNGLGETFENCEIVNVPROC)get_proc(libgl, "glGetFenceivNV");
ctx->_glFinishFenceNV = (PFNGLFINISHFENCENVPROC)get_proc(libgl, "glFinishFenceNV");
ctx->_glSetFenceNV = (PFNGLSETFENCENVPROC)get_proc(libgl, "glSetFenceNV");
ctx->_glBlitFramebufferNV = (PFNGLBLITFRAMEBUFFERNVPROC)get_proc(libgl, "glBlitFramebufferNV");
ctx->_glRenderbufferStorageMultisampleNV = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC)get_proc(libgl, "glRenderbufferStorageMultisampleNV");
ctx->_glVertexAttribDivisorNV = (PFNGLVERTEXATTRIBDIVISORNVPROC)get_proc(libgl, "glVertexAttribDivisorNV");
ctx->_glUniformMatrix2x3fvNV = (PFNGLUNIFORMMATRIX2X3FVNVPROC)get_proc(libgl, "glUniformMatrix2x3fvNV");
ctx->_glUniformMatrix3x2fvNV = (PFNGLUNIFORMMATRIX3X2FVNVPROC)get_proc(libgl, "glUniformMatrix3x2fvNV");
ctx->_glUniformMatrix2x4fvNV = (PFNGLUNIFORMMATRIX2X4FVNVPROC)get_proc(libgl, "glUniformMatrix2x4fvNV");
ctx->_glUniformMatrix4x2fvNV = (PFNGLUNIFORMMATRIX4X2FVNVPROC)get_proc(libgl, "glUniformMatrix4x2fvNV");
ctx->_glUniformMatrix3x4fvNV = (PFNGLUNIFORMMATRIX3X4FVNVPROC)get_proc(libgl, "glUniformMatrix3x4fvNV");
ctx->_glUniformMatrix4x3fvNV = (PFNGLUNIFORMMATRIX4X3FVNVPROC)get_proc(libgl, "glUniformMatrix4x3fvNV");
ctx->_glReadBufferNV = (PFNGLREADBUFFERNVPROC)get_proc(libgl, "glReadBufferNV");
ctx->_glAlphaFuncQCOM = (PFNGLALPHAFUNCQCOMPROC)get_proc(libgl, "glAlphaFuncQCOM");
ctx->_glGetDriverControlsQCOM = (PFNGLGETDRIVERCONTROLSQCOMPROC)get_proc(libgl, "glGetDriverControlsQCOM");
ctx->_glGetDriverControlStringQCOM = (PFNGLGETDRIVERCONTROLSTRINGQCOMPROC)get_proc(libgl, "glGetDriverControlStringQCOM");
ctx->_glEnableDriverControlQCOM = (PFNGLENABLEDRIVERCONTROLQCOMPROC)get_proc(libgl, "glEnableDriverControlQCOM");
ctx->_glDisableDriverControlQCOM = (PFNGLDISABLEDRIVERCONTROLQCOMPROC)get_proc(libgl, "glDisableDriverControlQCOM");
ctx->_glExtGetTexturesQCOM = (PFNGLEXTGETTEXTURESQCOMPROC)get_proc(libgl, "glExtGetTexturesQCOM");
ctx->_glExtGetBuffersQCOM = (PFNGLEXTGETBUFFERSQCOMPROC)get_proc(libgl, "glExtGetBuffersQCOM");
ctx->_glExtGetRenderbuffersQCOM = (PFNGLEXTGETRENDERBUFFERSQCOMPROC)get_proc(libgl, "glExtGetRenderbuffersQCOM");
ctx->_glExtGetFramebuffersQCOM = (PFNGLEXTGETFRAMEBUFFERSQCOMPROC)get_proc(libgl, "glExtGetFramebuffersQCOM");
ctx->_glExtGetTexLevelParameterivQCOM = (PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC)get_proc(libgl, "glExtGetTexLevelParameterivQCOM");
ctx->_glExtTexObjectStateOverrideiQCOM = (PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC)get_proc(libgl, "glExtTexObjectStateOverrideiQCOM");
ctx->_glExtGetTexSubImageQCOM = (PFNGLEXTGETTEXSUBIMAGEQCOMPROC)get_proc(libgl, "glExtGetTexSubImageQCOM");
ctx->_glExtGetBufferPointervQCOM = (PFNGLEXTGETBUFFERPOINTERVQCOMPROC)get_proc(libgl, "glExtGetBufferPointervQCOM");
ctx->_glExtGetShadersQCOM = (PFNGLEXTGETSHADERSQCOMPROC)get_proc(libgl, "glExtGetShadersQCOM");
ctx->_glExtGetProgramsQCOM = (PFNGLEXTGETPROGRAMSQCOMPROC)get_proc(libgl, "glExtGetProgramsQCOM");
ctx->_glExtIsProgramBinaryQCOM = (PFNGLEXTISPROGRAMBINARYQCOMPROC)get_proc(libgl, "glExtIsProgramBinaryQCOM");
ctx->_glExtGetProgramBinarySourceQCOM = (PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC)get_proc(libgl, "glExtGetProgramBinarySourceQCOM");
ctx->_glStartTilingQCOM = (PFNGLSTARTTILINGQCOMPROC)get_proc(libgl, "glStartTilingQCOM");
ctx->_glEndTilingQCOM = (PFNGLENDTILINGQCOMPROC)get_proc(libgl, "glEndTilingQCOM");
}
