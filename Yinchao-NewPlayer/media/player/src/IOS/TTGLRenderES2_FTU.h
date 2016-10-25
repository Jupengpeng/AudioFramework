#ifndef __TT_GLRENDER_ES2_FTU_
#define __TT_GLRENDER_ES2_FTU_

#import "TTGLRenderES2.h"

#ifdef _VONAMESPACE
namespace _VONAMESPACE {
#endif

class TTGLRenderES2_FTU : public TTGLRenderES2 {
    
    
public:
    TTGLRenderES2_FTU(EAGLContext* pContext);
    virtual ~TTGLRenderES2_FTU();
    
    virtual int SetupFrameBuffer();
    virtual int DeleteFrameBuffer();
    
    virtual int SetupTexture();
    virtual int DeleteTexture();
    
    virtual int TextureSizeChange();
    
    virtual int CompileAllShaders();
    
    virtual int UploadTexture(TTVideoBuffer *pVideoBuffer);
    virtual int RenderToScreen();
    
    virtual bool IsGLRenderReady();
    
    virtual int RedrawInner();
    
private:
    CVOpenGLESTextureRef _lumaTexture;
    CVOpenGLESTextureRef _chromaTexture;
	CVOpenGLESTextureCacheRef videoTextureCache;
    
    GLint m_nTextureUniformY;
    GLint m_nTextureUniformUV;
};

#ifdef _VONAMESPACE
}
#endif

#endif

